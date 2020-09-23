#include "9cc.h"

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

// 次のトークンが変数のときには、トークンを1つ読み進めて
// トークンを返す。それ以外の場合にはNULLを返す。
Token *consume_ident(void)
{
	if (token->kind != TK_INDENT)
		return NULL;

	Token	*t = token;
	token = token->next;
	return t;
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
long expect_number()
{
	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません");
	long	val = token->val;
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

static bool startswith(char *p, char *q)
{
	return memcmp(p, q, strlen(q)) == 0;
}

static bool is_alpha(char c)
{
	return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c == '_';
}

static bool is_alnum(char c)
{
	return is_alpha(c) || ('0' <= c && c <= '9');
}

// 入力文字列user_inputをトークナイズしてそれを返す
Token *tokenize(void)
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
		// Identifier
		if (is_alpha(*p))
		{
			char	*q = p++;
			while (is_alnum(*p))
				p++;
			cur = new_token(TK_INDENT, cur, q, p - q);
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
		if (ispunct(*p))
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
