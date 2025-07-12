#include "observer.h"
#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>
#include <vector>
using namespace design_pattern;
void test_concurrent_observers() {
    auto subject = std::make_shared<Subject>();
    auto subject_other = std::make_shared<Subject>();

    std::vector<std::shared_ptr<Observer>> observers;
    for (int i = 0; i < 5; i++) {
        auto observer = design_pattern::Observer::create("Observer-" + std::to_string(i + 1));
        auto cb = [i](const std::string &msg) {
            printf("subject[1], observer[%d], msg[%s]\n", i, msg.c_str()); 
        };

        auto cb_other = [i](const std::string &msg) {
            printf("subject[2], observer[%d], msg[%s]\n", i, msg.c_str()); 
        };
        observer->set_callback(subject, cb);
        
        observer->set_callback(subject_other, cb_other);
        observers.emplace_back(observer);
    }
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; i++) {
        threads.emplace_back([subject, i] {
            for (int j = 0; j < 3; j++) {
                subject->create_message("Thread-" + std::to_string(i+1) + 
                                        " Message-" + std::to_string(j+1));
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }
    
    subject_other->create_message("MainThread Message-0");

    threads.emplace_back([&observers] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!observers.empty()) {
            auto obs = observers.back();
            obs->remove_self();
            observers.pop_back();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    subject_other->create_message("MainThread Message-1");

    for(auto &t : threads) {
        t.join();
    }

    subject->create_message("final message");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
int main() {
    test_concurrent_observers();
    return 0;
}