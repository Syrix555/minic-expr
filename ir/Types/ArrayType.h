///
/// @file ArrayType.h
/// @brief 数组类型定义
/// @author Syrix555 (2383402647@qq.com)
/// @version 1.0
/// @date 2025-06-08
///
/// @copyright Copyright (c) 2025
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-06-08 <td>1.0     <td>Syrix  <td>新建
/// </table>
///
#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include "Type.h"
#include "StorageSet.h"

#define Instanceof(res, type, var) auto res = dynamic_cast<type>(var)

///
/// @brief 数组类型
///
class ArrayType final : public Type {

    ///
    /// @brief Hash用结构体，为ArrayType提供哈希函数
    ///
    struct ArrayTypeHasher final {
        size_t operator()(const ArrayType &type) const noexcept {
            // 一个好的哈希函数应该结合所有决定类型唯一性的成员。
            // 这里我们结合元素类型指针的哈希值和元素数量的哈希值。
            size_t h1 = std::hash<const Type *>{}(type.getElementType());
            size_t h2 = std::hash<uint64_t>{}(type.getNumElements());
            // 简单地将两个哈希值组合起来
            return h1 ^ (h2 << 1);
        }
    };

    ///
    /// @brief 判断两者相等的结构体，为ArrayType提供等于函数
    ///
    struct ArrayTypeEqual final {
        size_t operator()(const ArrayType &lhs, const ArrayType &rhs) const noexcept {
            // 两个数组类型相等，当且仅当它们的元素类型和元素数量都相等。
            return lhs.getElementType() == rhs.getElementType() &&
                   lhs.getNumElements() == rhs.getNumElements();
        }
    };

public:
    ///
    /// @brief 获取（或创建）一个唯一的数组类型实例
    /// @param elementType 数组元素的类型
    /// @param numElements 数组元素的数量
    /// @return const ArrayType* 指向唯一的数组类型实例的指针
    ///
    static const ArrayType * get(Type * elementType, uint64_t numElements)
    {
        static StorageSet<ArrayType, ArrayTypeHasher, ArrayTypeEqual> storageSet;
        return storageSet.get(elementType, numElements);
    }

    ///
    /// @brief 获取数组的元素类型（只展开一层）
    /// @return const Type*
    ///
    [[nodiscard]] const Type * getElementType() const
    {
        return elementType;
    }
    
    ///
    /// @brief 获取数组的元素数量
    /// @return uint64_t
    ///
    [[nodiscard]] uint64_t getNumElements() const
    {
        return numElements;
    }

    ///
    /// @brief 返回根类型，也就是所有维度都展开后的最终元素类型
    /// @return const Type*
    ///
    [[nodiscard]] const Type * getRootType() const
    {
        return rootType;
    }

    ///
    /// @brief 返回数组的维度
    /// @return int32_t
    ///
    [[nodiscard]] int32_t getDepth() const
    {
        return depth;
    }

    ///
    /// @brief 获取类型的IR字符串表示
    /// @return std::string
    ///
    [[nodiscard]] std::string toString() const override
    {
        return rootType->toString();
    }

    /// @brief 获取类型维度后缀字符串
    /// @return 字符串
    [[nodiscard]] std::string getDimString() const override
    {
        std::string nestedDim = elementType->getDimString();
        return "[" + std::to_string(numElements) + "]" + nestedDim;
	}

    ///
    /// @brief 获得类型所占内存空间大小（字节）
    /// @return int32_t
    ///
    [[nodiscard]] int32_t getSize() const override
    {
        return static_cast<int32_t>(numElements) * elementType->getSize();
    }

private:
    ///
    /// @brief ArrayType的私有构造函数，只能被StorageSet调用
    /// @param _elementType 数组元素的类型
    /// @param _numElements 数组元素的数量
    ///
    ArrayType(const Type* _elementType, uint64_t _numElements)
        : Type(Type::ArrayTyID),
          elementType(_elementType),
          numElements(_numElements) {

        // 模仿 PointerType 的逻辑来计算 rootType 和 depth
        if (elementType->isArrayType()) {
            Instanceof(elementAsArrayType, const ArrayType*, elementType);
            this->rootType = elementAsArrayType->getRootType();
            this->depth = elementAsArrayType->getDepth() + 1;
        } else {
            this->rootType = this->elementType;
            this->depth = 1;
        }
    }

    ///
    /// @brief 数组的直接元素类型
    /// 例如：数组类型 [10 x [20 x i32]] 的元素类型是 [20 x i32]
    ///
    const Type* elementType;

    ///
    /// @brief 数组的元素数量
    ///
    uint64_t numElements;

    ///
    /// @brief 所有维度都展开后指向的元素类型，不再是数组类型
    /// 例如：数组类型 [10 x [20 x i32]] 的根类型为 i32
    ///
    const Type* rootType;

    ///
    /// @brief 数组的维度
    /// 例如：数组类型 [10 x [20 x i32]] 的深度为 2
    ///
    int32_t depth = 1;
};