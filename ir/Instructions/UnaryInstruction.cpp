///
/// @file UnaryInstruction.cpp
/// @brief 一元操作指令
/// @author Syrix555 (2383402647@qq.com)
/// @version 1.0
/// @date 2025-05-03
///
/// @copyright Copyright (c) 2025
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-05-03 <td>1.0     <td>Syrix555  <td>新建
/// </table>
///
#include "UnaryInstruction.h"
#include "Instruction.h"
#include "Value.h"
#include <string>

/// @brief 构造函数
/// @param _op 操作符
/// @param _result 结果操作数
/// @param _srcval 源操作数
UnaryInstruction::UnaryInstruction(Function * _func,
                                   IRInstOperator _op,
                                   Value * _srcval,
                                   Type * _type)
	: Instruction(_func, _op, _type)
{
    addOperand(_srcval);
}

/// @brief 转换成字符串
/// @param str 转换后的字符串
void UnaryInstruction::toString(std::string & str)
{
    Value * src = getOperand(0);

    switch (op) {
        case IRInstOperator::IRINST_OP_MINUS_I:

            // 求负指令，一元运算
            str = getIRName() + " = neg " + src->getIRName();
            break;

        default:
            // 未知指令
            Instruction::toString(str);
            break;
    }
}