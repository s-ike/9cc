#ifndef __9CC_H__
#define __9CC_H__

#define _GNU_SOURCE
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// tokenize.c
//

// トークンの種類
typedef enum
{
	TK_RESERVED,	// 記号
	TK_INDENT,		// 識別子
	TK_NUM,			// 整数トークン
	TK_EOF,			// 入力の終わりを表すトークン
} TokenKind;

// トークン型
typedef struct Token Token;
struct Token
{
	TokenKind	kind;	// トークンの型
	Token		*next;	// 次の入力トークン
	int			val;	// KindがTK_NUMの場合、その数値
	char		*str;	// トークン文字列
	int			len;	// トークンの長さ
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident(void);
void expect(char *op);
long expect_number();
bool at_eof();
Token *tokenize(void);

extern char		*user_input;
extern Token	*token;

//
// parse.c
//

// Local variable
typedef struct Var Var;
struct Var
{
	Var		*next;
	char	*name;	// Variable name
	int		offset;	// Offset from RBP
};

// 抽象構文木のノードの種類
typedef enum
{
	ND_ADD,		// +
	ND_SUB,		// -
	ND_MUL,		// *
	ND_DIV,		// /
	ND_EQ,		// ==
	ND_NE,		// !=
	ND_LT,		// <
	ND_LE,		// <=
	ND_ASSIGN,	// =
	ND_RETURN,	// "return"
	ND_VAR,		// Variable
	ND_NUM,		// 数値
} NodeKind;

// 抽象構文木（AST）のノードの型
typedef struct Node Node;
struct Node
{
	NodeKind	kind;	// ノードの型
	Node		*next;	// 次のノード
	Node		*lhs;	// 左辺
	Node		*rhs;	// 右辺
	Var			*var;	// kindがND_VARの場合のみ使用
	long		val;	// kindがND_NUMの場合のみ使用
};

Node *program(void);

//
// Code generator
//

void codegen(Node *node);

#endif
