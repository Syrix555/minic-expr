
// Generated from MiniC.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"
#include "MiniCVisitor.h"


/**
 * This class provides an empty implementation of MiniCVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  MiniCBaseVisitor : public MiniCVisitor {
public:

  virtual std::any visitCompileUnit(MiniCParser::CompileUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncDef(MiniCParser::FuncDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncType(MiniCParser::FuncTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncFParams(MiniCParser::FuncFParamsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncFParam(MiniCParser::FuncFParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlock(MiniCParser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlockItemList(MiniCParser::BlockItemListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlockItem(MiniCParser::BlockItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVarDecl(MiniCParser::VarDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBasicType(MiniCParser::BasicTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVarDef(MiniCParser::VarDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnStatement(MiniCParser::ReturnStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignStatement(MiniCParser::AssignStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlockStatement(MiniCParser::BlockStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionStatement(MiniCParser::ExpressionStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfStatement(MiniCParser::IfStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhileStatement(MiniCParser::WhileStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBreakStatement(MiniCParser::BreakStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitContinueStatement(MiniCParser::ContinueStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpr(MiniCParser::ExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCond(MiniCParser::CondContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddExp(MiniCParser::AddExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddOp(MiniCParser::AddOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMulExp(MiniCParser::MulExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMulOp(MiniCParser::MulOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelExp(MiniCParser::RelExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelOp(MiniCParser::RelOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqExp(MiniCParser::EqExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqOp(MiniCParser::EqOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLAndExp(MiniCParser::LAndExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLAndOp(MiniCParser::LAndOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLOrExp(MiniCParser::LOrExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLOrOp(MiniCParser::LOrOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitVal(MiniCParser::InitValContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfStmt(MiniCParser::IfStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhileStmt(MiniCParser::WhileStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBreakStmt(MiniCParser::BreakStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitContinueStmt(MiniCParser::ContinueStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExp(MiniCParser::UnaryExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryOp(MiniCParser::UnaryOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryExp(MiniCParser::PrimaryExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRealParamList(MiniCParser::RealParamListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLVal(MiniCParser::LValContext *ctx) override {
    return visitChildren(ctx);
  }


};

