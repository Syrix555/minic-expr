﻿///
/// @file InstSelectorArm32.cpp
/// @brief 指令选择器-ARM32的实现
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-21
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-21 <td>1.0     <td>zenglj  <td>新做
/// </table>
///
#include <cstdint>
#include <cstdio>

#include "ArrayType.h"
#include "BranchInstruction.h"
#include "Common.h"
#include "GlobalVariable.h"
#include "ILocArm32.h"
#include "InstSelectorArm32.h"
#include "Instruction.h"
#include "PlatformArm32.h"

#include "PointerType.h"
#include "RegVariable.h"
#include "Function.h"

#include "LabelInstruction.h"
#include "GotoInstruction.h"
#include "FuncCallInstruction.h"
#include "MoveInstruction.h"
#include "Type.h"
#include "Value.h"

/// @brief 构造函数
/// @param _irCode 指令
/// @param _iloc ILoc
/// @param _func 函数
InstSelectorArm32::InstSelectorArm32(vector<Instruction *> & _irCode,
                                     ILocArm32 & _iloc,
                                     Function * _func,
                                     SimpleRegisterAllocator & allocator)
    : ir(_irCode), iloc(_iloc), func(_func), simpleRegisterAllocator(allocator)
{
    translator_handlers[IRInstOperator::IRINST_OP_ENTRY] = &InstSelectorArm32::translate_entry;
    translator_handlers[IRInstOperator::IRINST_OP_EXIT] = &InstSelectorArm32::translate_exit;

    translator_handlers[IRInstOperator::IRINST_OP_LABEL] = &InstSelectorArm32::translate_label;
    translator_handlers[IRInstOperator::IRINST_OP_GOTO] = &InstSelectorArm32::translate_goto;

    translator_handlers[IRInstOperator::IRINST_OP_ASSIGN] = &InstSelectorArm32::translate_assign;

    translator_handlers[IRInstOperator::IRINST_OP_ADD_I] = &InstSelectorArm32::translate_add_int32;
    translator_handlers[IRInstOperator::IRINST_OP_SUB_I] = &InstSelectorArm32::translate_sub_int32;
    translator_handlers[IRInstOperator::IRINST_OP_MINUS_I] = &InstSelectorArm32::translate_minus_int32;
    translator_handlers[IRInstOperator::IRINST_OP_MUL_I] = &InstSelectorArm32::translate_mul_int32;
    translator_handlers[IRInstOperator::IRINST_OP_DIV_I] = &InstSelectorArm32::translate_div_int32;
    translator_handlers[IRInstOperator::IRINST_OP_MOD_I] = &InstSelectorArm32::translate_mod_int32;

    translator_handlers[IRInstOperator::IRINST_OP_GT_I] = &InstSelectorArm32::translate_gt_int32;
    translator_handlers[IRInstOperator::IRINST_OP_LT_I] = &InstSelectorArm32::translate_lt_int32;
    translator_handlers[IRInstOperator::IRINST_OP_GE_I] = &InstSelectorArm32::translate_ge_int32;
    translator_handlers[IRInstOperator::IRINST_OP_LE_I] = &InstSelectorArm32::translate_le_int32;
    translator_handlers[IRInstOperator::IRINST_OP_EQ_I] = &InstSelectorArm32::translate_eq_int32;
    translator_handlers[IRInstOperator::IRINST_OP_NE_I] = &InstSelectorArm32::translate_ne_int32;

    translator_handlers[IRInstOperator::IRINST_OP_BRANCH] = &InstSelectorArm32::translate_branch;

    translator_handlers[IRInstOperator::IRINST_OP_LOAD] = &InstSelectorArm32::translate_load;
    translator_handlers[IRInstOperator::IRINST_OP_STORE] = & InstSelectorArm32::translate_store;

    translator_handlers[IRInstOperator::IRINST_OP_FUNC_CALL] = &InstSelectorArm32::translate_call;
    translator_handlers[IRInstOperator::IRINST_OP_ARG] = &InstSelectorArm32::translate_arg;
}

///
/// @brief 析构函数
///
InstSelectorArm32::~InstSelectorArm32()
{}

/// @brief 指令选择执行
void InstSelectorArm32::run()
{
    for (auto inst: ir) {

        // 逐个指令进行翻译
        if (!inst->isDead()) {
            translate(inst);
        }
    }
}

/// @brief 指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate(Instruction * inst)
{
    // 操作符
    IRInstOperator op = inst->getOp();

    map<IRInstOperator, translate_handler>::const_iterator pIter;
    pIter = translator_handlers.find(op);
    if (pIter == translator_handlers.end()) {
        // 没有找到，则说明当前不支持
        printf("Translate: Operator(%d) not support", (int) op);
        return;
    }

    // 开启时输出IR指令作为注释
    if (showLinearIR) {
        outputIRInstruction(inst);
    }

    (this->*(pIter->second))(inst);
}

///
/// @brief 输出IR指令
///
void InstSelectorArm32::outputIRInstruction(Instruction * inst)
{
    std::string irStr;
    inst->toString(irStr);
    if (!irStr.empty()) {
        iloc.comment(irStr);
    }
}

/// @brief NOP翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_nop(Instruction * inst)
{
    (void) inst;
    iloc.nop();
}

/// @brief Label指令指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_label(Instruction * inst)
{
    Instanceof(labelInst, LabelInstruction *, inst);

    iloc.label(labelInst->getName());
}

/// @brief goto指令指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_goto(Instruction * inst)
{
    Instanceof(gotoInst, GotoInstruction *, inst);

    // 无条件跳转
    iloc.jump(gotoInst->getTarget()->getName());
}

/// @brief 函数入口指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_entry(Instruction * inst)
{
    // 查看保护的寄存器
    auto & protectedRegNo = func->getProtectedReg();
    auto & protectedRegStr = func->getProtectedRegStr();

    bool first = true;
    for (auto regno: protectedRegNo) {
        if (first) {
            protectedRegStr = PlatformArm32::regName[regno];
            first = false;
        } else {
            protectedRegStr += "," + PlatformArm32::regName[regno];
        }
    }

    if (!protectedRegStr.empty()) {
        iloc.inst("push", "{" + protectedRegStr + "}");
    }

    // 为fun分配栈帧，含局部变量、函数调用值传递的空间等
    iloc.allocStack(func, ARM32_TMP_REG_NO);
}

/// @brief 函数出口指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_exit(Instruction * inst)
{
    if (inst->getOperandsNum()) {
        // 存在返回值
        Value * retVal = inst->getOperand(0);

        // 赋值给寄存器R0
        iloc.load_var(0, retVal);
    }

    // 恢复栈空间
    iloc.inst("mov", "sp", "fp");

    // 保护寄存器的恢复
    auto & protectedRegStr = func->getProtectedRegStr();
    if (!protectedRegStr.empty()) {
        iloc.inst("pop", "{" + protectedRegStr + "}");
    }

    iloc.inst("bx", "lr");
}

/// @brief 赋值指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_assign(Instruction * inst)
{
    Value * result = inst->getOperand(0);
    Value * arg1 = inst->getOperand(1);

    int32_t arg1_regId = arg1->getRegId();
    int32_t result_regId = result->getRegId();

    if (arg1_regId != -1) {
        // 寄存器 => 内存
        // 寄存器 => 寄存器

        // r8 -> rs 可能用到r9
        iloc.store_var(arg1_regId, result, ARM32_TMP_REG_NO);
    } else if (result_regId != -1) {
        // 内存变量 => 寄存器
        if (arg1->getType()->isPointerType()) {
            Instanceof(pointer, PointerType *, arg1->getType());
            const Type * tmp = pointer->getPointeeType();
            if (Instanceof(array, const ArrayType *, tmp)) {
                if (array->getNumElements() == 0) {
                    iloc.load_var(result_regId, arg1);			// 说明传递的是函数形参中的数组类型，需要加载的是地址
                } else {
                    iloc.lea_var(result_regId, arg1);
				}
			} else {
				iloc.load_var(result_regId, arg1);
			}
        } else {
            iloc.load_var(result_regId, arg1);
		}
    } else {
        // 内存变量 => 内存变量

        int32_t temp_regno = simpleRegisterAllocator.Allocate();

        // arg1 -> r8
        if (arg1->getType()->isPointerType()) {
            Instanceof(pointer, PointerType *, arg1->getType());
            if (Instanceof(array, const ArrayType *, pointer->getPointeeType())) {
                if (array->getNumElements() == 0) {
                    iloc.load_var(temp_regno, arg1);			// 说明传递的是函数形参中的数组类型，需要加载的是地址
                } else {
                    iloc.lea_var(temp_regno, arg1);
				}
			} else {
				iloc.load_var(temp_regno, arg1);
			}
        } else {
            iloc.load_var(temp_regno, arg1);
		}

        // r8 -> rs 可能用到r9
        iloc.store_var(temp_regno, result, ARM32_TMP_REG_NO);

        simpleRegisterAllocator.free(temp_regno);
    }
}

/// @brief 一元操作指令翻译成ARM32汇编
/// @param inst IR指令
/// @param operator_name 操作码
/// @param rs_reg_no 结果寄存器号
/// @param op1_reg_no 源操作数1寄存器号
void InstSelectorArm32::translate_one_operator(Instruction * inst, string operator_name)
{
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t result_reg_no = inst->getRegId();
    int32_t load_result_reg_no, load_arg1_reg_no;

    // 看arg1是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg1_reg_no == -1) {

        // 分配一个寄存器r8
        load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);

        // arg1 -> r8，这里可能由于偏移不满足指令的要求，需要额外分配寄存器
        iloc.load_var(load_arg1_reg_no, arg1);
    } else {
        load_arg1_reg_no = arg1_reg_no;
    }

    // 看结果变量是否是寄存器，若不是则需要分配一个新的寄存器来保存运算的结果		备注：本身就不可能存在这种情况，比较像防御式编程
    if (result_reg_no == -1) {
        // 分配一个寄存器r10，用于暂存结果
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
    } else {
        load_result_reg_no = result_reg_no;
    }

    // r8 -> r10
    iloc.inst(operator_name,
              PlatformArm32::regName[load_result_reg_no],
              PlatformArm32::regName[load_arg1_reg_no]);

    // 结果不是寄存器，则需要把rs_reg_name保存到结果变量中
    if (result_reg_no == -1) {

        // 这里使用预留的临时寄存器，因为立即数可能过大，必须借助寄存器才可操作。

        // r10 -> result
        iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(arg1);
    simpleRegisterAllocator.free(result);
}

/// @brief 二元操作指令翻译成ARM32汇编
/// @param inst IR指令
/// @param operator_name 操作码
/// @param rs_reg_no 结果寄存器号
/// @param op1_reg_no 源操作数1寄存器号
/// @param op2_reg_no 源操作数2寄存器号
void InstSelectorArm32::translate_two_operator(Instruction * inst, string operator_name)
{
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = inst->getRegId();
    int32_t load_result_reg_no, load_arg1_reg_no, load_arg2_reg_no;

    // 看arg1是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    // 获取数组首地址时需要特别处理
    if (arg1->getType()->isPointerType()) {
        Instanceof(pointer, PointerType *, arg1->getType());
        if (pointer->getPointeeType()->isArrayType()) {
            load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);
            Instanceof(array, const ArrayType *, pointer->getPointeeType());
            if (array->getNumElements() == 0) {
				// 函数形参的话是地址保存在相应位置，读取即可
                iloc.load_var(load_arg1_reg_no, arg1);
            } else {
				// 其他情况下我们直接获取栈偏移找到数组首地址
                iloc.lea_var(load_arg1_reg_no, arg1);
			}
		} else {
			if (arg1_reg_no == -1) {

				// 分配一个寄存器r8
				load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);

				// arg1 -> r8，这里可能由于偏移不满足指令的要求，需要额外分配寄存器
				iloc.load_var(load_arg1_reg_no, arg1);
			} else {
				load_arg1_reg_no = arg1_reg_no;
			}
		}
    } else {
		if (arg1_reg_no == -1) {

			// 分配一个寄存器r8
			load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);

			// arg1 -> r8，这里可能由于偏移不满足指令的要求，需要额外分配寄存器
			iloc.load_var(load_arg1_reg_no, arg1);
		} else {
			load_arg1_reg_no = arg1_reg_no;
		}
	}

    if (arg2->getType()->isPointerType()) {
        Instanceof(pointer, PointerType *, arg2->getType());
        if (pointer->getPointeeType()->isArrayType()) {
            load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);
            Instanceof(array, const ArrayType *, pointer->getPointeeType());
            if (array->getNumElements() == 0) {
                // 函数形参的话是地址保存在相应位置，读取即可
                iloc.load_var(load_arg2_reg_no, arg2);
            } else {
				// 其他情况下我们直接获取栈偏移找到数组首地址
                iloc.lea_var(load_arg2_reg_no, arg2);
			}
		} else {
			if (arg2_reg_no == -1) {

				// 分配一个寄存器r8
				load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);

				// arg1 -> r8，这里可能由于偏移不满足指令的要求，需要额外分配寄存器
				iloc.load_var(load_arg2_reg_no, arg2);
			} else {
				load_arg2_reg_no = arg2_reg_no;
			}
		}
    } else {
		if (arg2_reg_no == -1) {

			// 分配一个寄存器r8
			load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);

			// arg1 -> r8，这里可能由于偏移不满足指令的要求，需要额外分配寄存器
			iloc.load_var(load_arg2_reg_no, arg2);
		} else {
			load_arg2_reg_no = arg2_reg_no;
		}
    }

    // 看结果变量是否是寄存器，若不是则需要分配一个新的寄存器来保存运算的结果		备注：本身就不可能存在这种情况，比较像防御式编程
    if (result_reg_no == -1) {
        // 分配一个寄存器r10，用于暂存结果
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
    } else {
        load_result_reg_no = result_reg_no;
    }

    // r8 + r9 -> r10
    iloc.inst(operator_name,
              PlatformArm32::regName[load_result_reg_no],
              PlatformArm32::regName[load_arg1_reg_no],
              PlatformArm32::regName[load_arg2_reg_no]);

    // 结果不是寄存器，则需要把rs_reg_name保存到结果变量中
    if (result_reg_no == -1) {

        // 这里使用预留的临时寄存器，因为立即数可能过大，必须借助寄存器才可操作。

        // r10 -> result
        iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(arg1);
    simpleRegisterAllocator.free(arg2);
    simpleRegisterAllocator.free(result);
}

/// @brief 无结果寄存器指令翻译成ARM32汇编
/// @param inst IR指令
/// @param operator_name 操作码
void InstSelectorArm32::translate_no_result(Instruction * inst, string operator_name)
{
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t load_arg1_reg_no, load_arg2_reg_no;

    // 看arg1是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg1_reg_no == -1) {

        // 分配一个寄存器r8
        load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);

        // arg1 -> r8，这里可能由于偏移不满足指令的要求，需要额外分配寄存器
        iloc.load_var(load_arg1_reg_no, arg1);
    } else {
        load_arg1_reg_no = arg1_reg_no;
    }

    // 看arg2是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg2_reg_no == -1) {

        // 分配一个寄存器r9
        load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);

        // arg2 -> r9
        iloc.load_var(load_arg2_reg_no, arg2);
    } else {
        load_arg2_reg_no = arg2_reg_no;
    }

    // r8 + r9
    iloc.inst_no_res("cmp",
              PlatformArm32::regName[load_arg1_reg_no],
              PlatformArm32::regName[load_arg2_reg_no]);

    // 如果当前这条指令的结果在后续将会被使用
    if (inst->isUsed()) {
        int32_t result_reg_no = inst->getRegId();
        int32_t load_result_reg_no;
        
        if (result_reg_no == -1) {
            load_result_reg_no = simpleRegisterAllocator.Allocate(inst);
		}else {
			load_result_reg_no = result_reg_no;
        }
        
        if (operator_name == "bgt") {
            iloc.inst("movgt", PlatformArm32::regName[load_result_reg_no], "#1");
            iloc.inst("movle", PlatformArm32::regName[load_result_reg_no], "#0");
            iloc.store_var(load_result_reg_no, inst, ARM32_TMP_REG_NO);
        } else if (operator_name == "blt") {
            iloc.inst("movlt", PlatformArm32::regName[load_result_reg_no], "#1");
            iloc.inst("movge", PlatformArm32::regName[load_result_reg_no], "#0");
            iloc.store_var(load_result_reg_no, inst, ARM32_TMP_REG_NO);
        } else if (operator_name == "bge") {
            iloc.inst("movge", PlatformArm32::regName[load_result_reg_no], "#1");
            iloc.inst("movlt", PlatformArm32::regName[load_result_reg_no], "#0");
            iloc.store_var(load_result_reg_no, inst, ARM32_TMP_REG_NO);
        } else if (operator_name == "ble") {
            iloc.inst("movle", PlatformArm32::regName[load_result_reg_no], "#1");
            iloc.inst("movgt", PlatformArm32::regName[load_result_reg_no], "#0");
            iloc.store_var(load_result_reg_no, inst, ARM32_TMP_REG_NO);
        } else if (operator_name == "beq") {
            iloc.inst("moveq", PlatformArm32::regName[load_result_reg_no], "#1");
            iloc.inst("movne", PlatformArm32::regName[load_result_reg_no], "#0");
            iloc.store_var(load_result_reg_no, inst, ARM32_TMP_REG_NO);
        } else if (operator_name == "bne") {
            iloc.inst("movne", PlatformArm32::regName[load_result_reg_no], "#1");
            iloc.inst("moveq", PlatformArm32::regName[load_result_reg_no], "#0");
            iloc.store_var(load_result_reg_no, inst, ARM32_TMP_REG_NO);
        }

        simpleRegisterAllocator.free(inst);
	}

    // 释放寄存器
    simpleRegisterAllocator.free(arg1);
    simpleRegisterAllocator.free(arg2);
}

/// @brief 整数加法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_add_int32(Instruction * inst)
{
    translate_two_operator(inst, "add");
}

/// @brief 整数减法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_sub_int32(Instruction * inst)
{
    translate_two_operator(inst, "sub");
}

/// @brief 整数求负指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_minus_int32(Instruction * inst)
{
    translate_one_operator(inst, "neg");
}

/// @brief 整数乘法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_mul_int32(Instruction * inst)
{
    translate_two_operator(inst, "mul");
}

/// @brief 整数除法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_div_int32(Instruction * inst)
{
    translate_two_operator(inst, "sdiv");
}

/// @brief 整数取余指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_mod_int32(Instruction * inst)
{
    // 这里简单使用函数translate_two_operator()不现实，因为不可能只使用一条指令完成取余
    // 故这里复用translate_two_operator()的结构进行多条二元运算指令的翻译
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);			//dividend
    Value * arg2 = inst->getOperand(1);			//divisor

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = inst->getRegId();
    int32_t load_dividend_reg_no, load_divisor_reg_no, load_result_reg_no, quotient_reg_no;

    // 分配一个寄存器用于保存计算得到的商的值
    quotient_reg_no = simpleRegisterAllocator.Allocate();

    // 看arg1是否是寄存器，若是则寄存器寻址，否则要load被除数到寄存器中
    if (arg1_reg_no == -1) {

        // 分配一个寄存器r8
        load_dividend_reg_no = simpleRegisterAllocator.Allocate(arg1);

        // arg1 -> r8，这里可能由于偏移不满足指令的要求，需要额外分配寄存器
        iloc.load_var(load_dividend_reg_no, arg1);
    } else {
        load_dividend_reg_no = arg1_reg_no;
    }

    // 看arg2是否是寄存器，若是则寄存器寻址，否则要load除数到寄存器中
    if (arg2_reg_no == -1) {

        // 分配一个寄存器r9
        load_divisor_reg_no = simpleRegisterAllocator.Allocate(arg2);

        // arg2 -> r9
        iloc.load_var(load_divisor_reg_no, arg2);
    } else {
        load_divisor_reg_no = arg2_reg_no;
    }

    // 看结果变量是否是寄存器，若不是则需要分配一个新的寄存器来保存运算的结果		备注：本身就不可能存在这种情况，比较像防御式编程
    if (result_reg_no == -1) {
        // 分配一个寄存器r10，用于暂存结果
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
    } else {
        load_result_reg_no = result_reg_no;
    }

    // 首先执行sdiv求商
    iloc.inst("sdiv",
              PlatformArm32::regName[quotient_reg_no],
              PlatformArm32::regName[load_dividend_reg_no],
              PlatformArm32::regName[load_divisor_reg_no]);

    // 接着计算商 * 除数
    iloc.inst("mul",
              PlatformArm32::regName[load_result_reg_no],
              PlatformArm32::regName[quotient_reg_no],
              PlatformArm32::regName[load_divisor_reg_no]);

    // 最后计算余数，得到真正的结果
    iloc.inst("sub",
              PlatformArm32::regName[load_result_reg_no],
              PlatformArm32::regName[load_dividend_reg_no],
              PlatformArm32::regName[load_result_reg_no]);

    // 结果不是寄存器，则需要把rs_reg_name保存到结果变量中
    if (result_reg_no == -1) {

        // 这里使用预留的临时寄存器，因为立即数可能过大，必须借助寄存器才可操作。

        // r10 -> result
        iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(arg1);
    simpleRegisterAllocator.free(arg2);
    simpleRegisterAllocator.free(result);
    simpleRegisterAllocator.free(quotient_reg_no);
}

/// @brief 关系运算>指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_gt_int32(Instruction * inst)
{
    translate_no_result(inst, "bgt");
    haveCmp = true;
    cmpType = "bgt";
}

/// @brief 关系运算<指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_lt_int32(Instruction * inst)
{
	translate_no_result(inst, "blt");
    haveCmp = true;
    cmpType = "blt";
}

/// @brief 关系运算>=指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_ge_int32(Instruction * inst)
{
	translate_no_result(inst, "bge");
    haveCmp = true;
    cmpType = "bge";
}

/// @brief 关系运算<=指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_le_int32(Instruction * inst)
{
	translate_no_result(inst, "ble");
    haveCmp = true;
    cmpType = "ble";
}

/// @brief 关系运算==指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_eq_int32(Instruction * inst)
{
	translate_no_result(inst, "beq");
    haveCmp = true;
    cmpType = "beq";

}

/// @brief 关系运算!=指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_ne_int32(Instruction * inst)
{
	translate_no_result(inst, "bne");
    haveCmp = true;
    cmpType = "bne";
}

/// @brief 分支跳转指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_branch(Instruction * inst)
{
    Instanceof(branchInst, BranchInstruction *, inst);
    auto trueLabel = branchInst->getTarget1()->getName();
    auto falseLbel = branchInst->getTarget2()->getName();

    if (haveCmp == true) {
        iloc.branch(cmpType, trueLabel);
        iloc.jump(falseLbel);
        haveCmp = false;
        cmpType.clear();
	}
}

/// @brief 加载指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_load(Instruction * inst)
{
    Value * result = inst;
    Value * addr = inst->getOperand(0);

    int32_t result_reg_no = result->getRegId();
    int32_t addr_reg_no = addr->getRegId();
    int32_t load_result_reg_no, load_addr_reg_no;

    if (result_reg_no == -1) {
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
        iloc.load_var(load_result_reg_no, result);
    } else {
        load_result_reg_no = result_reg_no;
    }

    if (addr_reg_no == -1) {
        load_addr_reg_no = simpleRegisterAllocator.Allocate(addr);
        iloc.load_var(load_addr_reg_no, addr);
    } else {
        load_addr_reg_no = addr_reg_no;
	}

    iloc.inst("ldr",
              PlatformArm32::regName[load_result_reg_no],
              "[" + PlatformArm32::regName[load_addr_reg_no] + "]");

    if (result_reg_no != load_result_reg_no) {
        iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
	}

    simpleRegisterAllocator.free(addr);
    simpleRegisterAllocator.free(result);
}

/// @brief 存储指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_store(Instruction * inst)
{
    Value * addr = inst->getOperand(0);
    Value * result = inst->getOperand(1);

	int32_t addr_reg_no = addr->getRegId();
	int32_t result_reg_no = result->getRegId();
    int32_t load_result_reg_no, load_addr_reg_no;

    if (addr_reg_no == -1) {
        load_addr_reg_no = simpleRegisterAllocator.Allocate(addr);
        iloc.load_var(load_addr_reg_no, addr);
    } else {
        load_addr_reg_no = addr_reg_no;
	}

    if (result_reg_no == -1) {
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
        iloc.load_var(load_result_reg_no, result);
    } else {
        load_result_reg_no = result_reg_no;
    }

    iloc.inst("str",
              PlatformArm32::regName[load_result_reg_no],
              "[" + PlatformArm32::regName[load_addr_reg_no] + "]");

    simpleRegisterAllocator.free(addr);
    simpleRegisterAllocator.free(result);
}

/// @brief 函数调用指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_call(Instruction * inst)
{
    FuncCallInstruction * callInst = dynamic_cast<FuncCallInstruction *>(inst);

    int32_t operandNum = callInst->getOperandsNum();

    if (operandNum != realArgCount) {

        // 两者不一致 也可能没有ARG指令，正常
        if (realArgCount != 0) {

            minic_log(LOG_ERROR, "ARG指令的个数与调用函数个数不一致");
        }
    }

    if (operandNum) {

        // 强制占用这几个寄存器参数传递的寄存器
        simpleRegisterAllocator.Allocate(0);
        simpleRegisterAllocator.Allocate(1);
        simpleRegisterAllocator.Allocate(2);
        simpleRegisterAllocator.Allocate(3);

        // 前四个的后面参数采用栈传递
        int esp = 0;
        for (int32_t k = 4; k < operandNum; k++) {

            auto arg = callInst->getOperand(k);

            // 新建一个内存变量，用于栈传值到形参变量中
            MemVariable * newVal;
            if (arg->getType()->isPointerType()) {
                newVal = func->newMemVariable(arg->getType());
            } else {
                newVal = func->newMemVariable((Type *) PointerType::get(arg->getType()));
			}
            newVal->setMemoryAddr(ARM32_SP_REG_NO, esp);
            esp += 4;

            Instruction * assignInst = new MoveInstruction(func, newVal, arg);

            // 翻译赋值指令
            translate_assign(assignInst);

            delete assignInst;
        }

        for (int32_t k = 0; k < operandNum && k < 4; k++) {

            auto arg = callInst->getOperand(k);

            // 检查实参的类型是否是临时变量。
            // 如果是临时变量，该变量可更改为寄存器变量即可，或者设置寄存器号
            // 如果不是，则必须开辟一个寄存器变量，然后赋值即可

            Instruction * assignInst = new MoveInstruction(func, PlatformArm32::intRegVal[k], arg);

            // 翻译赋值指令
            translate_assign(assignInst);

            delete assignInst;
        }
    }

    iloc.call_fun(callInst->getName());

    if (operandNum) {
        simpleRegisterAllocator.free(0);
        simpleRegisterAllocator.free(1);
        simpleRegisterAllocator.free(2);
        simpleRegisterAllocator.free(3);
    }

    // 赋值指令
    if (callInst->hasResultValue()) {

        // 新建一个赋值操作
        Instruction * assignInst = new MoveInstruction(func, callInst, PlatformArm32::intRegVal[0]);

        // 翻译赋值指令
        translate_assign(assignInst);

        delete assignInst;
    }

    // 函数调用后清零，使得下次可正常统计
    realArgCount = 0;
}

///
/// @brief 实参指令翻译成ARM32汇编
/// @param inst
///
void InstSelectorArm32::translate_arg(Instruction * inst)
{
    // 翻译之前必须确保源操作数要么是寄存器，要么是内存，否则出错。
    Value * src = inst->getOperand(0);

    // 当前统计的ARG指令个数
    int32_t regId = src->getRegId();

    if (realArgCount < 4) {
        // 前四个参数
        if (regId != -1) {
            if (regId != realArgCount) {
                // 肯定寄存器分配有误
                minic_log(LOG_ERROR, "第%d个ARG指令对象寄存器分配有误: %d", argCount + 1, regId);
            }
        } else {
            minic_log(LOG_ERROR, "第%d个ARG指令对象不是寄存器", argCount + 1);
        }
    } else {
        // 必须是内存分配，若不是则出错
        int32_t baseRegId;
        bool result = src->getMemoryAddr(&baseRegId);
        if ((!result) || (baseRegId != ARM32_SP_REG_NO)) {

            minic_log(LOG_ERROR, "第%d个ARG指令对象不是SP寄存器寻址", argCount + 1);
        }
    }

    realArgCount++;
}
