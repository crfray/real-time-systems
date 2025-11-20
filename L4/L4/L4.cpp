#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>
#include <iostream>

const int NUM_DAYS = 5;
const int MAX_NUGGETS_PER_eater = 10000;

std::mutex table_lock;
std::vector<int> bowls = { 3000, 3000, 3000 };
std::atomic<bool> cook_fired(false);
std::atomic<int> fatmen_eaten[3] = { 0, 0, 0 };
std::atomic<bool> has_exploded[3] = { false, false, false };
std::atomic<int> exploded_count(0);
std::atomic<bool> time_up(false);
std::atomic<bool> time_is_up(false);

void eater(int id, int gluttony) {
    while (!cook_fired.load() && !time_is_up.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lock(table_lock);

        if (cook_fired.load() || time_is_up.load())
            return;

        if (bowls[id] <= 0) {
            cook_fired = true;
            return;
        }

        int to_eat = std::min(gluttony, bowls[id]);
        bowls[id] -= to_eat;
        fatmen_eaten[id] += to_eat;


        if (!has_exploded[id] && fatmen_eaten[id] >= MAX_NUGGETS_PER_eater) {
            has_exploded[id] = true;
            exploded_count++;
        }
    }
}

void cook(int efficiency_factor) {
    auto start = std::chrono::steady_clock::now();
    const auto duration = std::chrono::seconds(NUM_DAYS);

    while (!cook_fired.load()) {
        auto now = std::chrono::steady_clock::now();
        if (now - start >= duration) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (cook_fired.load()) break;

        std::lock_guard<std::mutex> lock(table_lock);
        if (cook_fired.load()) break;

        for (int i = 0; i < 3; ++i) {
            bowls[i] += efficiency_factor;
        }
    }
    time_is_up = true; // даём сигнал остановиться
}

void run_test(int gluttony, int efficiency_factor, const std::string& name) {
    //сброс
    bowls = { 3000, 3000, 3000 };
    cook_fired = false;
    time_is_up = false;
    exploded_count = 0;
    for (int i = 0; i < 3; ++i) has_exploded[i] = false;
    for (int i = 0; i < 3; ++i) fatmen_eaten[i] = 0;


    std::cout << "\n=== " << name << " ===\n";
    std::cout << "обжорство = " << gluttony << ", продуктивность кука = " << efficiency_factor << "\n";

    std::thread t_cook(cook, efficiency_factor);
    std::thread t1(eater, 0, gluttony);
    std::thread t2(eater, 1, gluttony);
    std::thread t3(eater, 2, gluttony);

    t_cook.join();
    t1.join();
    t2.join();
    t3.join();

    // результат
    if (cook_fired) {
        std::cout << "результат: кука уволили\n";
    }
    else if (exploded_count == 3) {
        std::cout << "результат: кук не получил зарплату\n";
    }
    else {
        std::cout << "результат: кук уволился сам\n";
    }

    std::cout << "в тарелках у обжор: ";
    for (int p : bowls) std::cout << p << " ";
    std::cout << std::endl;
    std::cout << "обжоры съели: ";
    for (int i = 0; i < 3; i++) std::cout << fatmen_eaten[i] << " ";
    std::cout << std::endl;
    std::cout << "взорвалось обжор: " << exploded_count << "\n";
}

int main() {
    setlocale(LC_ALL, "RU");

    // Набор коэффициентов 1:
    run_test(500, 100, "условие 1: увольнение");

    // Набор коэффициентов 2:
    run_test(220, 800, "условие 2: кук не получил зарплату");

    // Набор коэффициентов 3:
    run_test(200, 500, "условие 3: увольняется сам");

    return 0;
}