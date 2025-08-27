//
// Created by Shimura on 2025/8/27.
//

#include <iostream>
#include <string>

// 包含我们刚刚创建的哈希表头文件
#include "../include/MyHashMap/MyHashMap.h"

int main() {
    // 尝试创建一个我们的哈希表实例
    // 用 string 和 int 来实例化，它们都满足 Hashable 和 EqualityComparable 的要求
    MyHashMap<std::string, int> my_map;

    // 打印一句话，证明程序成功运行了
    std::cout << "MyHashMap project is running!" << std::endl;
    std::cout << "Successfully created an instance of MyHashMap." << std::endl;

    return 0;
}