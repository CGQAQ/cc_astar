#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <algorithm>
#include <functional>
#include <cstring>
#include <cmath>

using namespace std;

const int IINFINITY = INT32_MAX;

struct Point {
    int x;
    int y;
};

struct Node {
    Point p;
    optional<weak_ptr<Node>> parent;
    vector<weak_ptr<Node>> neighbors;
    int fscore;
    int gscore;

    bool operator==(Node other) const;
};

bool Node::operator==(Node other) const {
    return this->p.x == other.p.x && this->p.y == other.p.y;
}

using collection_t = vector<shared_ptr<Node>>;

void
init_neighbors(int width, int height, vector<shared_ptr<Node>>& nodes) {
    if (width * height != nodes.size()) return;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            pair<int, int> np[4] = {
                    {x - 1, y},
                    {x, y + 1},
                    {x + 1, y},
                    {x, y - 1}
            };
            vector<pair<int, int>> vnp = {};
            /* auto last = remove_if(np, np + 4, [width, height](auto v) {
                 return v.first < 0 || v.first >= width || v.second < 0 || v.second > height;
                 });*/
            for (auto npi : np) {
                if (npi.first >= 0 && npi.first < width && npi.second >= 0 && npi.second < height) {
                    vnp.push_back(npi);
                }
            }

            //            for_each(np, last, [](auto v) {
            //                cout << "(" << v.first << "," << v.second << ")" << endl;
            //            });
            for (auto a : vnp) {
                nodes[y * width + x]->neighbors.push_back(nodes[a.second * width + a.first]);
            }
        }
    }
}

optional<collection_t>
init(int width, int height) {
    if (width <= 0 || height <= 0)
        return nullopt;
    collection_t result;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            result.push_back(
                    make_shared<Node>(Node{
                            .p = {.x = x, .y = y},
                            .parent  = nullopt,
                            .neighbors = vector<weak_ptr<Node>>(),
                            .fscore = IINFINITY,
                            .gscore = IINFINITY,
                    }));
        }
    }
    return result;
}

vector<weak_ptr<Node>> reconstruct_path(weak_ptr<Node> current_) {
    vector<weak_ptr<Node>> result_path;
    result_path.push_back(current_);
    optional<weak_ptr<Node>> current = current_.lock()->parent;
    while (current.has_value())
    {
        auto _current = current.value().lock();
        result_path.push_back(_current);
        current = _current->parent;
    }
    reverse(result_path.begin(), result_path.end());
    return result_path;
}

// typedef int(*HFUNC)(weak_ptr<Node>, weak_ptr<Node>);
using HFUNC = function<int(weak_ptr<Node>, weak_ptr<Node>)>;

optional<vector<weak_ptr<Node>>>
astar(weak_ptr<Node> start, weak_ptr<Node> end, HFUNC h) {
vector<weak_ptr<Node>> openSet;
openSet.push_back(start);

// Init start node
auto s = start.lock();
s->fscore = 0;
s->gscore = h(start, end);

// Start algorithm
while (!openSet.empty())
{
sort(openSet.begin(), openSet.end(), [](auto a, auto b) {
return a.lock()->fscore < b.lock()->fscore;
});

auto current = openSet.front();
openSet.erase(openSet.begin());

if (current.lock() == end.lock()) {
return reconstruct_path(current);
}



for (auto neighbor : current.lock()->neighbors) {
int tentative_gscore = current.lock()->gscore + 1;
if (tentative_gscore < neighbor.lock()->gscore) {
auto g = neighbor.lock();
g->parent = current;
g->gscore = tentative_gscore;
g->fscore = g->gscore + h(neighbor, end);
if (none_of(openSet.begin(), openSet.end(), [&g](auto v) {
return v.lock() == g;
})){
openSet.push_back(g);
}
}
}
}
return nullopt;
}


int main() {
    auto nodes = init(10, 10).value();
    init_neighbors(10, 10, nodes);
    optional<vector<weak_ptr<Node>>> p = astar(nodes[0], nodes[10 * 10 - 1], [](auto a_, auto b_) {
        auto a = a_.lock();
        auto b = b_.lock();
        return sqrt(pow(a->p.x - b->p.y, 2) + pow(a->p.y - b->p.y, 2));
    });

    /* HANDLE hbuff = CreateConsoleScreenBuffer(
          GENERIC_WRITE | GENERIC_READ,
          FILE_SHARE_READ,
          NULL,
          CONSOLE_TEXTMODE_BUFFER,
          NULL
      );

      SetConsoleActiveScreenBuffer(hbuff);

      WriteConsole(hbuff, )*/

    char ch[100] = {};
    memset(ch, 'O', 100);
    for (auto a = p.value().begin(); a < p.value().end(); ++a) {
        if (a == p.value().begin())
            ch[a->lock()->p.y * 10 + a->lock()->p.x] = 'S';
        else if (a == p.value().end() - 1)
            ch[a->lock()->p.y * 10 + a->lock()->p.x] = 'E';
        else
            ch[a->lock()->p.y * 10 + a->lock()->p.x] = 'P';
    }

    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            cout << ch[y * 10 + x];
        }
        cout << endl;
    }

    return 0;
}

