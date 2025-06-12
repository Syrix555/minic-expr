///
/// @file LoadInstruction.h
/// @brief 加载指令定义
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
/// @brief 从内存地址加载数据的指令
///
class LoadInstruction : public Instruction {
public:
    ///
    /// @brief 构造函数
    /// @param _func 所属的函数
    /// @param address 要加载的内存地址
    ///
    LoadInstruction(Function * _func, Value * _address, Type * _type);

    /// @brief 转换成字符串
    void toString(std::string & str) override;
};