#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include "openblas/cblas.h"
using namespace std;

const int N = 1024;

// генерацияя матрицы случайным образом
void gen_matrix(vector<float>& M) {
    random_device rnd;
    mt19937 gen(rnd());
    uniform_real_distribution<float> dist(0.0, 1.0);
    for (int i = 0; i < N * N; i++) {
        M[i] = dist(gen);
    }
}

// 1-й вариант перемножения - по формуле из линейной алгебры
void multiply_alg(vector<float>& A, vector<float>& B, vector<float>& C) {
    for (int i = 0; i < N; i++) { // строка A
        for (int j = 0; j < N; j++) { // столбец B
            float sum = 0;
            for (int k = 0; k < N; k++) { 
                sum += A[i * N + k] * B[k * N + j]; // суммирование
            }
            C[i * N + j] = sum;
        }
    }
}

// 2-й вариант - результат работы функции cblas_sgemm
void multiply_BLAS(const vector<float>& A, const vector<float>& B, vector<float>& C) {
    cblas_sgemm(
        CblasRowMajor,  
        CblasNoTrans,   
        CblasNoTrans,  
        N, N, N,        
        1.0f,          
        A.data(),       
        N,              
        B.data(),       
        N,              
        0.0f,          
        C.data(),       
        N              
    );
}

// 3-й вариант — оптимизированный алгоритм
void multiply_opt(const vector<float>& A, const vector<float>& B, vector<float>& C) {
    // используем блочное умножение
    const int BLOCK_SIZE = 128;  
    fill(C.begin(), C.end(), 0.0f);
    float* c_data = C.data();
    const float* a_data = A.data();
    const float* b_data = B.data();

    for (int i = 0; i < N; i += BLOCK_SIZE) {
        for (int j = 0; j < N; j += BLOCK_SIZE) {
            for (int k = 0; k < N; k += BLOCK_SIZE) {

                int i_end = min(i + BLOCK_SIZE, N);
                int j_end = min(j + BLOCK_SIZE, N);
                int k_end = min(k + BLOCK_SIZE, N);

                // перестановка циклов с ijk на ikj
                for (int ii = i; ii < i_end; ii++) {
                    float* c_row = c_data + ii * N;

                    for (int kk = k; kk < k_end; kk++) {
                        float aik = a_data[ii * N + kk]; 
                        const float* b_row = b_data + kk * N;

                        int jj = j;
                        // используем размотку циклов
                        for (; jj + 7 < j_end; jj += 8) {
                            c_row[jj] += aik * b_row[jj];
                            c_row[jj + 1] += aik * b_row[jj + 1];
                            c_row[jj + 2] += aik * b_row[jj + 2];
                            c_row[jj + 3] += aik * b_row[jj + 3];
                            c_row[jj + 4] += aik * b_row[jj + 4];
                            c_row[jj + 5] += aik * b_row[jj + 5];
                            c_row[jj + 6] += aik * b_row[jj + 6];
                            c_row[jj + 7] += aik * b_row[jj + 7];
                        }
                        for (; jj < j_end; jj++) {
                            c_row[jj] += aik * b_row[jj];
                        }
                    }
                }
            }
        }
    }
}

int main() {
    vector<float> A(N * N);
    vector<float> B(N * N);
    vector<float> C1(N * N, 0);
    vector<float> C2(N * N, 0);
    vector<float> C3(N * N, 0);

    cout << "Генерацияя матрицы..." << endl;
    gen_matrix(A);
    gen_matrix(B);

    double operations = 2.0 * N * N * N;
    double time_alg, time_blas, time_opt;

    cout << "\n1. Перемножение по алгебраической формуле\n";
    auto start = chrono::high_resolution_clock::now();
    multiply_alg(A, B, C1);
    auto end = chrono::high_resolution_clock::now();
    time_alg = chrono::duration<double>(end - start).count();
    double time = chrono::duration<double>(end - start).count();
    cout << "Время: " << time_alg << " c\n";
    cout << "Производительность: " << (operations / time_alg) * 1e-6 << " MFLOPS\n";

    cout << "\n2. Перемножение с использованием BLAS \n";
    start = chrono::high_resolution_clock::now();
    multiply_BLAS(A, B, C2);
    end = chrono::high_resolution_clock::now();
    time_blas = chrono::duration<double>(end - start).count();
    cout << "Время: " << time_blas << " c\n";
    cout << "Производительность: " << (operations / time_blas) * 1e-6 << " MFLOPS\n";

    cout << "\n3. Оптимизированное перемножение\n";
    start = chrono::high_resolution_clock::now();
    multiply_opt(A, B, C3);
    end = chrono::high_resolution_clock::now();
    time_opt = chrono::duration<double>(end - start).count();
    cout << "Время: " << time_opt << " с\n";
    cout << "Производительность: " << (operations / time_opt) * 1e-6 << " MFLOPS\n";

    double perf_alg = (operations / time_alg) * 1e-6;
    double perf_blas = (operations / time_blas) * 1e-6;
    double perf_opt = (operations / time_opt) * 1e-6;
    double percent = (perf_opt / perf_blas) * 100; // процент производительности

    cout << fixed << setprecision(2);
    cout << "Процент от BLAS: " << percent << "%" << endl;
    cout << "\n Абуева Елена Дмитриевна, 090304 - РПИа - 025";
}