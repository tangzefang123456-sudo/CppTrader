#include "trader/common/memory_pool.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <string>

namespace trader {
namespace common {

// 构造函数实现
template <typename T>
MemoryPool<T>::MemoryPool(size_t initial_size) 
    : free_list_(nullptr), 
      memory_blocks_(nullptr), 
      block_size_(initial_size), 
      total_nodes_(0), 
      allocated_nodes_(0) {
    if (initial_size == 0) {
        throw std::invalid_argument("Initial size must be greater than 0");
    }
    Expand();
}

// 析构函数实现
template <typename T>
MemoryPool<T>::~MemoryPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 释放所有内存块
    void* current = memory_blocks_;
    while (current != nullptr) {
        void* next = static_cast<void**>(current)[0];
        free(current);
        current = next;
    }
}

// 分配节点实现
template <typename T>
MemoryPoolNode<T>* MemoryPool<T>::Allocate() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 如果空闲列表为空，扩展内存池
    if (free_list_ == nullptr) {
        Expand();
    }
    
    // 从空闲列表中取出第一个节点
    MemoryPoolNode<T>* node = free_list_;
    free_list_ = free_list_->next;
    
    // 如果新的空闲列表不为空，更新其prev指针
    if (free_list_ != nullptr) {
        free_list_->prev = nullptr;
    }
    
    // 重置节点指针
    node->next = nullptr;
    node->prev = nullptr;
    
    allocated_nodes_++;
    return node;
}

// 释放节点实现
template <typename T>
void MemoryPool<T>::Deallocate(MemoryPoolNode<T>* node) {
    if (node == nullptr) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 将节点放回空闲列表头部
    node->next = free_list_;
    if (free_list_ != nullptr) {
        free_list_->prev = node;
    }
    node->prev = nullptr;
    free_list_ = node;
    
    allocated_nodes_--;
}

// 获取当前节点数量实现
template <typename T>
size_t MemoryPool<T>::Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return total_nodes_;
}

// 获取已分配节点数量实现
template <typename T>
size_t MemoryPool<T>::AllocatedSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return allocated_nodes_;
}

// 扩展内存池实现
template <typename T>
void MemoryPool<T>::Expand() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 计算新内存块的大小
    size_t block_size_bytes = block_size_ * sizeof(MemoryPoolNode<T>) + sizeof(void*);
    
    // 分配新内存块
    void* new_block = malloc(block_size_bytes);
    if (new_block == nullptr) {
        throw std::bad_alloc();
    }
    
    // 将新内存块添加到内存块列表头部
    static_cast<void**>(new_block)[0] = memory_blocks_;
    memory_blocks_ = new_block;
    
    // 获取新内存块中节点的起始地址
    MemoryPoolNode<T>* node_start = static_cast<MemoryPoolNode<T>*>(static_cast<void*>(static_cast<char*>(new_block) + sizeof(void*)));
    
    // 初始化新内存块中的所有节点
    for (size_t i = 0; i < block_size_; ++i) {
        MemoryPoolNode<T>* node = &node_start[i];
        node->next = (i == block_size_ - 1) ? nullptr : &node_start[i + 1];
        node->prev = (i == 0) ? nullptr : &node_start[i - 1];
    }
    
    // 将新节点添加到空闲列表头部
    if (free_list_ != nullptr) {
        free_list_->prev = &node_start[block_size_ - 1];
        node_start[block_size_ - 1].next = free_list_;
    }
    free_list_ = &node_start[0];
    
    // 更新节点总数
    total_nodes_ += block_size_;
    
    // 下次扩展时，内存块大小翻倍
    block_size_ *= 2;
}

// 模板类的显式实例化
template class MemoryPool<int>;
template class MemoryPool<long>;
template class MemoryPool<float>;
template class MemoryPool<double>;
template class MemoryPool<std::string>;

} // namespace common
} // namespace trader
