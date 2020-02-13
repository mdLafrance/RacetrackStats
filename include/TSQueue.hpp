#pragma once

/*
    Thread Safe Queue class.

    Quick and dirty queue class which is thread-safe wrt reading and writing.
    Used to feed data to and from worker threads.

    Max Lafrance
*/

#include <queue>
#include <mutex>

template <class T>
class TSQueue {
    std::queue<T> queue;

    std::mutex* lock;

public:
    
    void push(const T& item);

    T pop();
    T peek() const;

    size_t size() const;

    TSQueue();
    ~TSQueue();
};

template <class T>
TSQueue<T>::TSQueue(){
    this->lock = new std::mutex();
}

template <class T>
TSQueue<T>::~TSQueue(){
    delete this->lock;
}

template <class T>
void TSQueue<T>::push(const T& item){
    std::lock_guard<std::mutex> lockGuard(*this->lock);
    this->queue.push(item);
}

template <class T>
T TSQueue<T>::pop(){
    std::lock_guard<std::mutex> lockGuard(*this->lock);
    T item = this->queue.pop();
    return item;
}

template <class T>
T TSQueue<T>::peek() const{
    std::lock_guard<std::mutex> lockGuard(*this->lock);
    T item = this->queue.front();
    return item;
}

template <class T>
size_t TSQueue<T>::size() const {
    std::lock_guard<std::mutex> lockGuard(*this->lock);
    size_t t = this->queue.size();
    return t;
}