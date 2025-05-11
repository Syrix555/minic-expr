///
/// @file BranchInstruction.h
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
#pragma once

#include "Instruction.h"
#include "LabelInstruction.h"
#include "Function.h"
#include "Value.h"

///
/// @brief 分支跳转指令
///
class BranchInstruction final : public Instruction {

	public:
        ///
        /// @brief 分支跳转指令的构造函数
        /// @param _srcval 源操作数
        /// @param _target1 跳转目标1
        /// @param _target2 跳转目标2
		///
        BranchInstruction(Function * _func,
                          Value * _srcval,
                          Instruction * _target1,
                          Instruction * _target2);

		/// @brief 转换成字符串
		void toString(std::string & str) override;

		///
		/// @brief 获取目标Label1指令
		/// @return LabelInstruction*
		///
        [[nodiscard]] LabelInstruction * getTarget1() const;

        ///
		/// @brief 获取目标Label2指令
		/// @return LabelInstruction*
		///
		[[nodiscard]] LabelInstruction * getTarget2() const;

	private:
		///
		/// @brief 跳转到的目标Label指令
		///
        LabelInstruction * target1;
        LabelInstruction * target2;
	};