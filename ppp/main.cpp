// 2-1 -- 1, 2, 4, 5
// 2-2 -- 1, 2
// 2-3 ---- 2, 3


#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include "DynamicQueue.h"

using namespace std;
int process_id_counter = 0;

class WaitQueue {
private:
    queue<Process> waitQueue;
public:
    void addProcess(Process p) {
        lock_guard<mutex> lock(mtx);
        waitQueue.push(p);
    }

    void wakeProcesses(int elapsedTime) {
        lock_guard<mutex> lock(mtx);
        queue<Process> tempQueue;
        while (!waitQueue.empty()) {
            Process p = waitQueue.front();
            waitQueue.pop();
            p.sleep_time -= elapsedTime;
            if (p.sleep_time <= 0) {
                // 프로세스를 다이나믹 큐에 다시 삽입
                cout << "Process " << p.id << " has woken up." << endl;
            }
            else {
                tempQueue.push(p);
            }
        }
        waitQueue = tempQueue;
    }

    void printWaitQueueState() {
        lock_guard<mutex> lock(mtx);
        queue<Process> tempQueue = waitQueue;
        while (!tempQueue.empty()) {
            Process p = tempQueue.front();
            tempQueue.pop();
            cout << p.id << (p.bg ? "B" : "F") << ":" << p.sleep_time << " ";
        }
        cout << "]" << endl;
    }
};

// 모니터 프로세스
void monitorProcess(DynamicQueue& dq, WaitQueue& wq, int interval) {
    while (true) {
        this_thread::sleep_for(chrono::seconds(interval));
        cout << "---------------------------" << endl;
        cout << "DQ: [ ";
        dq.printQueueState();
        cout << "---------------------------" << endl;
        cout << "WQ: [ ";
        wq.printWaitQueueState();
        cout << endl;
    }
}

// 셸 프로세스
void shellProcess(DynamicQueue& dq, WaitQueue& wq, vector<Process> commands, int sleepTime) {
    for (auto& cmd : commands) {
        this_thread::sleep_for(chrono::seconds(sleepTime));
        cout << "Running: [" << cmd.id << (cmd.bg ? "B" : "F") << "]" << endl;
        dq.enqueue(cmd);
        if (!cmd.bg) {
            // FG 프로세스는 실행 후 대기 큐에 추가
            wq.addProcess(cmd);
        }
    }
}

int main() {
    int threshold = 3;

    DynamicQueue dynamicQueue(threshold);
    WaitQueue waitQueue;

    vector<Process> commands = {
        Process(process_id_counter++, true),
        Process(process_id_counter++, false, 3),
        Process(process_id_counter++, true),
        Process(process_id_counter++, false, 2),
        Process(process_id_counter++, true),
        Process(process_id_counter++, false, 5)
    };

    // 모니터 프로세스 스레드 생성
    thread monitorThread(monitorProcess, ref(dynamicQueue), ref(waitQueue), 3);

    // 셸 프로세스 스레드 생성
    thread shellThread(shellProcess, ref(dynamicQueue), ref(waitQueue), commands, 2);

    monitorThread.join();
    shellThread.join();

    return 0;
}


/*
// ----------------------------------------------------------------------------------------
// 2-3

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <deque>
#include <algorithm>
#include <cstdio> // for fopen and fgets
#include <cstdlib> // for atoi

class CLI {
private:
    std::mutex mtx;
    int pid_counter;
    int interval;
    std::deque<std::string> dynamic_queue;
    std::deque<std::string> wait_queue;
    bool running;

public:
    CLI(int interval) : pid_counter(0), interval(interval), running(true) {}

    void shell(const std::string& filename) {
        FILE* file = std::fopen(filename.c_str(), "r");
        if (file == nullptr) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        char line[256];
        while (std::fgets(line, sizeof(line), file)) {
            std::this_thread::sleep_for(std::chrono::seconds(interval));
            execute_commands(line);
        }
        std::fclose(file);
        running = false;
    }

    void monitor(int monitor_interval) {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(monitor_interval));
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "---------------------------" << std::endl;
                std::cout << "Dynamic Queue: ";
                for (const auto& proc : dynamic_queue) {
                    std::cout << proc << " ";
                }
                std::cout << std::endl;
                std::cout << "Wait Queue: ";
                for (const auto& proc : wait_queue) {
                    std::cout << proc << " ";
                }
                std::cout << std::endl;
                std::cout << "---------------------------" << std::endl;
            }
        }
    }

    void execute_commands(const std::string& line) {
        char* token = std::strtok(const_cast<char*>(line.c_str()), ";");
        while (token != nullptr) {
            bool is_bg = std::strchr(token, '&') != nullptr;
            if (is_bg) {
                token = std::strtok(token, "&");
            }
            char** args = parse(token);
            exec(args, is_bg);
            free_args(args);

            token = std::strtok(nullptr, ";");
        }
    }

    char** parse(const char* command) {
        std::vector<char*> tokens;
        char* token = std::strtok(const_cast<char*>(command), " ");
        while (token != nullptr) {
            tokens.push_back(token);
            token = std::strtok(nullptr, " ");
        }
        char** args = new char* [tokens.size() + 1];
        for (size_t i = 0; i < tokens.size(); ++i) {
            args[i] = new char[std::strlen(tokens[i]) + 1];
            std::strcpy(args[i], tokens[i]);
        }
        args[tokens.size()] = nullptr; // 마지막 원소는 nullptr
        return args;
    }

    void exec(char** args, bool is_bg) {
        int n = 1, d = 1, p = 1, m = 1; // 시간은 1초로 가정
        for (int i = 1; args[i] != nullptr; ++i) {
            if (std::strcmp(args[i], "-n") == 0) {
                n = std::atoi(args[++i]);
            }
            else if (std::strcmp(args[i], "-d") == 0) {
                d = std::atoi(args[++i]);
            }
            else if (std::strcmp(args[i], "-p") == 0) {
                p = std::atoi(args[++i]);
            }
            else if (std::strcmp(args[i], "-m") == 0) {
                m = std::atoi(args[++i]);
            }
        }

        for (int i = 0; i < n; ++i) {
            int pid = pid_counter++;
            std::string proc_info = std::to_string(pid) + (is_bg ? "B" : "F");
            dynamic_queue.push_back(proc_info);
            if (std::strcmp(args[0], "echo") == 0) {
                std::thread(&CLI::echo, this, std::string(args[1]), p, d, pid, is_bg).detach();
            }
            else if (std::strcmp(args[0], "dummy") == 0) {
                std::thread(&CLI::dummy, this, d, pid, is_bg).detach();
            }
            else if (std::strcmp(args[0], "gcd") == 0) {
                int x = std::atoi(args[1]);
                int y = std::atoi(args[2]);
                std::thread(&CLI::gcd, this, x, y, d, pid, is_bg).detach();
            }
            else if (std::strcmp(args[0], "prime") == 0) {
                int x = std::atoi(args[1]);
                std::thread(&CLI::prime, this, x, d, pid, is_bg).detach();
            }
            else if (std::strcmp(args[0], "sum") == 0) {
                int x = std::atoi(args[1]);
                std::thread(&CLI::sum, this, x, m, d, pid, is_bg).detach();
            }
        }
    }

    void free_args(char** args) {
        for (int i = 0; args[i] != nullptr; ++i) {
            delete[] args[i];
        }
        delete[] args;
    }

    void echo(const std::string& message, int period, int duration, int pid, bool is_bg) {
        for (int i = 0; i < duration; ++i) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "Process " << pid << (is_bg ? "B" : "F") << ": echo " << message << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::seconds(period));
        }
        {
            std::lock_guard<std::mutex> lock(mtx);
            dynamic_queue.erase(std::remove(dynamic_queue.begin(), dynamic_queue.end(), std::to_string(pid) + (is_bg ? "B" : "F")), dynamic_queue.end());
        }
    }

    void dummy(int duration, int pid, bool is_bg) {
        for (int i = 0; i < duration; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Process " << pid << (is_bg ? "B" : "F") << ": Dummy process ended" << std::endl;
            dynamic_queue.erase(std::remove(dynamic_queue.begin(), dynamic_queue.end(), std::to_string(pid) + (is_bg ? "B" : "F")), dynamic_queue.end());
        }
    }

    void gcd(int a, int b, int duration, int pid, bool is_bg) {
        int result = gcd_calc(a, b);
        std::this_thread::sleep_for(std::chrono::seconds(duration));
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Process " << pid << (is_bg ? "B" : "F") << ": GCD of " << a << " and " << b << " is " << result << std::endl;
            dynamic_queue.erase(std::remove(dynamic_queue.begin(), dynamic_queue.end(), std::to_string(pid) + (is_bg ? "B" : "F")), dynamic_queue.end());
        }
    }

    int gcd_calc(int a, int b) {
        while (b != 0) {
            int t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    void prime(int n, int duration, int pid, bool is_bg) {
        int result = count_primes(n);
        std::this_thread::sleep_for(std::chrono::seconds(duration));
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Process " << pid << (is_bg ? "B" : "F") << ": Number of primes less than or equal to " << n << " is " << result << std::endl;
            dynamic_queue.erase(std::remove(dynamic_queue.begin(), dynamic_queue.end(), std::to_string(pid) + (is_bg ? "B" : "F")), dynamic_queue.end());
        }
    }

    int count_primes(int n) {
        std::vector<bool> is_prime(n + 1, true);
        is_prime[0] = is_prime[1] = false;
        for (int i = 2; i * i <= n; ++i) {
            if (is_prime[i]) {
                for (int j = i * i; j <= n; j += i) {
                    is_prime[j] = false;
                }
            }
        }
        return std::count(is_prime.begin(), is_prime.end(), true);
    }

    void sum(int n, int m, int duration, int pid, bool is_bg) {
        int result = sum_calc(n, m);
        std::this_thread::sleep_for(std::chrono::seconds(duration));
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Process " << pid << (is_bg ? "B" : "F") << ": Sum of integers up to " << n << " (divided by " << m << " threads) is " << result << std::endl;
            dynamic_queue.erase(std::remove(dynamic_queue.begin(), dynamic_queue.end(), std::to_string(pid) + (is_bg ? "B" : "F")), dynamic_queue.end());
        }
    }

    int sum_calc(int n, int m) {
        std::vector<std::thread> threads;
        std::vector<int> partial_sums(m, 0);
        int range = n / m;

        for (int i = 0; i < m; ++i) {
            int start = i * range + 1;
            int end = (i == m - 1) ? n : (i + 1) * range;
            threads.push_back(std::thread([start, end, &partial_sums, i]() {
                for (int j = start; j <= end; ++j) {
                    partial_sums[i] += j;
                }
                }));
        }

        for (auto& t : threads) {
            t.join();
        }

        int total_sum = 0;
        for (int sum : partial_sums) {
            total_sum += sum;
        }
        return total_sum % 1000000;
    }
};

int main() {
    CLI cli(1);

    std::thread shell_thread(&CLI::shell, &cli, "commands.txt");
    std::thread monitor_thread(&CLI::monitor, &cli, 1);

    shell_thread.join();
    monitor_thread.join();

    return 0;
}


*/