#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>

//В данном задании требуется реализовать небуферизованный канал, в который какие-то потоки могут класть данные, а другие потоки могут оттуда их читать.
//        В небуферизованном канале в один момент времени может храниться не более одного значения.

//Если поток хочет положить в канал новое значение (вызывает метод Put), но в канале все еще лежит другое непрочитанное значение, то поток должен
//быть заблокирован до того момента, когда старое значение прочитают.

//После того как поток положил в канал значение, он блокируется до тех пор как другой поток прочитает положенное значение.

//Если поток хочет прочитать значение (вызывает метод Get), но значения в канале еще нет, то поток должен быть заблокирован до того момента, пока
//        в канал не положат значение. Максимальное время ожидания может быть задано с помощью аргумента при вызове метода Get.

//Если какой-то поток получил значение из канала, то ни этот поток ни другие потоки больше не смогут получить это же самое значение из канала.

class TimeOut : public std::exception {
    const char* what() const noexcept override {
        return "Timeout";
    }
};

template<typename T>
class UnbufferedChannel {
public:
    void Put(const T& data) {
        std::unique_lock<std::mutex> guard(mtx_);

        while(data_){
            push_.wait(guard);
        }
        data_ = data;
        pull_.notify_one();
        while (data_){
            push_.wait(guard);
        }
    }

    T Get(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
        std::unique_lock<std::mutex> guard(mtx_);

        auto waiting_start = std::chrono::high_resolution_clock::now();
        while (!data_){
            pull_.wait_for(guard, timeout);
            if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - waiting_start) > timeout) throw TimeOut();
        }

        T temp = *data_;
        data_ = std::nullopt;

        push_.notify_all();
        return temp;
    }
private:
    std::optional<T> data_ = std::nullopt;
    std::condition_variable push_;
    std::condition_variable pull_;
    std::mutex mtx_;
};
