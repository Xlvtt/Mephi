#pragma once
#include <atomic>
class SharedMutex {
public:
    void lock() {
        exclusive_locked_.store(true);
        while (locked_.exchange(true)){ //нахера тут shared_count?
            //locked_.wait(true);
        }
    }

    void unlock() {
        exclusive_locked_.store(false);
        locked_.store(false);
        //locked_.notify_all();
    }

    void lock_shared() {
        while (exclusive_locked_.load()){
            //exclusive_locked_.wait(true);
        }
        locked_.store(true);
        shared_count_.fetch_add(1);
    }

    void unlock_shared() {
        //кладем false только если размер ноль
        shared_count_.fetch_add(-1); //может ли уйти в минус?
        if(shared_count_.load() == 0){
            locked_.store(false);
        }
        //locked_.notify_all(); //нужно разбудить все
    }
private:
    std::atomic<bool> locked_ = false;
    std::atomic<int> shared_count_ = 0;
    std::atomic<bool> exclusive_locked_ = false;
};
