#include "../include/common/memory_pool.h"

// 跨平台线程安全实现
#ifdef _WIN32
#include <windows.h>
#define MEMORY_POOL_LOCK(mutex) EnterCriticalSection(&mutex)
#define MEMORY_POOL_UNLOCK(mutex) LeaveCriticalSection(&mutex)
#define MEMORY_POOL_INIT_LOCK(mutex) InitializeCriticalSection(&mutex)
#define MEMORY_POOL_DELETE_LOCK(mutex) DeleteCriticalSection(&mutex)
typedef CRITICAL_SECTION MemoryPoolMutex;
#else
#include <pthread.h>
#define MEMORY_POOL_LOCK(mutex) pthread_mutex_lock(&mutex)
#define MEMORY_POOL_UNLOCK(mutex) pthread_mutex_unlock(&mutex)
#define MEMORY_POOL_INIT_LOCK(mutex) pthread_mutex_init(&mutex, nullptr)
#define MEMORY_POOL_DELETE_LOCK(mutex) pthread_mutex_destroy(&mutex)
typedef pthread_mutex_t MemoryPoolMutex;
#endif

#include <cstring>
#include <stdexcept>

namespace common {

// 互斥锁实现类
template <typename T>
class MemoryPool<T>::MutexImpl {
public:
    MutexImpl() {
        MEMORY_POOL_INIT_LOCK(m_mutex);
    }

    ~MutexImpl() {
        MEMORY_POOL_DELETE_LOCK(m_mutex);
    }

    void lock() {
        MEMORY_POOL_LOCK(m_mutex);
    }

    void unlock() {
        MEMORY_POOL_UNLOCK(m_mutex);
    }

private:
    MemoryPoolMutex m_mutex;
};

// 构造函数
template <typename T>
MemoryPool<T>::MemoryPool(size_t initial_size)
    : m_initial_size(initial_size)
    , m_current_size(initial_size)
    , m_free_count(initial_size)
    , m_total_count(initial_size)
    , m_memory(nullptr)
    , m_free_list(nullptr)
    , m_mutex(new MutexImpl()) {
    allocate_memory(initial_size);
}

// 析构函数
template <typename T>
MemoryPool<T>::~MemoryPool() {
    m_mutex->lock();
    if (m_memory != nullptr) {
        // 释放所有内存块
        Node* current = reinterpret_cast<Node*>(m_memory);
        for (size_t i = 0; i < m_total_count; ++i) {
            // 调用对象的析构函数
            current->data.~T();
            ++current;
        }
        delete[] reinterpret_cast<char*>(m_memory);
        m_memory = nullptr;
    }
    m_mutex->unlock();
    delete m_mutex;
    m_mutex = nullptr;
}

// 分配一个对象
template <typename T>
T* MemoryPool<T>::allocate() {
    m_mutex->lock();
    
    if (m_free_list == nullptr) {
        // 没有空闲节点，双倍扩容
        size_t new_size = m_current_size * 2;
        allocate_memory(new_size);
        m_current_size = new_size;
    }

    // 从空闲列表中取出一个节点
    Node* node = m_free_list;
    m_free_list = node->next;
    if (m_free_list != nullptr) {
        m_free_list->prev = nullptr;
    }
    node->next = nullptr;
    node->prev = nullptr;
    --m_free_count;

    m_mutex->unlock();
    
    // 返回对象指针
    return &node->data;
}

// 释放一个对象
template <typename T>
void MemoryPool<T>::deallocate(T* ptr) {
    if (ptr == nullptr) {
        return;
    }

    m_mutex->lock();

    // 检查ptr是否在内存池中
    Node* node = reinterpret_cast<Node*>(reinterpret_cast<char*>(ptr) - offsetof(Node, data));
    char* node_ptr = reinterpret_cast<char*>(node);
    char* pool_start = reinterpret_cast<char*>(m_memory);
    char* pool_end = pool_start + m_total_count * sizeof(Node);

    if (node_ptr < pool_start || node_ptr >= pool_end) {
        m_mutex->unlock();
        throw std::invalid_argument("Invalid pointer: not in memory pool");
    }

    // 调用对象的析构函数
    node->data.~T();

    // 将节点放回空闲列表头部
    node->next = m_free_list;
    node->prev = nullptr;
    if (m_free_list != nullptr) {
        m_free_list->prev = node;
    }
    m_free_list = node;
    ++m_free_count;

    m_mutex->unlock();
}

// 销毁一个对象
template <typename T>
void MemoryPool<T>::destroy(T* ptr) {
    if (ptr == nullptr) {
        return;
    }
    ptr->~T();
    deallocate(ptr);
}

// 获取空闲节点个数
template <typename T>
size_t MemoryPool<T>::free_count() const {
    m_mutex->lock();
    size_t count = m_free_count;
    m_mutex->unlock();
    return count;
}

// 获取总节点个数
template <typename T>
size_t MemoryPool<T>::total_count() const {
    m_mutex->lock();
    size_t count = m_total_count;
    m_mutex->unlock();
    return count;
}

// 获取已使用节点个数
template <typename T>
size_t MemoryPool<T>::used_count() const {
    m_mutex->lock();
    size_t count = m_total_count - m_free_count;
    m_mutex->unlock();
    return count;
}

// 分配内存
template <typename T>
void MemoryPool<T>::allocate_memory(size_t size) {
    if (size == 0) {
        return;
    }

    // 分配内存
    size_t memory_size = size * sizeof(Node);
    char* new_memory = new char[memory_size];
    
    if (m_memory == nullptr) {
        // 第一次分配
        m_memory = reinterpret_cast<Node*>(new_memory);
    } else {
        // 扩容，复制原有数据
        char* old_memory = reinterpret_cast<char*>(m_memory);
        size_t old_memory_size = m_total_count * sizeof(Node);
        
        // 复制原有数据到新内存
        std::memcpy(new_memory, old_memory, old_memory_size);
        
        // 释放原有内存
        delete[] old_memory;
        
        m_memory = reinterpret_cast<Node*>(new_memory);
    }

    // 初始化新分配的节点
    Node* start = reinterpret_cast<Node*>(new_memory + m_total_count * sizeof(Node));
    Node* end = reinterpret_cast<Node*>(new_memory + memory_size);
    
    for (Node* node = start; node < end; ++node) {
        node->next = m_free_list;
        node->prev = nullptr;
        if (m_free_list != nullptr) {
            m_free_list->prev = node;
        }
        m_free_list = node;
    }

    m_total_count += size;
    m_free_count += size;
}

// 模板类的显式实例化
template class MemoryPool<int>;
template class MemoryPool<long>;
template class MemoryPool<float>;
template class MemoryPool<double>;
template class MemoryPool<std::string>;

} // namespace common
