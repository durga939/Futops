#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <atomic>
#include <csignal>
#include <chrono>

using namespace std;

atomic<bool> running(true);

mutex queueMutex;
condition_variable cv;
queue<string> logQueue;

mutex countMutex;
map<string, int> logCount;

// Handle Ctrl+C
void signalHandler(int signum) {
    cout << "\nSIGINT received. Exiting gracefully...\n";
    running = false;
    cv.notify_all();
}

// Thread 1: Read logs continuously
void logReader(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Unable to open log file\n";
        exit(1);
    }

    file.seekg(0, ios::end); // tail -f style

    string line;
    while (running) {
        while (getline(file, line)) {
            {
                lock_guard<mutex> lock(queueMutex);
                logQueue.push(line);
            }
            cv.notify_one();
        }
        this_thread::sleep_for(chrono::milliseconds(200));
        file.clear();
    }
    file.close();
}

// Thread 2: Process logs
void logProcessor() {
    ofstream errorFile("error.log", ios::app);

    while (running || !logQueue.empty()) {
        unique_lock<mutex> lock(queueMutex);
        cv.wait(lock, [] { return !logQueue.empty() || !running; });

        while (!logQueue.empty()) {
            string log = logQueue.front();
            logQueue.pop();
            lock.unlock();

            size_t pos1 = log.find("] [");
            size_t pos2 = log.find("]", pos1 + 3);

            if (pos1 == string::npos || pos2 == string::npos) {
                cerr << "Invalid log format: " << log << endl;
                lock.lock();
                continue;
            }

            string level = log.substr(pos1 + 3, pos2 - (pos1 + 3));

            {
                lock_guard<mutex> countLock(countMutex);
                logCount[level]++;
            }

            if (level == "ERROR") {
                errorFile << log << endl;
            }

            lock.lock();
        }
    }
    errorFile.close();
}

// Print summary every 10 seconds
void summaryPrinter() {
    while (running) {
        this_thread::sleep_for(chrono::seconds(10));
        lock_guard<mutex> lock(countMutex);

        cout << "\n--- Log Summary ---\n";
        for (auto &p : logCount) {
            cout << p.first << ": " << p.second << endl;
        }
        cout << "-------------------\n";
    }
}

int main() {
    signal(SIGINT, signalHandler);

    thread reader(logReader, "app.log");
    thread processor(logProcessor);
    thread summary(summaryPrinter);

    reader.join();
    processor.join();
    summary.join();

    cout << "Program exited cleanly.\n";
    return 0;
}
