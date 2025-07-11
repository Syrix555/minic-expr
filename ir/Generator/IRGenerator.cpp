﻿///
/// @file IRGenerator.cpp
/// @brief AST遍历产生线性IR的源文件
/// @author zenglj (zenglj@live.com)
/// @version 1.1
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// <tr><td>2024-11-23 <td>1.1     <td>zenglj  <td>表达式版增强
/// </table>
///
#include <any>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "AST.h"
#include "ArrayType.h"
#include "BranchInstruction.h"
#include "Common.h"
#include "ConstInt.h"
#include "FormalParam.h"
#include "Function.h"
#include "GlobalVariable.h"
#include "IRCode.h"
#include "IRGenerator.h"
#include "Instruction.h"
#include "IntegerType.h"
#include "LoadInstruction.h"
#include "LocalVariable.h"
#include "Module.h"
#include "EntryInstruction.h"
#include "LabelInstruction.h"
#include "ExitInstruction.h"
#include "FuncCallInstruction.h"
#include "PointerType.h"
#include "StoreInstruction.h"
#include "Type.h"
#include "UnaryInstruction.h"
#include "BinaryInstruction.h"
#include "MoveInstruction.h"
#include "GotoInstruction.h"
#include "Value.h"

#define Instanceof(res, type, var) auto res = dynamic_cast<type>(var)

/// @brief 构造函数
/// @param _root AST的根
/// @param _module 符号表
IRGenerator::IRGenerator(ast_node * _root, Module * _module) : root(_root), module(_module)
{
    /* 叶子节点 */
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_LITERAL_UINT] = &IRGenerator::ir_leaf_node_uint;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_VAR_ID] = &IRGenerator::ir_leaf_node_var_id;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_TYPE] = &IRGenerator::ir_leaf_node_type;

    /* 表达式运算， 加减乘除求余 */
    ast2ir_handlers[ast_operator_type::AST_OP_SUB] = &IRGenerator::ir_sub_or_minus;			//将ir_sub替换成分派方法，选择执行求负还是减法
    ast2ir_handlers[ast_operator_type::AST_OP_ADD] = &IRGenerator::ir_add;
    ast2ir_handlers[ast_operator_type::AST_OP_MUL] = &IRGenerator::ir_mul;
    ast2ir_handlers[ast_operator_type::AST_OP_DIV] = &IRGenerator::ir_div;
    ast2ir_handlers[ast_operator_type::AST_OP_MOD] = &IRGenerator::ir_mod;

    /* 关系表达式运算 */
    ast2ir_handlers[ast_operator_type::AST_OP_LT] = &IRGenerator::ir_cmp_lt;
    ast2ir_handlers[ast_operator_type::AST_OP_GT] = &IRGenerator::ir_cmp_gt;
    ast2ir_handlers[ast_operator_type::AST_OP_LE] = &IRGenerator::ir_cmp_le;
    ast2ir_handlers[ast_operator_type::AST_OP_GE] = &IRGenerator::ir_cmp_ge;
    ast2ir_handlers[ast_operator_type::AST_OP_EQ] = &IRGenerator::ir_cmp_eq;
    ast2ir_handlers[ast_operator_type::AST_OP_NE] = &IRGenerator::ir_cmp_ne;

    /* 逻辑表达式运算 */
    ast2ir_handlers[ast_operator_type::AST_OP_AND] = &IRGenerator::ir_and;
    ast2ir_handlers[ast_operator_type::AST_OP_OR] = &IRGenerator::ir_or;
    ast2ir_handlers[ast_operator_type::AST_OP_NOT] = &IRGenerator::ir_not;

    /* 语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_ASSIGN] = &IRGenerator::ir_assign;
    ast2ir_handlers[ast_operator_type::AST_OP_RETURN] = &IRGenerator::ir_return;
    ast2ir_handlers[ast_operator_type::AST_OP_IF] = &IRGenerator::ir_if;
    ast2ir_handlers[ast_operator_type::AST_OP_WHILE] = &IRGenerator::ir_while;
    ast2ir_handlers[ast_operator_type::AST_OP_BREAK] = &IRGenerator::ir_break;
    ast2ir_handlers[ast_operator_type::AST_OP_CONTINUE] = &IRGenerator::ir_continue;

    /* 数组相关 */
    ast2ir_handlers[ast_operator_type::AST_OP_ARRAY_INIT] = &IRGenerator::ir_array_init;
    ast2ir_handlers[ast_operator_type::AST_OP_ARRAY_INDEX] = &IRGenerator::ir_array_index;

    /* 函数调用 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_CALL] = &IRGenerator::ir_function_call;

    /* 函数定义 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_DEF] = &IRGenerator::ir_function_define;
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS] = &IRGenerator::ir_function_formal_params;

    /* 变量定义语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_DECL_STMT] = &IRGenerator::ir_declare_statment;
    ast2ir_handlers[ast_operator_type::AST_OP_VAR_DECL] = &IRGenerator::ir_variable_declare;

    /* 语句块 */
    ast2ir_handlers[ast_operator_type::AST_OP_BLOCK] = &IRGenerator::ir_block;

    /* 编译单元 */
    ast2ir_handlers[ast_operator_type::AST_OP_COMPILE_UNIT] = &IRGenerator::ir_compile_unit;
}

/// @brief 遍历抽象语法树产生线性IR，保存到IRCode中
/// @param root 抽象语法树
/// @param IRCode 线性IR
/// @return true: 成功 false: 失败
bool IRGenerator::run()
{
    ast_node * node;

    // 从根节点进行遍历
    node = ir_visit_ast_node(root);

    return node != nullptr;
}

/// @brief 根据AST的节点运算符查找对应的翻译函数并执行翻译动作
/// @param node AST节点
/// @return 成功返回node节点，否则返回nullptr
ast_node * IRGenerator::ir_visit_ast_node(ast_node * node)
{
    // 空节点
    if (nullptr == node) {
        return nullptr;
    }

    bool result;

    std::unordered_map<ast_operator_type, ast2ir_handler_t>::const_iterator pIter;
    pIter = ast2ir_handlers.find(node->node_type);
    if (pIter == ast2ir_handlers.end()) {
        // 没有找到，则说明当前不支持
        result = (this->ir_default)(node);
    } else {
        result = (this->*(pIter->second))(node);
    }

    if (!result) {
        // 语义解析错误，则出错返回
        node = nullptr;
    }

    return node;
}

/// @brief 未知节点类型的节点处理
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_default(ast_node * node)
{
    // 未知的节点
    printf("Unknown node(%d)\n", (int) node->node_type);	// Unkown -> Unknown
    return true;
}

/// @brief 编译单元AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_compile_unit(ast_node * node)
{
    module->setCurrentFunction(nullptr);

    for (auto son: node->sons) {

        // 遍历编译单元，要么是函数定义，要么是语句
        ast_node * son_node = ir_visit_ast_node(son);
        if (!son_node) {
            // TODO 自行追加语义错误处理
            minic_log(LOG_ERROR, "函数或语句不存在");
            return false;
        }
    }

    return true;
}

/// @brief 函数定义AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_define(ast_node * node)
{
    bool result;

    // 创建一个函数，用于当前函数处理
    if (module->getCurrentFunction()) {
        // 函数中嵌套定义函数，这是不允许的，错误退出
        // TODO 自行追加语义错误处理
        minic_log(LOG_ERROR, "不允许嵌套函数");
        return false;
    }

    // 函数定义的AST包含四个孩子
    // 第一个孩子：函数返回类型
    // 第二个孩子：函数名字
    // 第三个孩子：形参列表
    // 第四个孩子：函数体即block
    ast_node * type_node = node->sons[0];
    ast_node * name_node = node->sons[1];
    ast_node * param_node = node->sons[2];
    ast_node * block_node = node->sons[3];

    // 创建一个新的函数定义
    Function * newFunc = module->newFunction(name_node->name, type_node->type);
    if (!newFunc) {
        // 新定义的函数已经存在，则失败返回。
        // TODO 自行追加语义错误处理
        minic_log(LOG_ERROR, "函数%s已存在", name_node->name.c_str());
        return false;
    }

    // 当前函数设置有效，变更为当前的函数
    module->setCurrentFunction(newFunc);

    // 进入函数的作用域
    module->enterScope();

    // 获取函数的IR代码列表，用于后面追加指令用，注意这里用的是引用传值
    InterCode & irCode = newFunc->getInterCode();

    // 这里也可增加一个函数入口Label指令，便于后续基本块划分
    irCode.addInst(new LabelInstruction(newFunc));

    // 创建并加入Entry入口指令
    irCode.addInst(new EntryInstruction(newFunc));

    // 创建出口指令并不加入出口指令，等函数内的指令处理完毕后加入出口指令
    LabelInstruction * exitLabelInst = new LabelInstruction(newFunc);

    // 函数出口指令保存到函数信息中，因为在语义分析函数体时return语句需要跳转到函数尾部，需要这个label指令
    newFunc->setExitLabel(exitLabelInst);

    // 遍历形参，没有IR指令，不需要追加
    result = ir_function_formal_params(param_node);
    if (!result) {
        // 形参解析失败
        // TODO 自行追加语义错误处理
        minic_log(LOG_ERROR, "函数 %s 形参列表解析失败",name_node->name.c_str());
        return false;
    }
    node->blockInsts.addInst(param_node->blockInsts);

    // 新建一个Value，用于保存函数的返回值，如果没有返回值可不用申请
    LocalVariable * retValue = nullptr;
    if (!type_node->type->isVoidType()) {

        // 保存函数返回值变量到函数信息中，在return语句翻译时需要设置值到这个变量中
        retValue = static_cast<LocalVariable *>(module->newVarValue(type_node->type));
    }
    newFunc->setReturnValue(retValue);

    // TODO 这里最好设置返回值变量的初值为0，以便在没有返回值时能够返回0
    if (newFunc->getName() == "main") {

	}

    // 函数内已经进入作用域，内部不再需要做变量的作用域管理
    block_node->needScope = false;

    // 遍历block
    result = ir_block(block_node);
    if (!result) {
        // block解析失败
        // TODO 自行追加语义错误处理
        minic_log(LOG_ERROR, "函数 %s 语句块解析失败",name_node->name.c_str());
        return false;
    }

    // IR指令追加到当前的节点中
    node->blockInsts.addInst(block_node->blockInsts);

    // 此时，所有指令都加入到当前函数中，也就是node->blockInsts

    // node节点的指令移动到函数的IR指令列表中
    irCode.addInst(node->blockInsts);

    // 添加函数出口Label指令，主要用于return语句跳转到这里进行函数的退出
    irCode.addInst(exitLabelInst);

    // 函数出口指令
    irCode.addInst(new ExitInstruction(newFunc, retValue));

    // 恢复成外部函数
    module->setCurrentFunction(nullptr);

    // 退出函数的作用域
    module->leaveScope();

    return true;
}

/// @brief 形式参数AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_formal_params(ast_node * node)
{
    // TODO 目前形参还不支持，直接返回true

    // 每个形参变量都创建对应的临时变量，用于表达实参转递的值
    // 而真实的形参则创建函数内的局部变量。
    // 然后产生赋值指令，用于把表达实参值的临时变量拷贝到形参局部变量上。
    // 请注意这些指令要放在Entry指令后面，因此处理的先后上要注意。

    auto currentFunc = module->getCurrentFunction();
    for (auto son: node->sons) {
		// 获取形参类型
        Type * param_type = son->type;
        // 创建函数形参
        FormalParam * param = new FormalParam(param_type, "");
        // 保存到函数的形参列表中
        currentFunc->getParams().push_back(param);

        // 创建临时变量保存形参传入值
        son->val = module->newVarValue(param_type, son->sons[1]->name);

        // 产生赋值指令拷贝值
        node->blockInsts.addInst(new MoveInstruction(currentFunc, son->val, param));
	}
    return true;
}

/// @brief 函数调用AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_call(ast_node * node)
{
    std::vector<Value *> realParams;

    // 获取当前正在处理的函数
    Function * currentFunc = module->getCurrentFunction();

    // 函数调用的节点包含两个节点：
    // 第一个节点：函数名节点
    // 第二个节点：实参列表节点

    std::string funcName = node->sons[0]->name;
    int64_t lineno = node->sons[0]->line_no;

    ast_node * paramsNode = node->sons[1];

    // 根据函数名查找函数，看是否存在。若不存在则出错
    // 这里约定函数必须先定义后使用
    auto calledFunction = module->findFunction(funcName);
    if (nullptr == calledFunction) {
        minic_log(LOG_ERROR, "函数(%s)未定义或声明", funcName.c_str());
        return false;
    }

    // 当前函数存在函数调用
    currentFunc->setExistFuncCall(true);

    // 如果没有孩子，也认为是没有参数
    if (!paramsNode->sons.empty()) {

        int32_t argsCount = (int32_t) paramsNode->sons.size();

        // 当前函数中调用函数实参个数最大值统计，实际上是统计实参传参需在栈中分配的大小
        // 因为目前的语言支持的int和float都是四字节的，只统计个数即可
        if (argsCount > currentFunc->getMaxFuncCallArgCnt()) {
            currentFunc->setMaxFuncCallArgCnt(argsCount);
        }

		// TODO 这里请追加函数调用的语义错误检查，这里只进行了函数参数的个数检查等，其它请自行追加。
		if (paramsNode->sons.size() != calledFunction->getParams().size()) {
			// 函数参数的个数不一致，语义错误
			minic_log(LOG_ERROR, "第%lld行的被调用函数(%s)未定义或声明", (long long) lineno, funcName.c_str());
			return false;
		}

        // 遍历参数列表，孩子是表达式
        // 这里自左往右计算表达式
        auto it_paramsNode = calledFunction->getParams().begin();
        for (auto son: paramsNode->sons) {

            // 遍历Block的每个语句，进行显示或者运算
            ast_node * temp = ir_visit_ast_node(son);
            if (!temp) {
                return false;
            }

            node->blockInsts.addInst(temp->blockInsts);

            auto formal_param = *it_paramsNode;
            if (formal_param->getType()->isIntegerType() && temp->val->getType()->isPointerType()) {
                LoadInstruction * loadInst = new LoadInstruction(module->getCurrentFunction(),
                                                         		 temp->val,
                                                         		 IntegerType::getTypeInt());
        		node->blockInsts.addInst(loadInst);
        		temp->val = loadInst;
            }

            realParams.push_back(temp->val);

            ++it_paramsNode;
        }
    }

    // 返回调用有返回值，则需要分配临时变量，用于保存函数调用的返回值
    Type * type = calledFunction->getReturnType();

    FuncCallInstruction * funcCallInst = new FuncCallInstruction(currentFunc, calledFunction, realParams, type);

    // 创建函数调用指令
    node->blockInsts.addInst(funcCallInst);

    // 函数调用结果Value保存到node中，可能为空，上层节点可利用这个值
    node->val = funcCallInst;

    return true;
}

/// @brief 语句块（含函数体）AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_block(ast_node * node)
{
    // 进入作用域
    if (node->needScope) {
        module->enterScope();
    }

    if (node->parent->node_type != ast_operator_type::AST_OP_WHILE) {
        node->inherit_label(node->parent);
	}

    std::vector<ast_node *>::iterator pIter;
    for (pIter = node->sons.begin(); pIter != node->sons.end(); ++pIter) {

        // 遍历Block的每个语句，进行显示或者运算
        ast_node * temp = ir_visit_ast_node(*pIter);
        if (!temp) {
            return false;
        }

        node->blockInsts.addInst(temp->blockInsts);
    }

    // 离开作用域
    if (node->needScope) {
        module->leaveScope();
    }

    return true;
}

/// @brief 整数加法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_add(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 加法节点，左结合，先计算左节点，后计算右节点

    // 加法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ADD_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(addInst);

    node->val = addInst;

	if (left->val->getType()->isInt1Byte()) {
        LabelInstruction * trueLabel = new LabelInstruction(module->getCurrentFunction());
        LabelInstruction * falseLabel = new LabelInstruction(module->getCurrentFunction());
        LabelInstruction * exitLabel = new LabelInstruction(module->getCurrentFunction());

        node->blockInsts.addInst(new BranchInstruction(module->getCurrentFunction(),
                                                       left->val,
                                                       trueLabel,
                                                       falseLabel));
        node->blockInsts.addInst(trueLabel);

        ConstInt * one = module->newConstInt(1);
        ConstInt * zero = module->newConstInt(0);
        Value * tmpVal = module->newVarValue(IntegerType::getTypeInt());

        node->blockInsts.addInst(new MoveInstruction(module->getCurrentFunction(),
                                                     tmpVal,
                                                     one));
        node->blockInsts.addInst(new GotoInstruction(module->getCurrentFunction(), exitLabel));

        node->blockInsts.addInst(falseLabel);
        node->blockInsts.addInst(new MoveInstruction(module->getCurrentFunction(),
                                                     tmpVal,
                                                     zero));
        node->blockInsts.addInst(exitLabel);
        node->val = tmpVal;
	}

    return true;
}

/// @brief 负号分派方法，用于判断对应的是一元求负还是二元减法
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_sub_or_minus(ast_node * node)
{
    if (node->sons.size() == 2) {
        return ir_sub(node);
    } else if (node->sons.size() == 1) {
        return ir_minus(node);
    } else {
        return false;
	}
}

/// @brief 整数减法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_sub(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 减法节点，左结合，先计算左节点，后计算右节点

    // 减法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * subInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(subInst);

    node->val = subInst;

    return true;
}

/// @brief 求负运算AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_minus(ast_node * node)
{
    ast_node * src_node = node->sons[0];

    ast_node * num = ir_visit_ast_node(src_node);
    if (!num) {
		// 该变量无定值
        return false;
    }

    UnaryInstruction * minusInst = new UnaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MINUS_I,
                                                        num->val,
                                                        IntegerType::getTypeInt());

    node->blockInsts.addInst(num->blockInsts);
    node->blockInsts.addInst(minusInst);

    node->val = minusInst;

    return true;
    
}

/// @brief 整数乘法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mul(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 乘法节点，左结合，先计算左节点，后计算右节点

    // 乘法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 乘法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * mulInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MUL_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(mulInst);

    node->val = mulInst;

    return true;
}

/// @brief 整数除法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_div(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 除法节点，左结合，先计算左节点，后计算右节点

    // 除法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 除法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * divInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_DIV_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(divInst);

    node->val = divInst;

    return true;
}

/// @brief 整数求余AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mod(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 求余节点，左结合，先计算左节点，后计算右节点

    // 求余的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 求余的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * modInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MOD_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(modInst);

    node->val = modInst;

    return true;
}

/// @brief 关系运算<AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_cmp_lt(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 关系运算同样是左结合的，且没必要继续传标签

    // 左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    BinaryInstruction * ltInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_LT_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeBool());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(ltInst);

    node->val = ltInst;

    // 如果if的条件块就是本指令对应的node，那么添加if语句的条件跳转语句
    if (node->parent->node_type == ast_operator_type::AST_OP_IF ||
        node->parent->node_type == ast_operator_type::AST_OP_WHILE) {
        BranchInstruction * ifBranch = new BranchInstruction(module->getCurrentFunction(),
                                                             ltInst,
                                                             node->trueLabel,
                                                             node->falseLabel);
		node->blockInsts.addInst(ifBranch);
	}

    return true;
}

/// @brief 关系运算>AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_cmp_gt(ast_node * node)
{
	ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 关系运算同样是左结合的，且没必要继续传标签

    // 左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    BinaryInstruction * gtInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_GT_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeBool());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(gtInst);

    node->val = gtInst;

    // 如果if的条件块就是本指令对应的node，那么添加if语句的条件跳转语句
    if (node->parent->node_type == ast_operator_type::AST_OP_IF ||
        node->parent->node_type == ast_operator_type::AST_OP_WHILE) {
        BranchInstruction * ifBranch = new BranchInstruction(module->getCurrentFunction(),
                                                             node->val,
                                                             node->trueLabel,
                                                             node->falseLabel);
		node->blockInsts.addInst(ifBranch);
	}

    return true;
}

/// @brief 关系运算<=AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_cmp_le(ast_node * node)
{
	ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 关系运算同样是左结合的，且没必要继续传标签

    // 左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    BinaryInstruction * leInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_LE_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeBool());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(leInst);

    node->val = leInst;

    // 如果if的条件块就是本指令对应的node，那么添加if语句的条件跳转语句
    if (node->parent->node_type == ast_operator_type::AST_OP_IF ||
        node->parent->node_type == ast_operator_type::AST_OP_WHILE) {
        BranchInstruction * ifBranch = new BranchInstruction(module->getCurrentFunction(),
                                                             node->val,
                                                             node->trueLabel,
                                                             node->falseLabel);
		node->blockInsts.addInst(ifBranch);
	}

    return true;
}

/// @brief 关系运算>=AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_cmp_ge(ast_node * node)
{
	ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 关系运算同样是左结合的，且没必要继续传标签

    // 左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    BinaryInstruction * geInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_GE_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeBool());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(geInst);

    node->val = geInst;

    // 如果if的条件块就是本指令对应的node，那么添加if语句的条件跳转语句
    if (node->parent->node_type == ast_operator_type::AST_OP_IF ||
        node->parent->node_type == ast_operator_type::AST_OP_WHILE) {
        BranchInstruction * ifBranch = new BranchInstruction(module->getCurrentFunction(),
                                                             node->val,
                                                             node->trueLabel,
                                                             node->falseLabel);
		node->blockInsts.addInst(ifBranch);
	}

    return true;
}

/// @brief 相等性运算==AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_cmp_eq(ast_node * node)
{
	ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 相等性运算同样是左结合的，且没必要继续传标签

    // 左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    BinaryInstruction * eqInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_EQ_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeBool());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(eqInst);

    node->val = eqInst;

    // 如果if的条件块就是本指令对应的node，那么添加if语句的条件跳转语句
    if (node->parent->node_type == ast_operator_type::AST_OP_IF ||
        node->parent->node_type == ast_operator_type::AST_OP_WHILE) {
        BranchInstruction * ifBranch = new BranchInstruction(module->getCurrentFunction(),
                                                             node->val,
                                                             node->trueLabel,
                                                             node->falseLabel);
		node->blockInsts.addInst(ifBranch);
	}

    return true;
}

/// @brief 相等性运算!=AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_cmp_ne(ast_node * node)
{
	ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 相等性运算同样是左结合的，且没必要继续传标签

    // 左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    BinaryInstruction * neInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_NE_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeBool());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(neInst);

    node->val = neInst;

    // 如果if的条件块就是本指令对应的node，那么添加if语句的条件跳转语句
    if (node->parent->node_type == ast_operator_type::AST_OP_IF ||
        node->parent->node_type == ast_operator_type::AST_OP_WHILE) {
        BranchInstruction * ifBranch = new BranchInstruction(module->getCurrentFunction(),
                                                             node->val,
                                                             node->trueLabel,
                                                             node->falseLabel);
		node->blockInsts.addInst(ifBranch);
	}

    return true;
}

/// @brief 逻辑运算&&AST节点翻译成线性中间IR
/// @param node AST节点
/// @return true 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_and(ast_node * node)
{
	ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 获取当前作用域使用的函数
    Function * currentFunc = module->getCurrentFunction();
    
    LabelInstruction * truePass = new LabelInstruction(currentFunc);
    // 继承父节点的所有标签
    src1_node->inherit_label(node);
    src1_node->trueLabel = truePass;
    src2_node->inherit_label(node);

    // 在这里创建真传递出口，真假出口使用父节点条件分支结构的标签
    LabelInstruction * trueLabel = node->trueLabel;
    LabelInstruction * falseLabel = node->falseLabel;

    // 开始遍历两个子节点
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量无定值
        return false;
    }

    node->blockInsts.addInst(left->blockInsts);

    ConstInt * zero = module->newConstInt(0);

    // 如果and运算的源操作数只有一个变量，需要自行创建比较指令
    if (left->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID ||
        left->node_type == ast_operator_type::AST_OP_FUNC_CALL ||
        left->node_type == ast_operator_type::AST_OP_ADD ||
		left->node_type == ast_operator_type::AST_OP_SUB ||
		left->node_type == ast_operator_type::AST_OP_MUL ||
		left->node_type == ast_operator_type::AST_OP_DIV ||
		left->node_type == ast_operator_type::AST_OP_MOD ||
		left->node_type == ast_operator_type::AST_OP_ARRAY_INDEX) {
        BinaryInstruction * src1_cmp = new BinaryInstruction(currentFunc,
                                                             IRInstOperator::IRINST_OP_NE_I,
                                                             left->val,
                                                             zero,
                                                             IntegerType::getTypeBool());
        node->blockInsts.addInst(src1_cmp);
        left->val = src1_cmp;
    }

    // 基于结果进行真传递出口与假出口的传递
	if (left->node_type != ast_operator_type::AST_OP_AND
        && left->node_type != ast_operator_type::AST_OP_OR){
			BranchInstruction * src1_jmp = new BranchInstruction(currentFunc,
															   left->val,
															  truePass,
															  falseLabel);
			node->blockInsts.addInst(src1_jmp);
		}

    // 将真传递出口指令加入指令块中
    node->blockInsts.addInst(truePass);

    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量无定值
        return false;
    }

    node->blockInsts.addInst(right->blockInsts);

    // 处理右节点时假标签需要单独处理，如果后续子节点不再有and/or，这里假结点将直接跳出if语句，执行结束
    //! 父节点有or时，还可以继续判断，不应该跳出
    // if (src1_node->node_type != ast_operator_type::AST_OP_AND &&
    //     src1_node->node_type != ast_operator_type::AST_OP_OR &&
	// 	src2_node->node_type != ast_operator_type::AST_OP_AND &&
	// 	src2_node->node_type != ast_operator_type::AST_OP_OR &&
	// 	node->parent->node_type != ast_operator_type::AST_OP_IF &&
	// 	node->parent->node_type != ast_operator_type::AST_OP_OR &&
	// 	(node->parent->node_type == ast_operator_type::AST_OP_OR || node == node->parent->sons[1])) {
	// 		falseLabel = node->endLabel;
    // }

    // 如果and运算的源操作数只有一个变量，需要自行创建比较指令
    //! 这里需要重新处理条件，规定调用的函数必须有返回值
    if (right->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID ||
        right->node_type == ast_operator_type::AST_OP_FUNC_CALL ||
        right->node_type == ast_operator_type::AST_OP_ADD ||
		right->node_type == ast_operator_type::AST_OP_SUB ||
		right->node_type == ast_operator_type::AST_OP_MUL ||
		right->node_type == ast_operator_type::AST_OP_DIV ||
		right->node_type == ast_operator_type::AST_OP_MOD ||
		right->node_type == ast_operator_type::AST_OP_ARRAY_INDEX) {
        BinaryInstruction * src2_cmp = new BinaryInstruction(currentFunc,
                                                             IRInstOperator::IRINST_OP_NE_I,
                                                             right->val,
                                                             zero,
                                                             IntegerType::getTypeBool());
        node->blockInsts.addInst(src2_cmp);
        right->val = src2_cmp;
	}

    // 基于结果进行真出口与假出口的传递
	if (right->node_type != ast_operator_type::AST_OP_AND
        && right->node_type != ast_operator_type::AST_OP_OR){
			BranchInstruction * src2_jmp = new BranchInstruction(currentFunc,
															   right->val,
															  trueLabel,
															  falseLabel);
			node->blockInsts.addInst(src2_jmp);
		}

    return true;
}


/// @brief 逻辑运算||AST节点翻译成线性中间IR
/// @param node AST节点
/// @return true 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_or(ast_node * node)
{
	ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 获取当前作用域使用的函数
    Function * currentFunc = module->getCurrentFunction();

	LabelInstruction * falsePass = new LabelInstruction(currentFunc);
    // 继承父节点的所有标签
    src1_node->inherit_label(node);
    src1_node->falseLabel = falsePass;
    src2_node->inherit_label(node);

    // 在这里创建假传递出口，真假出口使用父节点条件分支结构的标签
	LabelInstruction * trueLabel = node->trueLabel;
    LabelInstruction * falseLabel = node->falseLabel;

    // 重新设置本节点保存的标签，使后面的子节点遍历能够顺利使用
    node->set_label(trueLabel, falsePass, node->endLabel);

    // 开始遍历两个子节点
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量无定值
        return false;
    }

    node->blockInsts.addInst(left->blockInsts);

    ConstInt * zero = module->newConstInt(0);

    // 如果and运算的源操作数只有一个变量，需要自行创建比较指令
    // TODO 同样不完善，需要修改
    if (left->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID ||
        left->node_type == ast_operator_type::AST_OP_FUNC_CALL ||
        left->node_type == ast_operator_type::AST_OP_ADD ||
		left->node_type == ast_operator_type::AST_OP_SUB ||
		left->node_type == ast_operator_type::AST_OP_MUL ||
		left->node_type == ast_operator_type::AST_OP_DIV ||
		left->node_type == ast_operator_type::AST_OP_MOD ||
		left->node_type == ast_operator_type::AST_OP_ARRAY_INDEX) {
        BinaryInstruction * src1_cmp = new BinaryInstruction(currentFunc,
                                                             IRInstOperator::IRINST_OP_NE_I,
                                                             left->val,
                                                             zero,
                                                             IntegerType::getTypeBool());
        node->blockInsts.addInst(src1_cmp);
        left->val = src1_cmp;
    }

    // 基于结果进行真出口与假传递出口的传递
	if (left->node_type != ast_operator_type::AST_OP_AND
        && left->node_type != ast_operator_type::AST_OP_OR){
			BranchInstruction * src1_jmp = new BranchInstruction(currentFunc,
															   left->val,
															  trueLabel,
															  falsePass);
			node->blockInsts.addInst(src1_jmp);
		}

    // 将假传递出口指令加入指令块中
    node->blockInsts.addInst(falsePass);

    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量无定值
        return false;
    }

    node->blockInsts.addInst(right->blockInsts);

    // 处理右节点时假标签需要单独处理，如果后续子节点不再有and/or，这里假结点将直接跳出if语句，执行结束
    //! 父节点有or时，还可以继续判断，不应该跳出，但如果是
    // if (src1_node->node_type != ast_operator_type::AST_OP_AND &&
    //     src1_node->node_type != ast_operator_type::AST_OP_OR &&
	// 	src2_node->node_type != ast_operator_type::AST_OP_AND &&
	// 	src2_node->node_type != ast_operator_type::AST_OP_OR &&
	// 	node->parent->node_type != ast_operator_type::AST_OP_IF &&
	// 	node->parent->node_type != ast_operator_type::AST_OP_OR &&
	// 	(node->parent->node_type == ast_operator_type::AST_OP_OR || node == node->parent->sons[1])){
	// 		falseLabel = node->endLabel;
    // }

    // 如果and运算的源操作数只有一个变量，需要自行创建比较指令
    // TODO 同样不完善，需要修改
    if (right->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID ||
        right->node_type == ast_operator_type::AST_OP_FUNC_CALL ||
        right->node_type == ast_operator_type::AST_OP_ADD ||
		right->node_type == ast_operator_type::AST_OP_SUB ||
		right->node_type == ast_operator_type::AST_OP_MUL ||
		right->node_type == ast_operator_type::AST_OP_DIV ||
		right->node_type == ast_operator_type::AST_OP_MOD ||
		right->node_type == ast_operator_type::AST_OP_ARRAY_INDEX) {
        BinaryInstruction * src2_cmp = new BinaryInstruction(currentFunc,
                                                             IRInstOperator::IRINST_OP_NE_I,
                                                             right->val,
                                                             zero,
                                                             IntegerType::getTypeBool());
        node->blockInsts.addInst(src2_cmp);
        right->val = src2_cmp;
	}

    // 基于结果进行真出口与假出口的传递（如果对应的是and/or运算就无需创建）
    if (right->node_type != ast_operator_type::AST_OP_AND
        && right->node_type != ast_operator_type::AST_OP_OR){
			BranchInstruction * src2_jmp = new BranchInstruction(currentFunc,
															   right->val,
															  trueLabel,
															  falseLabel);
			node->blockInsts.addInst(src2_jmp);
		}


    return true;
}

/// @brief 逻辑运算!AST节点翻译成线性中间IR
/// @param node AST节点
/// @return true 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_not(ast_node * node)
{
    ast_node * src_node = node->sons[0];
	ConstInt * zero = module->newConstInt(0);

    src_node->inherit_label(node);
    if (node->parent->node_type == ast_operator_type::AST_OP_IF ||
        node->parent->node_type == ast_operator_type::AST_OP_WHILE ||
        node->parent->node_type == ast_operator_type::AST_OP_AND ||
        node->parent->node_type == ast_operator_type::AST_OP_OR) {
        src_node->swap_true_false();
    }

    ast_node * src_res = ir_visit_ast_node(src_node);
    if (!src_res) {
        return false;
    }

    node->inherit_label(src_node);
    node->blockInsts.addInst(src_res->blockInsts);
    node->val = src_res->val;

    // TODO 处理条件仅存在单变量取反的情况(if语句中同理)，这里不够完善
    // TODO 计算式也需要处理，但看测试用例再考虑加不加
    if (src_node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID ||
        src_node->node_type == ast_operator_type::AST_OP_FUNC_CALL ||
        src_node->node_type == ast_operator_type::AST_OP_ARRAY_INDEX) {
		BinaryInstruction * eqInst = new BinaryInstruction(module->getCurrentFunction(),
														   IRInstOperator::IRINST_OP_EQ_I,
														   src_res->val,
														   zero,
														   IntegerType::getTypeBool());
        node->blockInsts.addInst(eqInst);
        node->val = eqInst;						// 这里必须将node节点值置为eq指令的目标操作数
	}
    return true;
}


/// @brief if语句块翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_if(ast_node * node)
{
    ast_node * cond_node = node->sons[0];

    // 获取当前作用域使用的函数
    Function * currentFunc = module->getCurrentFunction();
    ConstInt * zero = module->newConstInt(0);

    // 先分别创建三个Label：真入口、假入口、分支出口
    LabelInstruction * trueLabel = new LabelInstruction(currentFunc);
    LabelInstruction * falseLabel = new LabelInstruction(currentFunc);
    LabelInstruction * ifExitLabel = new LabelInstruction(currentFunc);

    //! 条件没用，而且不管怎么样都要继承
    // if (node->parent->node_type == ast_operator_type::AST_OP_BLOCK &&
    //     node->parent->parent->node_type == ast_operator_type::AST_OP_WHILE) {
    node->inherit_label(node->parent);
    // }

    //! 将创建的真假出口保存到节点中，以便条件表达式计算时使用这些出口
    cond_node->set_label(trueLabel, falseLabel, ifExitLabel);


    // 获取cond节点，并生成线性IR
    ast_node * cond_res = ir_visit_ast_node(cond_node);
    if (!cond_res) {
        // 条件中某个变量无定值
        return false;
	}

    //! 注意：这里不能仅仅将指令保存到cond_res节点，而需要保存到分支选择节点内，否则无法遍历到关系运算指令
	node->blockInsts.addInst(cond_res->blockInsts);

    //! 只能当仅有单变量时创建跳转指令，避免与逻辑运算发生冲突
    if (cond_node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID ||
        cond_node->node_type == ast_operator_type::AST_OP_SUB ||
        cond_node->node_type == ast_operator_type::AST_OP_NOT ||
        cond_node->node_type == ast_operator_type::AST_OP_ARRAY_INDEX ||
        cond_node->node_type == ast_operator_type::AST_OP_FUNC_CALL) {
		Value * cond_val;
        if (cond_node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID ||
            cond_node->node_type == ast_operator_type::AST_OP_SUB ||
            cond_node->node_type == ast_operator_type::AST_OP_ARRAY_INDEX ||
            cond_node->node_type == ast_operator_type::AST_OP_FUNC_CALL) {
            BinaryInstruction * neInst = new BinaryInstruction(currentFunc,
                                                               IRInstOperator::IRINST_OP_NE_I,
                                                               cond_res->val,
                                                               zero,
                                                               IntegerType::getTypeBool());
            node->blockInsts.addInst(neInst);
            cond_val = neInst;
        } else {
            cond_val = cond_res->val;
		}
        if (cond_val != nullptr) {
			BranchInstruction * ifBranch = new BranchInstruction(currentFunc,
															   cond_val,
															  trueLabel,
															  falseLabel);
			node->blockInsts.addInst(ifBranch);
		}
	}

    // 接下来开始处理条件为真的语句块
    node->blockInsts.addInst(trueLabel);

    //! 注意：这里同样需要把true_res内的指令添加到本节点的指令块中，否则无法输出
    if (node->sons.size() > 1) {
        ast_node * true_node = node->sons[1];
        true_node->inherit_label(node);

    	ast_node * true_res = ir_visit_ast_node(true_node);
    	node->blockInsts.addInst(true_res->blockInsts);
    }

    // 接下来根据是否有falseBlock进行不同的操作
    if (node->sons.size() > 2) {
        ast_node * false_node = node->sons[2];
		false_node->inherit_label(node);

		// 存在falseBlock,需要插入无条件跳转出口命令
        GotoInstruction * ifExit = new GotoInstruction(currentFunc, ifExitLabel);
        node->blockInsts.addInst(ifExit);

        // 接下来执行else语句块
        node->blockInsts.addInst(falseLabel);

        //! 注意：这里同样需要把false_res内的指令添加到本节点的指令块中，否则无法输出
        ast_node * false_res = ir_visit_ast_node(false_node);
		node->blockInsts.addInst(false_res->blockInsts);

    } else if (node->sons.size() == 1 || node->sons.size() == 2) {
        // 不存在else语句块,证明为空,也不需要无条件跳转
        node->blockInsts.addInst(falseLabel);
    }

    // 最后插入出口标签,回到原函数流程
	node->blockInsts.addInst(ifExitLabel);

    return true;
}

/// @brief while语句块翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_while(ast_node * node)
{
    ast_node * cond_node = node->sons[0];
    ast_node * loop_node = node->sons[1];

	// 获取当前作用域使用的函数
	Function * currentFunc = module->getCurrentFunction();
	ConstInt * zero = module->newConstInt(0);

	// 先分别创建三个Label：循环入口、循环体入口、循环语句出口
	LabelInstruction * whileEntryLabel = new LabelInstruction(currentFunc);
	LabelInstruction * loopEntryLabel = new LabelInstruction(currentFunc);
    LabelInstruction * whileExitLabel = new LabelInstruction(currentFunc);

	//! 将创建的真假出口保存到节点中，以便条件表达式计算时使用这些出口
    cond_node->set_label(loopEntryLabel, whileExitLabel, whileExitLabel);
    // 循环体节点也需要继承标签，以便break/continue使用
    loop_node->set_label(loopEntryLabel, whileEntryLabel, whileExitLabel);

    // 首先插入循环入口标签
    node->blockInsts.addInst(whileEntryLabel);

    // 获取cond节点，并生成线性IR
    ast_node * cond_res = ir_visit_ast_node(cond_node);
    if (!cond_res) {
        // 条件中某个变量无定值
        return false;
    }

    node->blockInsts.addInst(cond_res->blockInsts);

    //! 只能当仅有单变量时创建跳转指令，避免与逻辑运算发生冲突
    if (cond_node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID ||
        cond_node->node_type == ast_operator_type::AST_OP_SUB ||
        cond_node->node_type == ast_operator_type::AST_OP_NOT ||
        cond_node->node_type == ast_operator_type::AST_OP_ARRAY_INDEX ||
        cond_node->node_type == ast_operator_type::AST_OP_FUNC_CALL) {
		Value * cond_val;
        if (cond_node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID ||
            cond_node->node_type == ast_operator_type::AST_OP_SUB ||
            cond_node->node_type == ast_operator_type::AST_OP_ARRAY_INDEX ||
        	cond_node->node_type == ast_operator_type::AST_OP_FUNC_CALL) {
            BinaryInstruction * neInst = new BinaryInstruction(currentFunc,
                                                               IRInstOperator::IRINST_OP_NE_I,
                                                               cond_res->val,
                                                               zero,
                                                               IntegerType::getTypeBool());
            node->blockInsts.addInst(neInst);
            cond_val = neInst;
        } else {
            cond_val = cond_res->val;
		}
        BranchInstruction * ifBranch = new BranchInstruction(currentFunc,
														     cond_val,
														     loopEntryLabel,
														     whileExitLabel);
		node->blockInsts.addInst(ifBranch);
    }

    // 处理循环体
    node->blockInsts.addInst(loopEntryLabel);

    ast_node * loop_res = ir_visit_ast_node(loop_node);
    node->blockInsts.addInst(loop_res->blockInsts);

    // 循环体结束后必须无条件跳转到循环起始位置
    node->blockInsts.addInst(new GotoInstruction(currentFunc, whileEntryLabel));

    // 最后插入循环出口
    node->blockInsts.addInst(whileExitLabel);

    return true;
}

/// @brief break语句块翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_break(ast_node * node)
{
    auto breakLabel = node->parent->endLabel;
    if (node->parent->node_type == ast_operator_type::AST_OP_WHILE) {
        breakLabel = node->endLabel;
	}
    node->blockInsts.addInst(new GotoInstruction(module->getCurrentFunction(), breakLabel));
    return true;
}

/// @brief continue语句块翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_continue(ast_node * node)
{
    auto continueLabel = node->parent->falseLabel;
    if (node->parent->node_type == ast_operator_type::AST_OP_WHILE) {
        continueLabel = node->falseLabel;
	}
    node->blockInsts.addInst(new GotoInstruction(module->getCurrentFunction(), continueLabel));
    return true;
}

/// @brief 赋值AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_assign(ast_node * node)
{
    ast_node * son1_node = node->sons[0];
    ast_node * son2_node = node->sons[1];

    // 赋值节点，自右往左运算

    // 赋值运算符的左侧操作数
    ast_node * left = ir_visit_ast_node(son1_node);
    if (!left) {
        // 某个变量没有定值
        // 这里缺省设置变量不存在则创建，因此这里不会错误
        return false;
    }

    // 赋值运算符的右侧操作数
    ast_node * right = ir_visit_ast_node(son2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

	node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(left->blockInsts);

    Value * res = nullptr;
    Value * right_val = right->val;
    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
    if (right->node_type == ast_operator_type::AST_OP_ARRAY_INDEX) {
        LoadInstruction * loadInst = new LoadInstruction(module->getCurrentFunction(),
                                                         right->val,
                                                         IntegerType::getTypeInt());
        node->blockInsts.addInst(loadInst);
        right_val = loadInst;
        if (left->node_type != ast_operator_type::AST_OP_ARRAY_INDEX) {
            MoveInstruction * movInst = new MoveInstruction(module->getCurrentFunction(), left->val, loadInst);
			// 创建临时变量保存IR的值，以及线性IR指令
        	node->blockInsts.addInst(movInst);
        	res = movInst;
		}
    } else if (left->node_type != ast_operator_type::AST_OP_ARRAY_INDEX) {
		MoveInstruction * movInst = new MoveInstruction(module->getCurrentFunction(), left->val, right->val);
		// 创建临时变量保存IR的值，以及线性IR指令
        node->blockInsts.addInst(movInst);
        res = movInst;
	}

	if (left->node_type == ast_operator_type::AST_OP_ARRAY_INDEX) {
        StoreInstruction * storeInst = new StoreInstruction(module->getCurrentFunction(),
                                                         left->val,
                                                          right_val);
        node->blockInsts.addInst(storeInst);
        res = storeInst;
	}

    // 这里假定赋值的类型是一致的
    node->val = res;

    return true;
}

/// @brief return节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_return(ast_node * node)
{
    ast_node * right = nullptr;

    // return语句可能没有没有表达式，也可能有，因此这里必须进行区分判断
    if (!node->sons.empty()) {

        ast_node * son_node = node->sons[0];

        // 返回的表达式的指令保存在right节点中
        right = ir_visit_ast_node(son_node);
        if (!right) {

            // 某个变量没有定值
            return false;
        }
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
    Function * currentFunc = module->getCurrentFunction();

    // 返回值存在时则移动指令到node中
    if (right) {

        // 创建临时变量保存IR的值，以及线性IR指令
        node->blockInsts.addInst(right->blockInsts);

        if (right->node_type == ast_operator_type::AST_OP_ARRAY_INDEX) {
            LoadInstruction * loadInst = new LoadInstruction(module->getCurrentFunction(),
                                                          right->val,
                                                             IntegerType::getTypeInt());
            node->blockInsts.addInst(loadInst);

            node->blockInsts.addInst(new MoveInstruction(currentFunc, currentFunc->getReturnValue(), loadInst));
        	node->val = loadInst;
        } else {
            // 返回值赋值到函数返回值变量上，然后跳转到函数的尾部
        	node->blockInsts.addInst(new MoveInstruction(currentFunc, currentFunc->getReturnValue(), right->val));

        	node->val = right->val;
		}

    } else {
        // 没有返回值
        node->val = nullptr;
    }

    // 跳转到函数的尾部出口指令上
    node->blockInsts.addInst(new GotoInstruction(currentFunc, currentFunc->getExitLabel()));

    return true;
}

/// @brief 类型叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_type(ast_node * node)
{
    // 不需要做什么，直接从节点中获取即可。

    return true;
}

/// @brief 标识符叶子节点翻译成线性中间IR，变量声明的不走这个语句
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_var_id(ast_node * node)
{
    Value * val;

    // 查找ID型Value
    // 变量，则需要在符号表中查找对应的值

    val = module->findVarValue(node->name);

    node->val = val;

    node->type = val->getType();

    return true;
}

/// @brief 无符号整数字面量叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_uint(ast_node * node)
{
    ConstInt * val;

    // 新建一个整数常量Value
    val = module->newConstInt((int32_t) node->integer_val);

    node->val = val;

    return true;
}

/// @brief 变量声明语句节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_declare_statment(ast_node * node)
{
    bool result = false;

    for (auto & child: node->sons) {

        // 遍历每个变量声明
        result = ir_variable_declare(child);
        if (!result) {
            break;
        }
		node->blockInsts.addInst(child->blockInsts);
    }

    return result;
}

/// @brief 变量定声明节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_variable_declare(ast_node * node)
{
    // 共有两个孩子，第一个类型，第二个变量名

    // TODO 这里可强化类型等检查

    auto varName = node->sons[1]->name;
    Type * varType = node->type;

    if (varType->isArrayType()) {

        const Type * pointer_to_array = PointerType::get(varType);
        node->val = module->newVarValue(const_cast<Type *>(pointer_to_array), varName);

        Instanceof(var2array, ArrayType *, varType);
        if (node->sons.size() > var2array->getDepth() + 2) {				// size > type + id + dim_nodes.size
			ast_node * array_init_node = node->sons[var2array->getDepth() + 2];
            bool res = ir_visit_ast_node(array_init_node);
            if (!res) {
                return false;
			}
		}
    } else {

        node->val = module->newVarValue(node->sons[0]->type, node->sons[1]->name);

        Instanceof(val2global, GlobalVariable *, node->val);
        if (node->sons.size() > 2 && val2global == nullptr) {
			ast_node * init = ir_visit_ast_node(node->sons[2]);
			if (!init) {
				return false;
			}
			MoveInstruction * movInst = new MoveInstruction(module->getCurrentFunction(), node->val, init->val);

			node->blockInsts.addInst(init->blockInsts);
			node->blockInsts.addInst(movInst);
        } else if (node->sons.size() > 2 && val2global != nullptr) {
            ast_node * init = ir_visit_ast_node(node->sons[2]);
			if (!init) {
				return false;
            }
            if (init->val != nullptr) {
                Instanceof(value2constInt, ConstInt *, init->val);
                if (value2constInt == nullptr) {
                    // 说明该值为一个负数，需要单独进行处理
                    value2constInt = module->newConstInt(0-init->sons[0]->integer_val);
				}
                val2global->setInitVal(value2constInt);
			}
		}
	}
    return true;
}

/// @brief 数组初始化节点翻译成线性中间IR
/// @param node AST节点
bool IRGenerator::ir_array_init(ast_node * node)
{
    // TODO: 等待实现
    return false;
}

/// @brief 数组索引对应的左值节点翻译成线性中间IR
/// @param node AST节点
bool IRGenerator::ir_array_index(ast_node * node)
{
    ast_node * base_node = node->sons[0];
    ast_node * index_node = node->sons[1];

    ast_node * visited_index = ir_visit_ast_node(index_node);
    node->blockInsts.addInst(visited_index->blockInsts);

    ast_node * visited_base = ir_visit_ast_node(base_node);
    node->blockInsts.addInst(visited_base->blockInsts);

    // 为本节点保存数组基址
    if (visited_base->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
        node->base_addr = visited_base->val;
    } else {
        node->base_addr = visited_base->base_addr;
    }

    // 将指针类型解套，取出真正的数据类型
    auto baseType = base_node->type;
    const Type * pointeeType = nullptr;
    if (baseType->isPointerType()) {
        Instanceof(base2ptr, PointerType *, baseType);
        pointeeType = base2ptr->getPointeeType();
        Instanceof(pointee2array, const ArrayType *, pointeeType);
        node->arrayDepth = pointee2array->getDepth();
        node->currentDepth = 0;
    } else if (baseType->isArrayType()) {
        pointeeType = baseType;
    }

    // 继承数组深度
    if (visited_base->node_type != ast_operator_type::AST_OP_LEAF_VAR_ID) {
        node->arrayDepth = visited_base->arrayDepth;
        node->currentDepth = visited_base->currentDepth;
    }

    // 获取元素类型
    const Type * elementType = nullptr;
    if (pointeeType->isArrayType()) {
        Instanceof(pointee2array,const ArrayType *, pointeeType);
        elementType = pointee2array->getElementType();
        node->currentDepth++;
    }

	// 获取元素个数/根类型大小
    ConstInt * numElements = nullptr;
    if (elementType->isArrayType()) {
        elementType = dynamic_cast<const ArrayType *>(elementType);
        numElements = module->newConstInt(static_cast<const ArrayType *>(elementType)->getNumElements());
    } else {
        numElements = module->newConstInt(elementType->getSize());
	}

    // 选择源操作数，给出mul指令
    Value * mulSrc = nullptr;
    if (node->sons[0]->node_type != ast_operator_type::AST_OP_ARRAY_INDEX) {
        mulSrc = visited_index->val;
    } else {
        mulSrc = visited_base->val;
	}
    BinaryInstruction * mulInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MUL_I,
                                                        mulSrc,
                                                        numElements,
                                                        IntegerType::getTypeInt());
    node->blockInsts.addInst(mulInst);

    // 选择源操作数，给出add指令
    Value * addSrc = nullptr;
    if (node->parent->node_type != ast_operator_type::AST_OP_ARRAY_INDEX ||
        node->currentDepth == node->arrayDepth) {
        addSrc = node->base_addr;
    } else {
        //! 这么写有可能指向自身
        addSrc = node->parent->sons[1]->val;
    }

    // 为add指令的值确定类型
    Type * addInstType = nullptr;
    if (node->parent->node_type != ast_operator_type::AST_OP_ARRAY_INDEX ||
        (node->parent->node_type == ast_operator_type::AST_OP_ARRAY_INDEX && node->currentDepth == node->arrayDepth)) {
		addInstType = new PointerType(IntegerType::getTypeInt());
    } else {
        addInstType = IntegerType::getTypeInt();
    }

    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ADD_I,
                                                        mulInst,
                                                        addSrc,
                                                        addInstType);
    node->blockInsts.addInst(addInst);

    node->type = const_cast<Type *>(elementType);

    // 将地址对应值加载到临时变量中
    if ((node->parent->node_type != ast_operator_type::AST_OP_ARRAY_INDEX &&
         node->parent->node_type != ast_operator_type::AST_OP_ASSIGN &&
         node->parent->node_type != ast_operator_type::AST_OP_RETURN &&
         node->parent->node_type != ast_operator_type::AST_OP_FUNC_REAL_PARAMS) ||
        (node->parent->node_type == ast_operator_type::AST_OP_ARRAY_INDEX && node->currentDepth == node->arrayDepth)) {
        LoadInstruction * loadInst = new LoadInstruction(module->getCurrentFunction(),
                                                         addInst,
                                                         IntegerType::getTypeInt());
        node->blockInsts.addInst(loadInst);
        node->currentDepth = node->arrayDepth = -1;					// 好像不是特别重要
        node->val = loadInst;
    } else {
        node->val = addInst;
    }

    return true;
}
