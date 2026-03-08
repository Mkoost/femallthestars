#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <deque>
#include <algorithm>

struct StaticGraph {
    std::vector<int> head;
    std::vector<int> arrows;

    StaticGraph(int num_vertex, int num_arrows)
        : head(num_vertex + 1, 0), arrows(num_arrows) {}
};

struct DynamicGraph {
    struct Node {
        int id;
        Node* ptr;
    };

    std::vector<Node*> head;
    std::deque<Node> nodes;
    std::vector<Node*> tail;
    std::vector<int> degree;

    explicit DynamicGraph(int num_vertex) {
        head.resize(num_vertex, nullptr);
        tail.resize(num_vertex, nullptr);
        degree.resize(num_vertex, 0);
    }

    DynamicGraph(int num_vertex, int /*num_arrows*/) {
        head.resize(num_vertex, nullptr);
        tail.resize(num_vertex, nullptr);
        degree.resize(num_vertex, 0);
    }

    bool is_linked(int id1, int id2) const {
        Node* tmp = head[id1 - 1];
        while (tmp) {
            if (tmp->id == id2) return true;
            tmp = tmp->ptr;
        }
        return false;
    }

    void add_arrow(int id1, int id2) {
        if (is_linked(id1, id2)) return;

        nodes.push_back(Node{id2, nullptr});
        Node* new_node = &nodes.back();
        Node* last = tail[id1 - 1];
        tail[id1 - 1] = new_node;

        if (last)
            last->ptr = new_node;
        else
            head[id1 - 1] = new_node;

        ++degree[id1 - 1];
    }

    void add_edge(int id1, int id2) {
        add_arrow(id1, id2);
        add_arrow(id2, id1);
    }

    StaticGraph get_static() const {
        int n = static_cast<int>(head.size());
        int m = static_cast<int>(nodes.size());
        StaticGraph graph(n, m);

        int ln = 0;
        for (int i = 0; i < n; ++i) {
            graph.head[i] = ln;
            Node* tmp = head[i];
            while (tmp) {
                graph.arrows[ln++] = tmp->id;
                tmp = tmp->ptr;
            }
        }
        graph.head[n] = ln;
        return graph;
    }

    // Построение структуры уровней от вершины id
    DynamicGraph create_level_struct(int id) const {
        int n = static_cast<int>(head.size());
        std::vector<bool> mask(n, true);      // все вершины не посещены
        DynamicGraph graph(n, n);

        // Стартовая вершина
        mask[id - 1] = false;
        graph.add_arrow(1, id);               // уровень 0 хранится в вершине 1

        int level = 0;
        // Пока есть вершины на текущем уровне и не вышли за пределы head
        while (level < n && graph.head[level] != nullptr) {
            Node* ptr = graph.head[level];     // список вершин текущего уровня

            while (ptr) {
                int cur = ptr->id;             // текущая вершина исходного графа
                ptr = ptr->ptr;

                // Все соседи cur в исходном графе
                Node* neigh = head[cur - 1];
                while (neigh) {
                    int v = neigh->id;
                    if (mask[v - 1]) {          // ещё не посещена
                        mask[v - 1] = false;
                        // Добавляем v на следующий уровень (уровень level+1 хранится в вершине level+2)
                        graph.add_arrow(level + 2, v);
                    }
                    neigh = neigh->ptr;
                }
            }
            ++level;
        }
        return graph;
    }

    // Поиск последнего непустого уровня (линейный проход с конца)
    int find_last_non_null(const DynamicGraph& graph) const {
        for (int i = static_cast<int>(graph.head.size()) - 1; i >= 0; --i) {
            if (graph.head[i] != nullptr)
                return i + 1;   // возвращаем номер уровня (1‑based для удобства)
        }
        return 0; // пустой граф
    }

    // Находит среди вершин уровня l вершину с наименьшей степенью в исходном графе
    int find_farest_vertex(const DynamicGraph& levelstruct, int l) const {
        // l – номер уровня (1‑based)
        if (l < 1 || l > static_cast<int>(levelstruct.head.size()))
            return -1;
        Node* ptr = levelstruct.head[l - 1];
        if (!ptr) return -1;

        int best = ptr->id;
        int min_deg = degree[best - 1];

        ptr = ptr->ptr;
        while (ptr) {
            int v = ptr->id;
            if (degree[v - 1] < min_deg) {
                min_deg = degree[v - 1];
                best = v;
            }
            ptr = ptr->ptr;
        }
        return best;
    }

    // Поиск псевдопериферийного узла, начиная с вершины id
    DynamicGraph find_quasiinit_vertex(int id) {
        DynamicGraph levelstruct_1 = create_level_struct(id);
        int l = find_last_non_null(levelstruct_1);   // последний уровень (1‑based)

        while (true) {
            int newid = find_farest_vertex(levelstruct_1, l);
            if (newid == -1) break;   // защита от ошибок

            DynamicGraph levelstruct_2 = create_level_struct(newid);
            int newl = find_last_non_null(levelstruct_2);

            if (l >= newl)
                return levelstruct_2;  // возвращаем структуру от найденной вершины
            else {
                levelstruct_1 = std::move(levelstruct_2);
                l = newl;
                id = newid;
            }
        }
        return levelstruct_1; // запасной вариант
    }

    std::vector<int> rcm(int id){
        auto levelstruct = find_quasiinit_vertex(id);

        for(auto u: levelstruct.head) std::cout << u << " ";
        std::cout << std::endl;
        int n = head.size();
        std::vector<int> replacevec(n);
        std::vector<Node*> newhead(n);

        int i = 0;
        int k = 0;
        Node* ptr = levelstruct.head[i];
        ++i;
        while(ptr != nullptr){
            do{
                replacevec[k++] = ptr->id;
                ptr = ptr->ptr;
            } while(ptr != nullptr);
            ptr = levelstruct.head[i++];
        }
        std::reverse(replacevec.begin(), replacevec.end());
        i = 0;

        ptr = head[i];
        ++i;
        for(auto u: head){
            Node* ptr = u;
            while(ptr != nullptr){
                ptr->id = replacevec[ptr->id - 1];
                ptr = ptr->ptr;
            }
        }

        for(int i = 0; i < n; ++i)
            newhead[replacevec[i] - 1] = head[i];
        std::swap(head, newhead);

        return replacevec;

    }
};

// Запись DynamicGraph в файл (для отладки)
void writeGraphToFile(const DynamicGraph& graph, const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Ошибка открытия файла " << filename << std::endl;
        return;
    }
    int n = static_cast<int>(graph.head.size());
    out << n << "\n";
    for (int i = 0; i < n; ++i) {
        out << i + 1 << ": ";
        DynamicGraph::Node* tmp = graph.head[i];
        while (tmp) {
            out << tmp->id << " ";
            tmp = tmp->ptr;
        }
        out << "\n";
    }
}

// Запись StaticGraph в файл (альтернативный формат)
void writeGraphToFile(const StaticGraph& graph, const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Ошибка открытия файла " << filename << std::endl;
        return;
    }
    int n = static_cast<int>(graph.head.size()) - 1;
    out << n << "\n";
    for (int i = 0; i < n; ++i) {
        out << i << ": ";
        for (int j = graph.head[i]; j < graph.head[i + 1]; ++j) {
            out << graph.arrows[j] - 1 << " ";
        }
        out << "\n";
    }
}

// Чтение сетки из файла формата .txt (NE NP NC, затем элементы, затем координаты)
DynamicGraph readMeshFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
        return DynamicGraph(0);
    }

    int NE, NP, NC;
    in >> NE >> NP >> NC;
    std::cout << "Чтение сетки: NE=" << NE << ", NP=" << NP << ", NC=" << NC << std::endl;

    DynamicGraph graph(NP);

    // Чтение элементов (треугольников)
    for (int i = 0; i < NE; ++i) {
        int elemId, numVertex;
        in >> elemId >> numVertex;
        std::vector<int> vertices(numVertex);
        for (int j = 0; j < numVertex; ++j) {
            in >> vertices[j];
        }
        // Добавляем рёбра треугольника
        if (numVertex >= 3) {
            graph.add_edge(vertices[0], vertices[1]);
            graph.add_edge(vertices[1], vertices[2]);
            graph.add_edge(vertices[2], vertices[0]);
        }
    }

    // Пропускаем координаты узлов
    for (int i = 0; i < NP; ++i) {
        int nodeId;
        double x, y, z;
        in >> nodeId >> x >> y >> z;
    }

    // Остальные данные игнорируем
    return graph;
}

int main() {
    auto graph = readMeshFromFile("res32.txt");
    if (graph.head.empty()) {
        std::cerr << "Граф пуст!" << std::endl;
        return 1;
    }
    writeGraphToFile(graph, "truegraph.txt");

    // Начинаем поиск с произвольной вершины, например 1
    DynamicGraph result = graph.find_quasiinit_vertex(1);

    writeGraphToFile(result, "graph.txt");
    std::cout << "Структура уровней от псевдопериферийного узла записана в graph.t" << std::endl;
    auto rp = graph.rcm(1);
    for(auto u: rp) std::cout << u << " ";
    std::cout << std::endl;

    writeGraphToFile(graph, "graph2.txt");

    return 0;
}
