#include <gtest/gtest.h>
#include "object_pool.h"
#include <thread>
#include <vector>

using namespace uvapi;

class ObjectPoolTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

struct TestObject {
    int value;
    TestObject() : value(0) {}
    explicit TestObject(int v) : value(v) {}
};

TEST_F(ObjectPoolTest, AcquireAndRelease) {
    ObjectPool<TestObject> pool(5);
    
    EXPECT_EQ(pool.size(), 5);
    
    {
        auto obj = pool.acquire();
        EXPECT_EQ(pool.size(), 4);
        obj->value = 42;
    }
    
    EXPECT_EQ(pool.size(), 5);
}

TEST_F(ObjectPoolTest, MultipleAcquire) {
    ObjectPool<TestObject> pool(3);
    
    auto obj1 = pool.acquire();
    auto obj2 = pool.acquire();
    auto obj3 = pool.acquire();
    
    EXPECT_EQ(pool.size(), 0);
    
    auto obj4 = pool.acquire();
    EXPECT_EQ(pool.size(), 0);
    
    obj1.reset();
    EXPECT_EQ(pool.size(), 1);
}

TEST_F(ObjectPoolTest, MaxSizeLimit) {
    ObjectPool<TestObject> pool(5);
    
    std::vector<std::unique_ptr<TestObject, std::function<void(TestObject*)>>> objects;
    
    for (int i = 0; i < 20; ++i) {
        objects.push_back(pool.acquire());
    }
    
    EXPECT_EQ(pool.size(), 0);
    
    objects.clear();
    
    EXPECT_EQ(pool.size(), 50);
}

TEST_F(ObjectPoolTest, Resize) {
    ObjectPool<TestObject> pool(10);
    
    EXPECT_EQ(pool.size(), 10);
    EXPECT_EQ(pool.max_size(), 100);
    
    pool.resize(5);
    
    EXPECT_EQ(pool.max_size(), 50);
}

TEST_F(ObjectPoolTest, ThreadSafety) {
    ObjectPool<TestObject> pool(10);
    
    std::vector<std::thread> threads;
    const int num_threads = 10;
    const int operations_per_thread = 100;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&pool, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                auto obj = pool.acquire();
                obj->value = j;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(pool.size(), 10);
}

TEST_F(ObjectPoolTest, MemoryPoolManagerSingleton) {
    auto& manager = MemoryPoolManager::instance();
    
    auto pool1 = manager.getPool<TestObject>(5);
    auto pool2 = manager.getPool<TestObject>(5);
    
    EXPECT_EQ(pool1.get(), pool2.get());
}

TEST_F(ObjectPoolTest, MemoryPoolManagerAcquire) {
    auto& manager = MemoryPoolManager::instance();
    
    auto obj = manager.acquire<TestObject>();
    obj->value = 100;
    
    EXPECT_EQ(obj->value, 100);
}

TEST_F(ObjectPoolTest, MemoryPoolManagerPoolCount) {
    auto& manager = MemoryPoolManager::instance();
    manager.clear();
    
    EXPECT_EQ(manager.poolCount(), 0);
    
    manager.getPool<TestObject>();
    manager.getPool<int>();
    manager.getPool<std::string>();
    
    EXPECT_EQ(manager.poolCount(), 3);
}

TEST_F(ObjectPoolTest, MemoryPoolManagerClear) {
    auto& manager = MemoryPoolManager::instance();
    
    manager.getPool<TestObject>();
    manager.getPool<int>();
    
    EXPECT_GT(manager.poolCount(), 0);
    
    manager.clear();
    
    EXPECT_EQ(manager.poolCount(), 0);
}