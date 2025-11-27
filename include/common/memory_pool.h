#ifndef COMMON_MEMORY_POOL_H
#define COMMON_MEMORY_POOL_H

#include <cstddef>
#include <stdexcept>

namespace common {

/**
 * @brief 通用内存池组件
 * @tparam T 内存池管理的对象类型（结构体或类）
 */
template <typename T>
class MemoryPool {
public:
    /**
     * @brief 构造函数
     * @param initial_size 初始节点个数
     * @throws std::bad_alloc 如果内存分配失败
     */
    explicit MemoryPool(size_t initial_size = 16);

    /**
     * @brief 析构函数
     */
    ~MemoryPool();

    /**
     * @brief 分配一个对象
     * @return 指向分配对象的指针
     * @throws std::bad_alloc 如果内存分配失败
     */
    T* allocate();

    /**
     * @brief 释放一个对象
     * @param ptr 指向要释放对象的指针
     * @throws std::invalid_argument 如果ptr无效
     */
    void deallocate(T* ptr);

    /**
     * @brief 构造一个对象
     * @tparam Args 构造函数参数类型
     * @param args 构造函数参数
     * @return 指向构造对象的指针
     * @throws std::bad_alloc 如果内存分配失败
     */
    template <typename... Args>
    T* construct(Args&&... args);

    /**
     * @brief 销毁一个对象
     * @param ptr 指向要销毁对象的指针
     * @throws std::invalid_argument 如果ptr无效
     */
    void destroy(T* ptr);

    /**
     * @brief 获取空闲节点个数
     * @return 空闲节点个数
     */
    size_t free_count() const;

    /**
     * @brief 获取总节点个数
     * @return 总节点个数
     */
    size_t total_count() const;

    /**
     * @brief 获取已使用节点个数
     * @return 已使用节点个数
     */
    size_t used_count() const;

    /**
     * @brief 禁止拷贝构造
     */
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    /**
     * @brief 禁止移动构造
     */
    MemoryPool(MemoryPool&&) = delete;
    MemoryPool& operator=(MemoryPool&&) = delete;

private:
    /**
     * @brief 内存池节点结构
     */
    struct Node {
        T data;           // 对象数据
        Node* next;       // 下一个节点指针
        Node* prev;       // 上一个节点指针
    };

    /**
     * @brief 分配内存
     * @param size 要分配的节点个数
     * @throws std::bad_alloc 如果内存分配失败
     */
    void allocate_memory(size_t size);

    const size_t m_initial_size;  // 初始节点个数
    size_t m_current_size;        // 当前节点个数
    size_t m_free_count;          // 空闲节点个数
    size_t m_total_count;         // 总节点个数
    
    Node* m_memory;               // 内存池起始地址
    Node* m_free_list;            // 空闲节点列表
    
    // 线程安全互斥锁（具体实现隐藏在cpp文件中）
    class MutexImpl;
    MutexImpl* m_mutex;           // 互斥锁指针
};

// 模板方法的内联实现
template <typename T>
template <typename... Args>
T* MemoryPool<T>::construct(Args&&... args) {
    T* ptr = allocate();
    try {
        // 在分配的内存上构造对象
        new (ptr) T(std::forward<Args>(args)...);
    } catch (...) {
        deallocate(ptr);
        throw;
    }
    return ptr;
}

} // namespace common

// 模板类的显式实例化声明（具体实例化在cpp文件中）
extern template class common::MemoryPool<int>;
extern template class common::MemoryPool<long>;
extern template class common::MemoryPool<float>;
extern template class common::MemoryPool<double>;
extern template class common::MemoryPool<std::string>;

#endif // COMMON_MEMORY_POOL_H
