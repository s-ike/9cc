#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Tokenizer
//

// トークンの種類
typedef enum
{
	TK_RESERVED,	// 記号
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

// 入力プログラム
char	*user_input;

// 現在着目しているトークン
Token	*token;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...)
{
	va_list	ap;
	int		pos;

	va_start(ap, fmt);
	pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, "");	// pos個の空白を出力
	fprintf(stderr, "^");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op)
{
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
		return false;
	token = token->next;
	return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op)
{
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
		error_at(token->str, "'%s'ではありません", op);
	token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number()
{
	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません");
	int	val = token->val;
	token = token->next;
	return val;
}

bool at_eof()
{
	return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
	Token	*tok = calloc(1, sizeof(Token));

	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool startswith(char *p, char *q)
{
	return memcmp(p, q, strlen(q)) == 0;
}

// 入力文字列user_inputをトークナイズしてそれを返す
Token *tokenize()
{
	Token	head;
	Token	*cur;
	char	*p = user_input;

	head.next = NULL;
	cur = &head;

	while (*p)
	{
		// Skip whitespace characters.
		if (isspace(*p))
		{
			p++;
			continue;
		}
		// Multi-letter punctuator
		if (startswith(p, "==") || startswith(p, "!=") ||
			startswith(p, "<=") || startswith(p, ">="))
		{
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}
		// Single-letter punctuator
		if (strchr("+-*/()<>", *p))
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}
		// Integer literal
		if (isdigit(*p))
		{
			cur = new_token(TK_NUM, cur, p, 0);
			char	*q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = q - p;
			continue;
		}
		error_at(p, "トークナイズできません");
	}
	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

//
// Parser
//

// 抽象構文木のノードの種類
typedef enum
{
	ND_ADD,	// +
	ND_SUB,	// -
	ND_MUL,	// *
	ND_DIV,	// /
	ND_EQ,	// ==
	ND_NE,	// !=
	ND_LT,	// <
	ND_LE,	// <=
	ND_NUM,	// 数値
} NodeKind;

// 抽象構文木（AST）のノードの型
typedef struct Node Node;
struct Node
{
	NodeKind	kind;	// ノードの型
	Node		*lhs;	// 左辺
	Node		*rhs;	// 右辺
	int			val;	// kindがND_NUMの場合のみ使用
};

static Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node	*node;

	node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

static Node *new_node_num(int val)
{
	Node	*node;

	node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

static Node *expr(void);
static Node *equality(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void);
static Node *unary(void);
static Node *primary(void);

static Node *expr(void)
{
	return equality();
}

static Node *equality(void)
{
	Node	*node = relational();

	while (1)
	{
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NE, node, relational());
		else
			return node;
	}
}

static Node *relational(void)
{
	Node	*node = add();

	while (1)
	{
		if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume("<="))
			node = new_node(ND_LE, node, add());
		else if (consume(">"))
			node = new_node(ND_LT, add(), node);
		else if (consume(">="))
			node = new_node(ND_LE, add(), node);
		else
			return node;
	}
}

static Node *add(void)
{
	Node	*node = mul();

	while (1)
	{
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return node;
	}
}

static Node *mul(void)
{
	Node	*node = unary();

	while (1)
	{
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return node;
	}
}

static Node *unary(void)
{
	if (consume("+"))
		return unary();
	else if (consume("-"))
		return new_node(ND_SUB, new_node_num(0), unary());
	else
		return primary();
}

static Node *primary(void)
{
	if (consume("("))
	{
		Node *node = expr();
		expect(")");
		return node;
	}
	// 数値
	return new_node_num(expect_number());
}

//
// Code generator
//

static void gen(Node *node)
{
	if (node->kind == ND_NUM)
	{
		printf("  push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->kind)
	{
	case ND_ADD:
		printf("  add rax, rdi\n");
		break;
	case ND_SUB:
		printf("  sub rax, rdi\n");
		break;
	case ND_MUL:
		printf("  imul rax, rdi\n");
		break;
	case ND_DIV:
		printf("  cqo\n");
		printf("  idiv rdi\n");
		break;
	case ND_EQ:
		printf("  cmp rax, rdi\n");
		printf("  sete al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_NE:
		printf("  cmp rax, rdi\n");
		printf("  setne al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_LT:
		printf("  cmp rax, rdi\n");
		printf("  setl al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_LE:
		printf("  cmp rax, rdi\n");
		printf("  setle al\n");
		printf("  movzb rax, al\n");
		break;
	}

	printf("  push rax\n");
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		error("引数の個数が正しくありません");
		return 1;
	}

	// トークナイズしてパースする
	user_input = argv[1];
	token = tokenize();
	Node *node = expr();

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// 抽象構文木を下りながらコード生成
	gen(node);

	// スタックトップに式全体の値が残っているはずなので
	// それをRAXにロードして関数からの返り値とする
	printf("  pop rax\n");
	printf("  ret\n");
	return 0;
}
