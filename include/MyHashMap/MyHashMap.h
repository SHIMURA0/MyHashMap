//
// Created by Shimura on 2025/8/27.
//

// #ifndef MINI_HASHTABLE_MINI_HASHTABLE_H
// #define MINI_HASHTABLE_MINI_HASHTABLE_H
//
// #endif //MINI_HASHTABLE_MINI_HASHTABLE_H

// 防止头文件被重复包含，这是现代 C++ 更推荐的做法
#pragma once

#include <concepts>
#include <functional>
#include <string>
#include <vector>

// C++20 Concepts: 定义一个合格的 Key 需要满足什么条件
template <typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<size_t>;
};

template <typename T>
concept EqualityComparable = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
};

// 使用 Concepts 来约束模板参数 K
template <typename K, typename V>
    requires Hashable<K> && EqualityComparable<K>
class MyHashMap
{
  public:
    // 构造函数
    explicit MyHashMap(size_t initial_buckets = 16) : m_size(0)
    {
        // 使用成员初始化列表将 m_size 初始化为 0，这是最高效的方式。

        // 为桶数组分配初始空间，并将所有桶都初始化为空指针。
        // 这表示，刚创建的哈希表，所有桶都是空的，没有任何链表。
        m_buckets.resize(initial_buckets, nullptr);
    }

    // 析构函数
    ~MyHashMap() = default;

  private:
    // Node 是哈希表存储数据的基本单元，代表链表中的一个节点
    struct Node
    {
        // --- 成员变量 ---

        K key;      // 存储键。类型 K 由模板参数决定。
        V value;    // 存储值。类型 V 由模板参数决定。
        Node *next; // 指向下一个节点的指针，用于形成链表来解决哈希冲突。
        // 如果这是链表的最后一个节点，它将是 nullptr。

        // --- 构造函数 ---

        // 一个方便的构造函数，用于在创建节点时直接初始化键、值和 next 指针。
        // 参数 k 和 v 使用 const 引用，以避免不必要的拷贝，提高效率。
        Node(const K &k, const V &v) : key(k), value(v), next(nullptr)
        {
            // 构造函数的主体是空的，因为所有的初始化工作都在 C++ 更推荐的
            // “成员初始化列表”（冒号后面的部分）中完成了。
        }
    };

    // “桶”数组。它是一个动态数组，每个元素都是一个指向Node 链表头部的指针。
    std::vector<Node *> m_buckets;
    // 当前存储的键值对总数。
    size_t m_size;
};