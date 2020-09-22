#include "9cc.h"

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

static Node *stmt(void);
static Node *expr(void);
static Node *equality(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void);
static Node *unary(void);
static Node *primary(void);

Node *program(void)
{
	Node	head = {};
	Node	*cur = &head;

	while (!at_eof())
	{
		cur->next = stmt();
		cur = cur->next;
	}
	return head.next;
}

static Node *stmt(void)
{
	Node	*node = expr();

	expect(";");
	return node;
}

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
