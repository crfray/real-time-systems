
#include <string>
#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>
#include <mutex>

using namespace std;


//mutex m;

void Func(string name)
{
    long double i = 0;
    auto start = chrono::steady_clock::now();

    while (true)
    {
        i += 1e-9L;  // прибавляем 10^(-9)
        auto now = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - start);
        if (elapsed.count() >= 1000) // 1 секунда
            break;
    }

    //m.lock();
    cout << name << ": " << i << endl;
    //m.unlock();
}

int main()
{
    thread thread1(Func, "Thread1");
    thread thread2(Func, "Thread2");
    thread thread3(Func, "Thread3");

    thread1.join();
    thread2.join();
    thread3.join();

    system("pause");
    return 0;
}