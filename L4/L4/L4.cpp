#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

using namespace std;


mutex table_mutex;
atomic<bool> stop_simulation{ false };
atomic<int> total_nuggets_eaten[3]{ 0 };
int dish[3] = { 3000, 3000, 3000 }; // по условию 3000 наггетсов в начале
atomic<int> fatmen_destroyed{ 0 }; //лопнувшие толстяки


int gluttony = 3;
int efficiency_factor = 3;

// необходимые флаги
atomic<int> eaters_done_count{ 0 };
atomic<bool> cook_just_added{ false };
atomic<bool> eaters_finished{ false };

void cook_run() {
    auto start_time = chrono::steady_clock::now();
    while (!stop_simulation) {
        {
            lock_guard<mutex> lock(table_mutex);
            int per_plate = efficiency_factor / 3;
            int remainder = efficiency_factor % 3;
            for (int i = 0; i < 3; ++i) {
                dish[i] += per_plate + (i < remainder ? 1 : 0);
            }
            cook_just_added = true;
            eaters_finished = false;
            eaters_done_count = 0; // сброс
        }

        // ждём толстяков (ограничено по времени)
        auto wait_start = chrono::steady_clock::now();
        while (!eaters_finished && !stop_simulation) {
            if (chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - wait_start).count() > 5) {
                // чтобы код не висел нужен выход
                break;
            }
            this_thread::yield(); // синхронизация потоков с помощью yield
        }
        cook_just_added = false;

        if (chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count() >= 5) {
            break;
        }

        this_thread::sleep_for(chrono::milliseconds(10));
    }
    // сигнал к завершению
    stop_simulation = true;
}

void fatman(int id) {
    while (!stop_simulation) {
        // ждём кука (если повар ещё работает)
        while (!cook_just_added && !stop_simulation) {
            this_thread::yield(); // также синхронизация
        }
        if (stop_simulation) break;

        bool destroyed_or_starved = false;
        {
            lock_guard<mutex> lock(table_mutex);
            if (dish[id] >= gluttony) {
                dish[id] -= gluttony;
                total_nuggets_eaten[id] += gluttony;
                if (total_nuggets_eaten[id] >= 10000) { //лимит 10к наггетсов
                    cout << "Толстяк " << id + 1 << " самоуничтожился!\n";
                    fatmen_destroyed++;
                    destroyed_or_starved = true;
                }
            }
            else {
                cout << "У толстяка " << id + 1 << " закончились наггетсы! Кука уволили.\n";
                destroyed_or_starved = true;
            }
        }

        if (destroyed_or_starved) {
            // этот толстяк больше не участвует
            return;
        }

        // толстяк доел
        int prev = eaters_done_count.fetch_add(1);
        if (prev == 2) {
            eaters_done_count = 0;
            eaters_finished = true;
        }

        this_thread::sleep_for(chrono::milliseconds(1));
    }
}

void run_simulation(int g, int e, const string& scenario_name) {
    
    stop_simulation = false;
    fatmen_destroyed = 0;
    for (int i = 0; i < 3; ++i) {
        total_nuggets_eaten[i] = 0;
        dish[i] = 3000;
    }
    cook_just_added = false;
    eaters_finished = false;

    gluttony = g;
    efficiency_factor = e;

    cout << "\n=== " << scenario_name << " ===\n";
    cout << "gluttony = " << gluttony << ", efficiency_factor = " << efficiency_factor << "\n";

    thread cook(cook_run);
    thread fatmen[3] = {
        thread(fatman, 0),
        thread(fatman, 1),
        thread(fatman, 2)
    };

    
    while (!stop_simulation) {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    
    cook.join();
    for (int i = 0; i < 3; ++i) {
        fatmen[i].join();
    }

    
    bool someone_starved = false;
    for (int i = 0; i < 3; ++i) {
        if (total_nuggets_eaten[i] < 10000 && dish[i] < gluttony) {
            someone_starved = true;
        }
    }

    if (fatmen_destroyed == 3) {
        cout << "РЕЗУЛЬТАТ: Кук не получил зарплату.\n";
    }
    else if (someone_starved) {
        cout << "РЕЗУЛЬТАТ: Кука уволили.\n";
    }
    else {
        cout << "РЕЗУЛЬТАТ: Кук уволился сам после 5 дней работы.\n";
    }

    for (int i = 0; i < 3; ++i) {
        cout << "Толстяк " << i + 1 << " съел: " << total_nuggets_eaten[i] << " наггетсов\n";
    }
}

int main() {
    setlocale(LC_ALL, "RU");
    run_simulation(100, 10, "Условие 1: Кука уволили");
    run_simulation(50, 500, "Условие 2: Кук не получил зарплату");
    run_simulation(10, 20, "Условие 3: Кук уволился сам");

    return 0;
}