///
/// @file BranchInstruction.cpp
/// @brief 分支跳转指令
/// @author Syrix555 (2383402647@qq.com)
/// @version 1.0
/// @date 2025-05-11
///
/// @copyright Copyright (c) 2025
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-05-11 <td>1.0     <td>Syrix  <td>新建
/// </table>
///

#include "Function.h"
#include "Instruction.h"
#include "LabelInstruction.h"
#include "Value.h"
#include "VoidType.h"

#include "BranchInstruction.h"

///
/// @brief 分支跳转指令的构造函数
///
BranchInstruction::BranchInstruction(Function * _func,
                                     Value * _srcval,
                                     Instruction * _target1,
                                     Instruction * _target2)
	: Instruction(_func, IRInstOperator::IRINST_OP_BRANCH, VoidType::getType())
{
    addOperand(_srcval);
    target1 = static_cast<LabelInstruction *>(_target1);
    target2 = static_cast<LabelInstruction *>(_target2);
}

/// @brief 转换成字符串
/// @param str 转换后的字符串
void BranchInstruction::toString(std::string & str)
{
    Value * src = getOperand(0);
    
    str = "bc " + src->getIRName() + ", label " + target1->getIRName() + ", label " + target2->getIRName();
}

///
/// @brief 获取目标Label1指令
/// @return LabelInstruction* label指令
///
LabelInstruction * BranchInstruction::getTarget1() const
{
    return target1;
}

///
/// @brief 获取目标Label2指令
/// @return LabelInstruction* label指令
///
LabelInstruction * BranchInstruction::getTarget2() const
{
    return target2;
}