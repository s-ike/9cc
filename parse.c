#include "9cc.h"

// All local variable instances created during parsing are
// accumulated to this list.
Var	*locals;

// Find a local variable by name.
static Var *find_var(Token *tok)
{
	for (Var *var = locals; var; var = var->next)
		if (strlen(var->name) == tok->len && !strncmp(tok->str, var->name, tok->len))
			return var;
	return NULL;
}

static Node *new_node(NodeKind kind)
{
	Node	*node;

	node = calloc(1, sizeof(Node));
	node->kind = kind;
	return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
	Node	*node;

	node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

static Node *new_unary(NodeKind kind, Node *expr)
{
	Node	*node;

	node = new_node(kind);
	node->lhs = expr;
	return node;
}

static Node *new_num(long val)
{
	Node	*node;

	node = new_node(ND_NUM);
	node->val = val;
	return node;
}

static Node *new_var_node(Var *var)
{
	Node	*node;

	node = new_node(ND_VAR);
	node->var = var;
	return node;
}

static Var *new_lvar(char *name)
{
	Var	*var;

	var = calloc(1, sizeof(Var));
	var->next = locals;
	var->name = name;
	if (locals)
		var->offset = locals->offset + 8;
	else
		var->offset = 8;
	locals = var;
	return var;
}

static Node *stmt(void);
static Node *expr(void);
static Node *assign(void);
static Node *equality(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void);
static Node *unary(void);
static Node *primary(void);

// program = stmt*
Node *program(void)
{
	Node	head = {};
	Node	*cur = &head;

	locals = NULL;
	while (!at_eof())
	{
		cur->next = stmt();
		cur = cur->next;
	}
	return head.next;
}

// stmt = "return" expr ";"
//      | expr ";"
static Node *stmt(void)
{
	Node	*node;

	if (consume("return"))
		node = new_unary(ND_RETURN, expr());
	else
		node = expr();
	expect(";");
	return node;
}

// expr = assign
static Node *expr(void)
{
	return assign();
}

// assign = equality ("=" assign)?
static Node *assign(void)
{
	Node	*node = equality();

	if (consume("="))
		node = new_binary(ND_ASSIGN, node, assign());
	return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality(void)
{
	Node	*node = relational();

	while (1)
	{
		if (consume("=="))
			node = new_binary(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_binary(ND_NE, node, relational());
		else
			return node;
	}
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(void)
{
	Node	*node = add();

	while (1)
	{
		if (consume("<"))
			node = new_binary(ND_LT, node, add());
		else if (consume("<="))
			node = new_binary(ND_LE, node, add());
		else if (consume(">"))
			node = new_binary(ND_LT, add(), node);
		else if (consume(">="))
			node = new_binary(ND_LE, add(), node);
		else
			return node;
	}
}

// add = mul ("+" mul | "-" mul)*
static Node *add(void)
{
	Node	*node = mul();

	while (1)
	{
		if (consume("+"))
			node = new_binary(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_binary(ND_SUB, node, mul());
		else
			return node;
	}
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul(void)
{
	Node	*node = unary();

	while (1)
	{
		if (consume("*"))
			node = new_binary(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_binary(ND_DIV, node, unary());
		else
			return node;
	}
}

// unary = ("+" | "-")? unary
//       | primary
static Node *unary(void)
{
	if (consume("+"))
		return unary();
	if (consume("-"))
		return new_binary(ND_SUB, new_num(0), unary());
	return primary();
}

// primary = "(" expr ")" | ident | num
static Node *primary(void)
{
	if (consume("("))
	{
		Node *node = expr();
		expect(")");
		return node;
	}
	Token	*tok = consume_ident();
	if (tok)
	{
		Var	*var = find_var(tok);
		if (!var)
			var = new_lvar(strndup(tok->str, tok->len));
		return new_var_node(var);
	}
	// 数値
	return new_num(expect_number());
}
