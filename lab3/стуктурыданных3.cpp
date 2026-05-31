#include <iostream>
#include <vector>
#include <list>
#include <chrono>
#include <iomanip>
using namespace std;
using namespace chrono;

// 1. Реализация через одномерный массив
class MiceArray {
    vector<int> mice; // вектор для хранения мышей: 0-серая, 1-белая, -1-съеденная
    int n; // общее количество мышей

    bool eating(int S, int need_gray, int need_white) {
        int gray = 0, white = 0;
        for (int i = 0; i < n; i++)
            if (mice[i] == 0) gray++; else if (mice[i] == 1) white++;

        if (gray < need_gray || white < need_white) return false;

        vector<int> status = mice; // создаём копию начального положения
        int cur = 0, alive = n;

        while (alive > 0 && (gray > need_gray || white > need_white)) {
            int steps = S;
            while (steps > 0) {
                if (status[cur] != -1) steps--;
                if (steps > 0) cur = (cur + 1) % n;
            }

            if (status[cur] == 0) gray--;
            else if (status[cur] == 1) white--;
            status[cur] = -1;
            alive--;

            if (gray == need_gray && white == need_white) return true;

            if (alive > 0) {
                cur = (cur + 1) % n;
                while (status[cur] == -1) cur = (cur + 1) % n;
            }
        }
        return (gray == need_gray && white == need_white);
    }

public:
    MiceArray(int total, const vector<int>& init) : n(total), mice(init) {}
    bool is_valid(int S, int K, int L) { return eating(S, K, L); }
};

// 2. Реализация через круговой двусвязный список
struct Node {
    int color; // // 0 - серая, 1 - белая
    Node* next, * prev;
    bool alive;
    Node(int c) : color(c), next(nullptr), prev(nullptr), alive(true) {}
};

class MiceList {
    Node* head;
    int size;

    bool eating(int S, int need_gray, int need_white) {
        int gray = 0, white = 0;
        Node* cur = head;
        for (int i = 0; i < size; i++) {
            if (cur->color == 0) gray++; else white++;
            cur = cur->next;
        }
        if (gray < need_gray || white < need_white) return false;

        cur = head;
        int alive = size;
        while (alive > 0 && (gray > need_gray || white > need_white)) {
            int steps = S;
            while (steps > 0) {
                if (cur->alive) steps--;
                if (steps > 0) cur = cur->next;
            }

            cur->alive = false;
            if (cur->color == 0) gray--; else white--;
            alive--;

            if (gray == need_gray && white == need_white) return true;

            if (alive > 0) {
                do cur = cur->next;
                while (!cur->alive);
            }
        }
        return (gray == need_gray && white == need_white);
    }

public:
    MiceList(int total, const vector<int>& init) : size(total) {
        head = new Node(init[0]);
        Node* cur = head;
        for (int i = 1; i < total; i++) {
            cur->next = new Node(init[i]);
            cur->next->prev = cur;
            cur = cur->next;
        }
        cur->next = head;
        head->prev = cur;
    }

    bool is_valid(int S, int K, int L) { return eating(S, K, L); }

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

// 3. Реализация через двусвязный список STL
class MiceSTL {
    list<int> mice; // 0 - серая, 1 - белая

    bool eating(int S, int need_gray, int need_white) {
        int gray = 0, white = 0;
        for (int m : mice)
            if (m == 0) gray++; else white++;
        if (gray < need_gray || white < need_white) return false;

        auto it = mice.begin();
        int alive = mice.size();

        while (alive > 0 && (gray > need_gray || white > need_white)) {
            for (int i = 1; i < S; i++) {
                it++;
                if (it == mice.end()) it = mice.begin();
            }

            if (*it == 0) gray--; else white--;
            it = mice.erase(it);
            if (it == mice.end() && !mice.empty()) it = mice.begin();
            alive--;

            if (gray == need_gray && white == need_white) return true;
        }
        return (gray == need_gray && white == need_white);
    }

public:
    MiceSTL(int total, const vector<int>& initial) {
        for (int i = 0; i < total; i++)
            mice.push_back(initial[i]);
    }

    bool is_valid(int S, int K, int L) { return eating(S, K, L); }
};

// Перебор
template<typename MiceClass>
bool backtrack(int gray_used, int white_used, vector<int>& current,
    vector<int>& result, int total, int S, int K, int L) {

    if (current.size() == total) {
        MiceClass test(total, current);
        if (test.is_valid(S, K, L)) {
            result = current;
            return true;
        }
        return false;
    }

    if (current.empty()) {
        current.push_back(0);
        if (backtrack<MiceClass>(1, 0, current, result, total, S, K, L))
            return true;
        current.pop_back();
        return false;
    }

    int can_eat = total - (K + L);

    if (gray_used < can_eat) {
        current.push_back(0);
        if (backtrack<MiceClass>(gray_used + 1, white_used, current, result, total, S, K, L))
            return true;
        current.pop_back();
    }

    if (white_used < can_eat) {
        current.push_back(1);
        if (backtrack<MiceClass>(gray_used, white_used + 1, current, result, total, S, K, L))
            return true;
        current.pop_back();
    }

    return false;
}

// Тестирование
template<typename MiceClass>
long long test(int N, int M, int S, int K, int L, vector<int>& result, const string& name) {
    cout << name << ":\n";
    vector<int> current;

    auto start = high_resolution_clock::now();
    bool found = backtrack<MiceClass>(1, 0, current, result, N + M, S, K, L);
    auto end = high_resolution_clock::now();

    if (found) {
        cout << "Результат: ";
        for (int x : result) cout << (x == 0 ? 'C' : 'B');
        cout << "\n";
    }
    else {
        cout << "Результат не найден\n";
    }

    long long duration = duration_cast<microseconds>(end - start).count();
    cout << "Время: " << duration << " мкс\n\n";
    return duration;
}

int main() {
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

    cout << "\nРезультаты тестирования: \n\n";
    vector<int> result1, result2, result3;

    long long time1 = test<MiceArray>(N, M, S, K, L, result1, "1. Массив: ");
    long long time2 = test<MiceList>(N, M, S, K, L, result2, "2. Связанный список: ");
    long long time3 = test<MiceSTL>(N, M, S, K, L, result3, "3. Связанный список из STL: ");

    cout << "Сравнение производительности\n";
    cout << "Массив: " << time1 << " мкс (1.00x)\n";
    cout << "Связанный список: " << time2 << " мкс ("
        << fixed << setprecision(2) << (double)time2 / time1 << "x)\n";
    cout << "Связанный список из STL: " << time3 << " мкс ("
        << fixed << setprecision(2) << (double)time3 / time1 << "x)\n";

    cout << "\nПроверка работоспособности:\n";
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