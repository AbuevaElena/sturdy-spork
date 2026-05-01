#include <iostream>
#include <vector>
#include <list>
#include <chrono>
#include <windows.h>

using namespace std;
using namespace chrono;

// 1. Реализация через одномерный массив
class MiceArray {
private:
    vector<int> mice; // 0 - серая мышь, 1 - белая мышь, -1 - съеденная
    int n;

public:
    MiceArray(int total, const vector<int>& init) : n(total), mice(init) {}

    bool is_valid(int S, int need_gray, int need_white) {
        int gray = 0, white = 0;
        for (int i = 0; i < n; i++) {
            if (mice[i] == 0) gray++;
            else if (mice[i] == 1) white++;
        }

        if (gray < need_gray || white < need_white) return false;

        vector<int> state = mice;
        int cur = 0;
        int alive = n;

        while (alive > 0 && (gray > need_gray || white > need_white)) {
            int steps = S;
            while (steps > 0) {
                if (state[cur] != -1) steps--;
                if (steps > 0) cur = (cur + 1) % n;
            }

            if (state[cur] == 0) gray--;
            else if (state[cur] == 1) white--;

            state[cur] = -1;
            alive--;

            if (gray == need_gray && white == need_white) return true;

            if (alive > 0) {
                cur = (cur + 1) % n;
                while (state[cur] == -1) cur = (cur + 1) % n;
            }
        }
        return (gray == need_gray && white == need_white);
    }
};

// 2. Реализация через круговой двусвязный список
struct Node {
    int color; // 0 или 1
    int id;
    Node* next;
    Node* prev;
    bool alive;

    Node(int c, int i) : color(c), id(i), next(nullptr), prev(nullptr), alive(true) {}
};

class MiceList {
private:
    Node* head;
    int size;

public:
    MiceList(int total, const vector<int>& init) : size(total) {
        head = new Node(init[0], 0);
        Node* cur = head;

        for (int i = 1; i < total; i++) {
            cur->next = new Node(init[i], i);
            cur->next->prev = cur;
            cur = cur->next;
        }
        cur->next = head;
        head->prev = cur;
    }

    bool is_valid(int S, int need_gray, int need_white) {
        int gray = 0, white = 0;
        Node* cur = head;

        for (int i = 0; i < size; i++) {
            if (cur->color == 0) gray++;
            else white++;
            cur = cur->next;
        }

        if (gray < need_gray || white < need_white) return false;

        cur = head;
        for (int i = 0; i < size; i++) {
            cur->alive = true;
            cur = cur->next;
        }

        cur = head;
        int alive = size;

        while (alive > 0 && (gray > need_gray || white > need_white)) {
            int steps = S;
            while (steps > 0) {
                if (cur->alive) steps--;
                if (steps > 0) cur = cur->next;
            }

            cur->alive = false;
            if (cur->color == 0) gray--;
            else white--;
            alive--;

            if (gray == need_gray && white == need_white) return true;

            if (alive > 0) {
                do {
                    cur = cur->next;
                } while (!cur->alive);
            }
        }
        return (gray == need_gray && white == need_white);
    }

    ~MiceList() {
        if (!head) return;
        Node* cur = head;
        Node* first = head;
        do {
            Node* next = cur->next;
            delete cur;
            cur = next;
        } while (cur != first);
    }
};

// 3. Реализация через двусвязный список из STL
class MiceSTL {
private:
    list<pair<int, int>> mice;

public:
    MiceSTL(int total, const vector<int>& initial) {
        for (int i = 0; i < total; i++) {
            mice.push_back({ initial[i], i });
        }
    }

    bool is_valid(int S, int need_gray, int need_white) {
        int gray = 0, white = 0;
        for (auto& m : mice) {
            if (m.first == 0) gray++;
            else white++;
        }

        if (gray < need_gray || white < need_white) return false;

        auto it = mice.begin();
        int alive = mice.size();

        while (alive > 0 && (gray > need_gray || white > need_white)) {
            for (int i = 1; i < S; i++) {
                it++;
                if (it == mice.end()) it = mice.begin();
            }

            if (it->first == 0) gray--;
            else white--;

            it = mice.erase(it);
            if (it == mice.end() && !mice.empty()) it = mice.begin();
            alive--;

            if (gray == need_gray && white == need_white) return true;
        }
        return (gray == need_gray && white == need_white);
    }
};

// Перебор
template<typename MiceClass>
bool solve_recursive(int gray_used, int white_used, vector<int>& current,
    vector<int>& result, int total_mice, int S, int K, int L) {

    // Базовый случай
    if (current.size() == total_mice) {
        MiceClass test(total_mice, current);
        if (test.is_valid(S, K, L)) {
            result = current;
            return true;  
        }
        return false;     
    }

    // Первая мышь только серая
    if (current.empty()) {
        current.push_back(0);
        if (solve_recursive<MiceClass>(gray_used + 1, white_used, current, result,
            total_mice, S, K, L)) return true;
        current.pop_back();
        return false;
    }

    int max_to_eat = total_mice - (K + L);

    // Пробуем добавить серую
    if (gray_used < max_to_eat) {
        current.push_back(0);
        if (solve_recursive<MiceClass>(gray_used + 1, white_used, current, result,
            total_mice, S, K, L)) return true;
        current.pop_back();
    }

    // Пробуем добавить белую
    if (white_used < max_to_eat) {
        current.push_back(1);
        if (solve_recursive<MiceClass>(gray_used, white_used + 1, current, result,
            total_mice, S, K, L)) return true;
        current.pop_back();
    }
    return false;  
}

// Тестирование производительности
template<typename MiceClass>
long long test_performance(int N, int M, int S, int K, int L,
    vector<int>& result, const string& name) {
    cout << name << ":\n";

    vector<int> current;

    auto start = high_resolution_clock::now();
    bool found = solve_recursive<MiceClass>(1, 0, current, result, N + M, S, K, L);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);

    if (found) {
        cout << "Результат: ";
        for (int x : result) cout << (x == 0 ? 'C' : 'B');
        cout << "\n";
    }
    else {
        cout << "Результат не найден\n";
    }

    cout << "Время: " << duration.count() << " мкс\n";
    cout << "Работоспособность: " << (found ? "КОРРЕКТНО" : "ОШИБКА") << "\n\n";

    return duration.count();
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    int N, M, S, K, L;
    cout << "Введите количество серых мышей (N): ";
    cin >> N;
    cout << "Введите количество белых мышей (M): ";
    cin >> M;
    cout << "Введите шаг S: ";
    cin >> S;
    cout << "Введите, сколько серых осталось (K): ";
    cin >> K;
    cout << "Введите, сколько белых осталось (L): ";
    cin >> L;

    cout << "\nРЕЗУЛЬТАТЫ ТЕСТИРОВАНИЯ\n";
    vector<int> result1, result2, result3;

    long long time1 = test_performance<MiceArray>(N, M, S, K, L, result1, "1. МАССИВ");
    long long time2 = test_performance<MiceList>(N, M, S, K, L, result2, "2. СВЯЗАННЫЙ СПИСОК");
    long long time3 = test_performance<MiceSTL>(N, M, S, K, L, result3, "3. СПИСОК ИЗ STL");

    cout << "СРАВНЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ\n";
    cout << "Массив: " << time1 << " мкс (1.00x)\n";
    cout << "Связный список: " << time2 << " мкс ("
        << fixed << setprecision(2) << (double)time2 / time1 << "x)\n";
    cout << "STL list: " << time3 << " мкс ("
        << fixed << setprecision(2) << (double)time3 / time1 << "x)\n";

    cout << "\nПРОВЕРКА РАБОТОСПОСОБНОСТИ\n";
    bool results_match = (result1 == result2 && result2 == result3);
    if (results_match && !result1.empty()) {
        cout << "Все три реализации дали одинаковый результат!\n";
    }
    else if (result1.empty() && result2.empty() && result3.empty()) {
        cout << "Решение не найдено ни одной реализацией\n";
        cout << "Результаты совпадают (все пустые)\n";
    }
    else {
        cout << "Результаты различаются, ошибка в реализации!\n";
    }

    cout << "\nАбуева Елена Дмитриевна, 090304 - РПИа - 025";
    cin.ignore();
    cin.get();
    return 0;
}