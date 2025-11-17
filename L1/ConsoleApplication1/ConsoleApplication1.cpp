#include <iostream>
#include <thread>
#include <string>
#include <time.h>

// Функция для вычисления факториала
unsigned long long factorial(int n) {
    unsigned long long result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// Выполняет указанное количество итераций вычисления факториала
void run_factorial_benchmark(int iterations, int n) {
    for (int i = 0; i < iterations; ++i) {
        volatile unsigned long long fact = factorial(n); // volatile — чтобы компилятор не убрал цикл
    }
}

// Функция для потока (используется в задании 2)
void thread_func(const std::string& name, int iterations, int n) {
    run_factorial_benchmark(iterations, n);
    std::cout << "Поток " << name << " завершён." << std::endl;
}

// Упражнение 1: обратный вывод строки
void exercise1() {
    std::cout << "\n=== Упражнение 1 ===\n";
    std::string s = "01234";
    // Приведение к signed int, чтобы избежать underflow в условии цикла
    for (unsigned int i = static_cast<int>(s.size()) - 1; i >= 0; --i) {
        std::cout << s[i] << std::endl;
    }
}

// Задание 1: однопоточное измерение времени
void task1() {
    std::cout << "\n=== Задание 1 ===\n";
    const int total_iterations = 10000000;
    const int n = 10;

    clock_t start = clock();
    run_factorial_benchmark(total_iterations, n);
    clock_t end = clock();

    double seconds = static_cast<double>(end - start) / CLOCKS_PER_SEC;
    std::cout << "10! = " << factorial(10) << std::endl;
    std::cout << "Time taken: " << seconds << " seconds" << std::endl;
}

// Задание 2: сравнение последовательного и параллельного выполнения
void task2() {
    std::cout << "\n=== Задание 2 ===\n";
    const int total_iterations = 10000000;
    const int n = 10;

    // Последовательное выполнение
    std::cout << "Последовательное выполнение..." << std::endl;
    clock_t start_seq = clock();

    run_factorial_benchmark(total_iterations, n);
    run_factorial_benchmark(total_iterations, n);

    clock_t end_seq = clock();
    double time_seq = static_cast<double>(end_seq - start_seq) / CLOCKS_PER_SEC;
    std::cout << "Время последовательного выполнения: " << time_seq << " секунд" << std::endl;

    // Параллельное выполнение
    std::cout << "\nПараллельное выполнение..." << std::endl;
    clock_t start_par = clock();

    std::thread t1(thread_func, "t1", total_iterations, n);
    std::thread t2(thread_func, "t2", total_iterations, n);

    t1.join();
    t2.join();

    clock_t end_par = clock();
    double time_par = static_cast<double>(end_par - start_par) / CLOCKS_PER_SEC;
    std::cout << "Время параллельного выполнения: " << time_par << " секунд" << std::endl;

    // Сравнение
    std::cout << "\nСравнение:" << std::endl;
    if (time_par > 0) {
        double speedup = time_seq / time_par;
        double efficiency = (speedup / 2.0) * 100.0;
        std::cout << "Ускорение: " << speedup << "x" << std::endl;
        std::cout << "Эффективность: " << efficiency << "%" << std::endl;
    }
    else {
        std::cout << "Невозможно рассчитать ускорение: время параллельного выполнения равно нулю." << std::endl;
    }

    system("pause");
}

// Главная функция: вызывает задания последовательно
int main() {
    setlocale(LC_ALL, "RU");

    exercise1();
    task1();
    task2();

    return 0;
}