///
/// @file StoreInstruction.cpp
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

#include "Value.h"
#include "VoidType.h"
#include <string>

#include "StoreInstruction.h"

///
/// @brief 构造函数
/// @param _func 所属的函数
/// @param _result 结果操作数
/// @param _address 地址
///
StoreInstruction::StoreInstruction(Function * _func, Value * _address, Value * _result)
    : Instruction(_func, IRInstOperator::IRINST_OP_STORE, VoidType::getType())
{
    addOperand(_address);
    addOperand(_result);
}

/// @brief 转换成字符串显示
/// @param str 转换后的字符串
void StoreInstruction::toString(std::string & str)
{
    Value  *addr = getOperand(0), *dstVal = getOperand(1);

    str = "*" + addr->getIRName() + " = " + dstVal->getIRName();
}