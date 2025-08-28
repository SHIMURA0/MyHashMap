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
        // unique_ptr 的默认构造就是空指针，所以我们不再需要第二个参数 nullptr
        m_buckets.resize(initial_buckets);
    }

    // 析构函数
    ~MyHashMap() = default;

    /**
     * @brief 在哈希表中插入一个键值对，或更新一个已存在的键。
     *
     * @details
     * 该函数实现了“插入或更新”(upsert)的逻辑。
     * 它首先会计算键(key)对应的桶索引，然后遍历该桶中的链表。
     * - 如果找到了一个与给定键匹配的节点，它会更新该节点的值(value)，并返回 false。
     * - 如果遍历完整个链表都没有找到匹配的键，它会创建一个新的节点，
     *   并使用高效的“头插法”将其插入到链表的头部。然后，它会增加哈希表的总大小计数，并返回 true。
     *
     * 这种设计确保了键的唯一性。
     *
     * @tparam K 键的类型。必须满足 Hashable 和 EqualityComparable 的概念约束。
     * @tparam V 值的类型。
     * @param key 要插入或更新的键。以常量引用的方式传递，以避免不必要的拷贝。
     * @param value 与键关联的值。以常量引用的方式传递。
     *
     * @return bool
     * - `true` 如果成功插入了一个全新的键值对。
     * - `false` 如果键已经存在，并且只是更新了其对应的值。
     *
     * @note
     * 这个实现完全基于 C++11/14/17 的智能指针 std::unique_ptr，保证了内存安全和自动资源管理 (RAII)。
     * 节点的创建使用了 std::make_unique，节点的链接和转移使用了 std::move，这都是现代 C++ 的最佳实践。
     *
     * @warning
     * 当前版本尚未实现自动扩容(rehash)逻辑。当负载因子过高时，性能会下降。
     *
     * @complexity
     * - 平均情况 (Average Case): O(1)，假设哈希函数能将键均匀分布。
     * - 最坏情况 (Worst Case): O(N)，其中 N 是哈希表中的元素总数。当所有键都哈希到同一个桶时发生。
     */
    bool insert(const K &key, const V &value)
    {
        // 步骤 1: 定位桶
        // 根据键的哈希值计算它应该落在哪个桶里。
        size_t index = get_bucket_index(key);

        // 为了遍历链表，我们从桶的头节点开始。
        // 使用 .get() 方法可以获取一个非拥有的裸指针(raw pointer)。
        // 这非常安全，因为我们只是用它来“观察”和“遍历”，而不会用它来修改所有权或删除节点。
        Node *current = m_buckets[index].get();

        // 步骤 2: 查找键是否存在
        // 遍历当前桶关联的整个链表。
        while (current != nullptr)
        {
            // 如果找到了匹配的键...
            // 在C++中，-> 操作符其实是一个“语法糖”（Syntactic
            // Sugar），它是一种更方便的写法，用来替代一个更繁琐的、两步走的操作。 current->key 完全等价于
            // (*current).key 通过解引用*current得到current这个内存地址的对象 (即一个Node对象)
            // 然后通过.key来访问这个Node对象的key属性
            if (current->key == key)
            {
                // ...就地更新该节点的值。
                current->value = value;
                // 操作完成，因为键已存在，所以返回 false。
                return false;
            }
            // 移动到链表中的下一个节点，继续查找。
            // 这里current ->next.get() 等价于 (*current).next.get() 因为Node对象中的next属性是一个智能指针
            // 我们需要使用.get()来获得其中被管理的裸指针
            current = current->next.get();
        }

        // 步骤 3: 插入新节点
        // 如果循环结束都没有返回，说明键在哈希表中不存在。我们需要执行插入操作。
        // 我们采用最高效的“头插法”(Head Insertion)。

        // 3.1: 创建一个由 std::unique_ptr 管理的新节点。
        // std::make_unique 是创建智能指针管理对象的首选方式，它异常安全。
        auto new_node = std::make_unique<Node>(key, value);

        // 3.2: 将新节点的 next 指针指向当前桶的头节点。
        // 这是头插法的关键一步：新节点将成为新的头，所以它的下一个节点必须是旧的头。
        // std::move(m_buckets[index]) 将桶中 unique_ptr 的“所有权”转移给了 new_node->next。
        // 执行后，m_buckets[index] 会变成空指针 nullptr。
        new_node->next = std::move(m_buckets[index]);

        // 3.3: 将桶的头指针指向新创建的节点。
        // 现在，让桶接管新节点的所有权。
        // std::move(new_node) 将 new_node 的“所有权”转移给了 m_buckets[index]。
        // 执行后，局部变量 new_node 会变成空指针 nullptr。
        m_buckets[index] = std::move(new_node);

        // 不要忘记将哈希表的总大小加一。
        m_size++;

        // TODO: 在这里添加逻辑，检查负载因子 (m_size / m_buckets.size()) 是否超过阈值，
        // 如果超过，则触发 rehash() 操作以进行扩容。

        // 成功插入了一个全新的元素，返回 true。
        return true;
    }

  private:
    // Node 是哈希表存储数据的基本单元，代表链表中的一个节点
    struct Node
    {
        // --- 成员变量 ---

        K key;   // 存储键。类型 K 由模板参数决定。
        V value; // 存储值。类型 V 由模板参数决定。
        // 不再使用裸指针！
        // next 现在是一个 unique_ptr，它独占地拥有下一个节点。
        // 这形成了一个所有权链条。
        std::unique_ptr<Node> next;

        // --- 构造函数 ---

        // 一个方便的构造函数，用于在创建节点时直接初始化键、值和 next 指针。
        // 参数 k 和 v 使用 const 引用，以避免不必要的拷贝，提高效率。
        // 构造函数保持不变，但 next 会被自动初始化为 nullptr
        Node(const K &k, const V &v) : key(k), value(v), next(nullptr)
        {
        }
    };

    // “桶”数组升级
    // vector 中存储的不再是 Node*，而是 std::unique_ptr<Node>。
    // vector 中的每个 unique_ptr 都独占地拥有其对应链表的“头节点”。
    std::vector<std::unique_ptr<Node>> m_buckets;

    // 当前存储的键值对总数。
    size_t m_size;

    size_t get_bucket_index(const K &key) const
    {
        return std::hash<K>{}(key) % m_buckets.size();
    }
};