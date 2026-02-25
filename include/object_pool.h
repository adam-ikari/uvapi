#ifndef UVAPI_OBJECT_POOL_H
#define UVAPI_OBJECT_POOL_H

#include <stack>
#include <mutex>
#include <memory>
#include <functional>
#include <deque>
#include <unordered_map>
#include <typeinfo>

namespace uvapi {

template<typename T>
class ObjectPool {
public:
    explicit ObjectPool(size_t initial_size = 10) 
        : max_size_(initial_size * 10) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (size_t i = 0; i < initial_size; ++i) {
            pool_.push(std::unique_ptr<T>(new T()));
        }
    }
    
    ~ObjectPool() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!pool_.empty()) {
            pool_.pop();
        }
    }
    
    std::unique_ptr<T, std::function<void(T*)>> acquire() {
        std::unique_ptr<T> obj;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!pool_.empty()) {
                obj = std::move(pool_.top());
                pool_.pop();
            } else {
                obj = std::unique_ptr<T>(new T());
            }
        }
        
        return std::unique_ptr<T, std::function<void(T*)>>(
            obj.release(),
            [this](T* ptr) {
                this->release(ptr);
            }
        );
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }
    
    size_t max_size() const {
        return max_size_;
    }
    
    void resize(size_t new_size) {
        std::lock_guard<std::mutex> lock(mutex_);
        while (pool_.size() > new_size && !pool_.empty()) {
            pool_.pop();
        }
        max_size_ = new_size * 10;
    }
    
private:
    void release(T* ptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pool_.size() < max_size_) {
            pool_.push(std::unique_ptr<T>(ptr));
        } else {
            delete ptr;
        }
    }
    
    mutable std::mutex mutex_;
    std::stack<std::unique_ptr<T>> pool_;
    size_t max_size_;
};

template<typename T>
class PoolAllocator {
public:
    using value_type = T;
    
    PoolAllocator(std::shared_ptr<ObjectPool<T>> pool)
        : pool_(pool) {}
    
    template<typename U>
    PoolAllocator(const PoolAllocator<U>& other)
        : pool_(other.pool_) {}
    
    T* allocate(size_t n) {
        if (n != 1) {
            return static_cast<T*>(::operator new(n * sizeof(T)));
        }
        auto ptr = pool_->acquire();
        T* raw = ptr.release();
        allocated_.push_back(raw);
        return raw;
    }
    
    void deallocate(T* p, size_t n) {
        if (n != 1) {
            ::operator delete(p);
            return;
        }
        auto it = std::find(allocated_.begin(), allocated_.end(), p);
        if (it != allocated_.end()) {
            allocated_.erase(it);
        }
    }
    
private:
    std::shared_ptr<ObjectPool<T>> pool_;
    std::deque<T*> allocated_;
};

class MemoryPoolManager {
public:
    static MemoryPoolManager& instance() {
        static MemoryPoolManager manager;
        return manager;
    }
    
    template<typename T>
    std::shared_ptr<ObjectPool<T>> getPool(size_t initial_size = 10) {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t type_hash = typeid(T).hash_code();
        
        auto it = pools_.find(type_hash);
        if (it == pools_.end()) {
            auto pool = std::shared_ptr<ObjectPool<T>>(new ObjectPool<T>(initial_size));
            pools_[type_hash] = pool;
            return pool;
        }
        
        return std::static_pointer_cast<ObjectPool<T>>(it->second);
    }
    
    template<typename T>
    std::unique_ptr<T, std::function<void(T*)>> acquire() {
        return getPool<T>()->acquire();
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        pools_.clear();
    }
    
    size_t poolCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pools_.size();
    }
    
private:
    MemoryPoolManager() {}
    ~MemoryPoolManager() {
        clear();
    }
    
    MemoryPoolManager(const MemoryPoolManager&) = delete;
    MemoryPoolManager& operator=(const MemoryPoolManager&) = delete;
    
    mutable std::mutex mutex_;
    std::unordered_map<size_t, std::shared_ptr<void>> pools_;
};

} 

#endif