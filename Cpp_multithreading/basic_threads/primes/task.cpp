#include "task.h"
#include <exception>
#include <cmath>
#include <vector>
#include <algorithm>
#include <thread>

PrimeNumbersSet::PrimeNumbersSet(): nanoseconds_under_mutex_(0), nanoseconds_waiting_mutex_(0){}

// Проверка, что данное число присутствует в множестве простых чисел
bool PrimeNumbersSet::IsPrime(uint64_t number) const{
    std::lock_guard<std::mutex> gw(set_mutex_);
    return (primes_.find(number) != primes_.end());
}

// Получить следующее по величине простое число из множества
uint64_t PrimeNumbersSet::GetNextPrime(uint64_t number) const{
    std::lock_guard<std::mutex> gw(set_mutex_);
    auto it = primes_.find(number);
    if (it == --(primes_.end())) throw std::invalid_argument("NO");
    return *(++it);
}

/*
 * Найти простые числа в диапазоне [from, to) и добавить в множество
 * Во время работы этой функции нужно вести учет времени, затраченного на ожидание лока мюьтекса,
 * а также времени, проведенного в секции кода под локом
 */
void PrimeNumbersSet::AddPrimesInRange(uint64_t from, uint64_t to){
    if (from<=2){
        auto t1 = std::chrono::high_resolution_clock::now();
        set_mutex_.lock();
        auto t2 = std::chrono::high_resolution_clock::now();
        primes_.insert(2);
        auto t3 = std::chrono::high_resolution_clock::now();
        set_mutex_.unlock();
        nanoseconds_waiting_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        nanoseconds_under_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(t3-t2).count();
        from=3;
    }
    else if (from%2==0) ++from;
    for(uint64_t num = from; num < to; num+=2){ //обработать единицу

        bool IsNumberPrime = true;
        if(num%2==0){IsNumberPrime = 0;}
        else{
             std::array<uint64_t, 5> arr = {2,3,5,7,11};
		     for(uint64_t i=0; i<5; ++i){
		        if(num%arr[i] == 0 && num!= arr[i]) {IsNumberPrime = false; goto Line;}
		     }
		     double sq = sqrt(static_cast<double>(num));
		     for(uint64_t i=1, temp=13; temp < 2*sqrt(static_cast<double>(num)); ++i, temp=12*i+1){
		        if (num%temp == 0) {IsNumberPrime = false; goto Line;}
		     }
		     for(uint64_t i=1, temp=17; temp < 2*sqrt(static_cast<double>(num)); ++i, temp=12*i+5){
		        if (num%temp == 0) {IsNumberPrime = false; goto Line;}
		     }
		     for(uint64_t i=1, temp=19; temp < 2*sqrt(static_cast<double>(num)); ++i, temp=12*i+7){
		        if (num%temp == 0) {IsNumberPrime = false; goto Line;}
		     }
		     for(uint64_t i=1, temp=23; temp < sq; ++i, temp=12*i+11){
		        if (num%temp == 0) {IsNumberPrime = false; goto Line;}
		     }
        }
		Line:
        if(IsNumberPrime){
            auto t1 = std::chrono::high_resolution_clock::now();
            set_mutex_.lock();
            auto t2 = std::chrono::high_resolution_clock::now();
            primes_.insert(num);
            auto t3 = std::chrono::high_resolution_clock::now();
            set_mutex_.unlock();
            nanoseconds_waiting_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
            nanoseconds_under_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(t3-t2).count();
        }
    }
}

// Посчитать количество простых чисел в диапазоне [from, to)
size_t PrimeNumbersSet::GetPrimesCountInRange(uint64_t from, uint64_t to) const{ //TODO корректность для маленьких диапазонов
    std::vector<uint64_t> lp(to, 0);
    std::vector<uint64_t> primes;

    if(lp[2] == 0){
        lp[2] = 2;
        primes.push_back(2);
    }
    for(uint64_t i=0; i<primes.size() && primes[i] <= lp[2] && 2*primes[i]<to; ++i){
        lp[2 * primes[i]] = primes[i];
    }

    
    for(uint64_t num=3; num<to; num+=2){
        if(lp[num] == 0){
            lp[num] = num;
            primes.push_back(num);
        }
        for(uint64_t i=0; i<primes.size() && primes[i] <= lp[num] && num*primes[i]<to; ++i){
            lp[num * primes[i]] = primes[i];
        }
    }
   	//return primes.size();
   	return std::distance(std::lower_bound(primes.begin(), primes.end(), from), primes.end());
}

// Получить наибольшее простое число из множества
uint64_t PrimeNumbersSet::GetMaxPrimeNumber() const{
    std::lock_guard<std::mutex> gw(set_mutex_);
    return *(--primes_.end());
}

// Получить суммарное время, проведенное в ожидании лока мьютекса во время работы функции AddPrimesInRange
std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeWaitingForMutex() const{
    return std::chrono::nanoseconds(nanoseconds_waiting_mutex_);
}

// Получить суммарное время, проведенное в коде под локом во время работы функции AddPrimesInRange
std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeUnderMutex() const{
    return std::chrono::nanoseconds(nanoseconds_under_mutex_);
}
