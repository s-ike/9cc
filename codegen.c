#include "9cc.h"

static void gen_addr(Node *node)
{
	if (node->kind == ND_VAR)
	{
		printf("  mov rax, rbp\n");
		printf("  sub rax, %d\n", node->var->offset);
		printf("  push rax\n");
		return;
	}
	error("not an lvalue");
}

static void gen(Node *node)
{
	switch (node->kind)
	{
	case ND_NUM:
		printf("  push %ld\n", node->val);
		return;
	case ND_VAR:
		gen_addr(node);
		printf("  pop rax\n");
		printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return;
	case ND_ASSIGN:
		gen_addr(node->lhs);
		gen(node->rhs);
		printf("  pop rdi\n");
		printf("  pop rax\n");
		printf("  mov [rax], rdi\n");
		printf("  push rdi\n");
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

void codegen(Node *node)
{
	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// Prologue
	// 変数26個分の領域(26 * 8)を確保する
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	printf("  sub rsp, 208\n");

	for (Node *n = node; n; n = n->next)
	{
		gen(n);
		printf("  pop rax\n");
	}

	// Epilogue
	// 最後の式の結果がRAXに残っているのでそれが返り値になる
	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");
}
