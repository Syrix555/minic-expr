grammar MiniC;

// 词法规则名总是以大写字母开头

// 语法规则名总是以小写字母开头

// 每个非终结符尽量多包含闭包、正闭包或可选符等的EBNF范式描述

// 若非终结符由多个产生式组成，则建议在每个产生式的尾部追加# 名称来区分，详细可查看非终结符stmt的描述

// 语法规则描述：EBNF范式

// 源文件编译单元定义
compileUnit: (funcDef | varDecl)* EOF;

// 函数定义，目前支持形参，也支持返回void类型等
funcDef: funcType T_ID T_L_PAREN funcFParams? T_R_PAREN block;

// 函数返回值类型
funcType: T_INT | T_VOID;

// 函数形参表定义：
funcFParams: funcFParam (T_COMMA funcFParam)*;

// 函数形参定义
funcFParam: basicType T_ID (T_L_BRACKET expr T_R_BRACKET)*;

// 语句块看用作函数体，这里允许多个语句，并且不含任何语句
block: T_L_BRACE blockItemList? T_R_BRACE;

// 每个ItemList可包含至少一个Item
blockItemList: blockItem+;

// 每个Item可以是一个语句，或者变量声明语句
blockItem: stmt | varDecl;

// 变量声明，目前支持变量含有初值
varDecl: basicType varDef (T_COMMA varDef)* T_SEMICOLON;

// 基本类型
basicType: T_INT;

// 变量定义，增加数组
varDef: T_ID (T_L_BRACKET expr T_R_BRACKET)* (T_ASSIGN initVal)?;

// 目前语句支持return和赋值语句
stmt:
	T_RETURN expr T_SEMICOLON			# returnStatement
	| lVal T_ASSIGN expr T_SEMICOLON	# assignStatement
	| block								# blockStatement
	| expr? T_SEMICOLON					# expressionStatement
	| ifStmt							# ifStatement
	| whileStmt                         # whileStatement
	| breakStmt							# breakStatement
	| continueStmt						# continueStatement;

// 表达式文法 expr : 表达式目前支持加法与减法，乘除法运算
expr: lOrExp;

// 条件表达式 cond : 目前支持关系表达式、相等性表达式和逻辑表达式
cond: lOrExp;

// 加减表达式，将 unaryExp 修改为 mulExp ，表示 * / % 比 + - 更先
addExp: mulExp (addOp mulExp)*;

// 加减运算符
addOp: T_ADD | T_SUB;

// 乘除表达式
mulExp: unaryExp (mulOp unaryExp)*;

// 乘除运算符
mulOp: T_MUL | T_DIV | T_MOD;

// 关系表达式
relExp: addExp (relOp addExp)*;

// 关系运算符
relOp: T_LT | T_GT | T_LE | T_GE;

// 相等性表达式
eqExp: relExp (eqOp relExp)*;

// 相等性运算符
eqOp: T_EQ | T_NE;

// 逻辑与表达式
lAndExp: eqExp (lAndOp eqExp)*;

// 逻辑与运算符
lAndOp: T_AND;

// 逻辑或表达式
lOrExp: lAndExp (lOrOp lAndExp)*;

// 逻辑或运算符
lOrOp: T_OR;

// 变量初值
initVal: expr | T_L_BRACE initVal (T_COMMA initVal)* T_R_BRACE;

// if语句
ifStmt: T_IF T_L_PAREN cond T_R_PAREN stmt (T_ELSE stmt)?;

// while语句
whileStmt: T_WHILE T_L_PAREN cond T_R_PAREN stmt;

// break语句
breakStmt : T_BREAK T_SEMICOLON;

// continue语句
continueStmt: T_CONTINUE T_SEMICOLON;

// 一元表达式
unaryExp:
	primaryExp
	| T_ID T_L_PAREN realParamList? T_R_PAREN
	| unaryOp unaryExp;

// 一元运算符
unaryOp: T_NOT | T_SUB;

// 基本表达式：括号表达式、整数、左值表达式
primaryExp: T_L_PAREN expr T_R_PAREN | T_DIGIT | lVal;

// 实参列表
realParamList: expr (T_COMMA expr)*;

// 左值表达式
lVal: T_ID (T_L_BRACKET expr T_R_BRACKET)*;

// 用正规式来进行词法规则的描述

T_L_PAREN: '(';
T_R_PAREN: ')';
T_SEMICOLON: ';';
T_L_BRACE: '{';
T_R_BRACE: '}';
T_L_BRACKET: '[';
T_R_BRACKET: ']';

T_ASSIGN: '=';
T_COMMA: ',';

T_ADD: '+';
T_SUB: '-';
T_MUL: '*';
T_DIV: '/';
T_MOD: '%';

T_LT: '<';
T_GT: '>';
T_LE: '<=';
T_GE: '>=';

T_EQ: '==';
T_NE: '!=';

T_AND: '&&';
T_OR: '||';
T_NOT: '!';

// 要注意关键字同样也属于T_ID，因此必须放在T_ID的前面，否则会识别成T_ID
T_RETURN: 'return';
T_INT: 'int';
T_VOID: 'void';
T_IF: 'if';
T_ELSE: 'else';
T_WHILE: 'while';
T_BREAK: 'break';
T_CONTINUE: 'continue';

T_ID: [a-zA-Z_][a-zA-Z0-9_]*;
T_DIGIT: '0' [xX] [0-9a-fA-F]+ |
		 '0' [0-7]+ |
		 [1-9][0-9]* |
		 '0' ;					//增加对8/16进制的支持

/* 空白符丢弃 */
WS: [ \r\n\t]+ -> skip;

// 丢弃注释
LINE_COMMENT: '//' ~[\r\n]* -> channel(HIDDEN);

// 多行注释
BLOCK_COMMENT: '/*' .*? '*/' -> channel(HIDDEN);