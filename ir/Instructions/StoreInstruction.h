///
/// @file StoreInstruction.h
/// @brief store指令
/// @author Syrix555 (2383402647@qq.com)
/// @version 1.0
/// @date 2025-06-11
///
/// @copyright Copyright (c) 2025
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-06-11 <td>1.0     <td>XXXXX  <td>内容
/// </table>
///

#pragma once

#include <string>

#include "Value.h"
#include "Instruction.h"

class Function;


///
/// @brief store指令
///
class StoreInstruction : public Instruction {
public:
    ///
    /// @brief 构造函数
    /// @param _func 所属的函数
    /// @param _address 要保存的内存地址
    /// @param _result 保存值的存放位置
    ///
    StoreInstruction(Function * _func, Value * _address, Value * _result);

    /// @brief 转换成字符串
    void toString(std::string & str) override;
};