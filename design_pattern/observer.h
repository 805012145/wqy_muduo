#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace design_pattern {
    class IObserver;
    class ISubject : public std::enable_shared_from_this<ISubject> {
    public:
        virtual ~ISubject() {};
        virtual void attach(const std::weak_ptr<IObserver>& observer) {}
        virtual void detach(const std::weak_ptr<IObserver>& observer) {}
        virtual void notify() = 0;
    };
    class IObserver : public std::enable_shared_from_this<IObserver> {
    public:
        virtual ~IObserver() {};
        virtual void update(std::shared_ptr<ISubject> subject, const std::string &msg) = 0;
    };
    class Subject : public ISubject {
    public:
        Subject() : notification_thread_(&Subject::notification_worker, this) {}
        virtual ~Subject() {
            {
                std::unique_lock<std::mutex> lock(notification_mutex_);
                shutdown_ = true;
                notification_cond_.notify_one();
            }
            notification_thread_.join();
            std::cout << "Subject destroyed\n";
        }
        void attach(const std::weak_ptr<IObserver>& observer) override {
            observers_.push_back(observer);
        }
        void detach(const std::weak_ptr<IObserver>& observer) override {
            auto it = std::find_if(observers_.begin(), observers_.end(), 
            [&](const std::weak_ptr<IObserver> &w) {
                return !w.expired() && w.lock() == observer.lock();
            });
            if (it != observers_.end()) {
                observers_.erase(it);
            }
        }
        void notify() override {
            std::unique_lock<std::mutex> lock(notification_mutex_);
            notification_queue_.push_back(current_message_);
            notification_cond_.notify_one();
        }
        void create_message(const std::string& msg) {
            std::lock_guard<std::mutex> lock(msg_mutex_);
            current_message_ = msg;
            notify();
        }
    private:
        void notification_worker() {
            while (true) {
                std::string msg;
                {
                    std::unique_lock<std::mutex> lock(notification_mutex_);
                    notification_cond_.wait(lock, [this] {
                        return !notification_queue_.empty() || shutdown_;
                    });

                    if (shutdown_ && notification_queue_.empty()) {
                        return;
                    }
                    if (!notification_queue_.empty()) {
                        msg = notification_queue_.front();
                        notification_queue_.pop_front();
                    }
                }
                if (!msg.empty()) {
                    std::unique_lock<std::mutex> lock(observers_mutex_);
                    auto it = observers_.begin();
                    while (it != observers_.end()) {
                        if (auto ob = it->lock()) {
                            try {
                                ob->update(shared_from_this(), msg);
                            } catch (const std::exception &e) {
                                std::cerr << "Exception in observer update: " << e.what() << "\n";
                            }
                            ++it;
                        } else {
                            std::cout << "has distoried\n";
                            it = observers_.erase(it);
                        }
                    }
                }
            }
        }
        std::list<std::weak_ptr<IObserver>> observers_;
        std::mutex observers_mutex_;

        std::string current_message_;
        std::mutex msg_mutex_;

        std::condition_variable notification_cond_;
        std::list<std::string> notification_queue_;
        std::mutex notification_mutex_;
        std::thread notification_thread_;
        std::atomic<bool> shutdown_{false};

    };
    class Observer : public IObserver {
    public:
        typedef std::function<void(const std::string &msg)> EventCallback;
        static std::shared_ptr<Observer> create(const std::string& name) {
            auto ob = std::shared_ptr<Observer>(new Observer(name));
            return ob;
        }
        virtual ~Observer() {
            remove_self();
            std::cout << "Goodbye, I was the Observer \"" << name_ << "\".\n";
        }

        void remove_self() {
            // 从所有subject中移除自身
            auto it = callback_map_.begin();
            while (it != callback_map_.end()) {
                if (auto subject = it->first.lock()) {
                    // 调用subject的detach，移除自己
                    subject->detach(weak_from_this());
                } 
                it = callback_map_.erase(it);
            }
        }

        void update(std::shared_ptr<ISubject> subject, const std::string &msg) override {
            auto it = callback_map_.find(subject);
            if (it != callback_map_.end()) {
                auto cb = it->second;
                cb(msg);
            } else {
                std::cout << "not found\n";
            }
        }
        
        void remove_from_subject(const std::shared_ptr<Subject>& subject) {
            // remove from this from subject list
            auto it = callback_map_.find(subject);
            if (auto subject = it->first.lock()) {
                // 调用subject的detach，移除自己
                subject->detach(weak_from_this());
            }
            it = callback_map_.erase(it);
        }

        void set_callback(std::shared_ptr<Subject> subject, EventCallback cb) {
            std::cout << "Observer " + name_ << " set_callback\n";
            callback_map_[subject] = cb;
            subject->attach(weak_from_this());
        }
    private:
        Observer(const std::string& name) : name_(name) {
            std::cout << "Observer \"" << name_ << "\" created\n";
        }
        std::map<std::weak_ptr<ISubject>, EventCallback, std::owner_less<std::weak_ptr<ISubject>>> callback_map_;
        std::string name_;
    };
}