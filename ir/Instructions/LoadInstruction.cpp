///
/// @file LoadInstruction.cpp
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

#include "Type.h"
#include "Value.h"
#include <string>

#include "LoadInstruction.h"

///
/// @brief 构造函数
/// @param _func 所属的函数
/// @param _address 地址
///
LoadInstruction::LoadInstruction(Function * _func, Value * _address, Type * _type)
    : Instruction(_func, IRInstOperator::IRINST_OP_LOAD, _type)
{
    addOperand(_address);
}

/// @brief 转换成字符串显示
/// @param str 转换后的字符串
void LoadInstruction::toString(std::string & str)
{
    Value *addr = getOperand(0);

    str = getIRName() + " = *" + addr->getIRName();
}