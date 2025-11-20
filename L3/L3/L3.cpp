#include <iostream>
#include <thread>
#include <mutex>
#include <string>

std::mutex m;

int coins = 100;
const int N = coins; // делаем const, чтобы не изменялось в процессе
int Bob_coins = 0;
int Tom_coins = 0;

void coin_sharing(std::string name, int& thief_coins, int& companion_coins) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(m);

            // Если монеты уже кончились, то выходим
            if (coins <= 0) {
                break;
            }

            // При нечетном кол-ве, отдаем покойнику
            if (N % 2 != 0 && coins == 1) {
                std::cout << "===================" << std::endl;
                std::cout << "*Последняя монета достается покойнику* " << std::endl;
                coins = 0;
                break;
            }

            // Проверка честности дележки
            if (thief_coins <= companion_coins) {
                coins--;
                thief_coins++;
                std::cout << name << " :" << thief_coins << " " << companion_coins << std::endl;
            }
            // Если не может - ничего не делаем
        }
        // Небольшая пауза для правильной работы дележки
        std::this_thread::yield();
    }
}

int main() {

    setlocale(LC_ALL, "ru");
    std::thread bob(coin_sharing, "Bob", std::ref(Bob_coins), std::ref(Tom_coins));
    std::thread tom(coin_sharing, "Tom", std::ref(Tom_coins), std::ref(Bob_coins));

    bob.join();
    tom.join();

    std::cout << "\nРезультат:\n";
    std::cout << "Bob: " << Bob_coins << std::endl;
    std::cout << "Tom: " << Tom_coins << std::endl;
    std::cout << "Покойник: " << (N - Bob_coins - Tom_coins) << std::endl;

    return 0;
}