#ifndef _SAFE_QUEUE_H
#define _SAFE_QUEUE_H

#include <iostream>
#include <string>
#include <unistd.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <initializer_list>

namespace sfq{
    template<typename T>
    class Safe_Queue{
    private:
        mutable std::mutex _mutex;
        mutable std::condition_variable _cond;
        using queue_type = std::queue<T>;
        queue_type queue_data;

    public:
        using val_type = typename queue_type::value_type;
        using cont_type = typename queue_type::container_type;
        Safe_Queue() = default;
        Safe_Queue(const Safe_Queue&) = delete;
        Safe_Queue& operator = (const Safe_Queue&) = delete;

        explicit Safe_Queue(const cont_type &c):queue_data(c){}
        Safe_Queue(std::initializer_list<val_type> list):Safe_Queue(list.begin(),list.end()){}

        // 将元素加入队列
        void push(const val_type &new_val){
            std::lock_guard<std::mutex> lk(_mutex);
            queue_data.push(std::move(new_val));
            _cond.notify_one();
        }

        // 从队列中弹出一个元素,如果队列为空就阻塞
        val_type wait_and_pop(){
            std::unique_lock<std::mutex>lk(_mutex);
            _cond.wait(lk,[this]{return !this->queue_data.empty();});
            auto value=std::move(queue_data.front());
            queue_data.pop();
            return value;
        }

        // 尝试从队列中弹出一个元素,如果队列为空返回false
        bool try_pop(val_type &value){
            std::lock_guard<std::mutex>lk(_mutex);
            if(queue_data.empty())
                return false;
            value=std::move(queue_data.front());
            queue_data.pop();
            return true;
        }

        // 返回队列是否为空，若为空返回 true
        auto Is_empty() const->decltype(queue_data.empty()) {
            std::lock_guard<std::mutex>lk(_mutex);
            return queue_data.empty();
        }

        // 返回队列中元素个数
        auto size() const->decltype(queue_data.size()){
            std::lock_guard<std::mutex>lk(_mutex);
            return queue_data.size();
        }
    };
}//BASE

#endif //_SAFE_QUEUE_H
