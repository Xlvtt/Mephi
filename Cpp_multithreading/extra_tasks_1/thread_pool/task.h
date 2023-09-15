#include <mutex>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <chrono>
#include <thread>

/*
 * Требуется написать класс ThreadPool, реализующий пул потоков, которые выполняют задачи из общей очереди.
 * С помощью метода PushTask можно положить новую задачу в очередь
 * С помощью метода Terminate можно завершить работу пула потоков.
 * Если в метод Terminate передать флаг wait = true,
 *  то пул подождет, пока потоки разберут все оставшиеся задачи в очереди, и только после этого завершит работу потоков.
 * Если передать wait = false, то все невыполненные на момент вызова Terminate задачи, которые остались в очереди,
 *  никогда не будут выполнены.
 * После вызова Terminate в поток нельзя добавить новые задачи.
 * Метод IsActive позволяет узнать, работает ли пул потоков. Т.е. можно ли подать ему на выполнение новые задачи.
 * Метод GetQueueSize позволяет узнать, сколько задач на данный момент ожидают своей очереди на выполнение.
 * При создании нового объекта ThreadPool в аргументах конструктора указывается количество потоков в пуле. Эти потоки
 *  сразу создаются конструктором.
 * Задачей может являться любой callable-объект, обернутый в std::function<void()>.
 */

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount) {
        for(size_t i=0; i<threadCount; ++i){
            threads_.emplace_back([&](){
                while(active_.load() || wait_.load()){
                    std::unique_lock<std::mutex> guard(mtx_);
                    if(!wait_.load()) break;
                    if(queue_.empty()){
                        if(!active_.load()) break;
                        cv_.wait(guard);
                    }
                    else{
                        auto task = queue_.front();
                        queue_.pop();
                        guard.unlock();
                        task();
                    }
                }
            });
        }
    }

    void PushTask(const std::function<void()>& task){
        std::lock_guard<std::mutex> guard(mtx_);
        if(!active_.load()) throw std::exception();
        queue_.push(task);
        cv_.notify_one(); //или notify_one ?
    }

    void Terminate(bool wait) {
        active_.store(false);
        if(!wait){
            wait_.store(false);
        }
        cv_.notify_all();

        for(auto& t: threads_){
            t.join();
        }
    }

    bool IsActive() const {
        return active_.load();
    }

    size_t QueueSize() const {
        std::lock_guard<std::mutex> guard(mtx_);
        return queue_.size();
    }
private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> queue_;
    std::atomic<bool> active_ = true;
    std::atomic<bool> wait_ = true;
    mutable std::mutex mtx_;
    std::condition_variable cv_;

};
