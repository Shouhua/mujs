/**
<expression> ::= <term> { ['+' | '-'] <expression> }
<term> ::= <number> { ['*' | '/'] <term> }
<number> ::= [0-9]+

LL(1)
*/

#include <stdio.h>
#include <stdlib.h>

enum
{
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_DIVID,
	TOKEN_MULTIPLY,

	TOKEN_NUMBER
} TOKEN_TYPE;

typedef struct
{
	const char *source;
	int index;
	int token_type;
	int tk_num;
} Ctx;

int skip_space(Ctx *ctx)
{
	while (*ctx->source == ' ')
		ctx->source++;
	return 0;
}

int parse_expression(Ctx *ctx);
int parse_term(Ctx *ctx);
int parse_digit(Ctx *ctx);
int lex(Ctx *ctx);

int parse_expression(Ctx *ctx)
{
	int token_type;
	int v1 = parse_term(ctx); // 1*2 2/1
	lex(ctx);
	if (ctx->token_type != TOKEN_PLUS && ctx->token_type != TOKEN_MINUS)
	{
		ctx->source--;
		return v1;
	}
	token_type = ctx->token_type;
	int v2 = parse_expression(ctx);
	if (token_type == TOKEN_PLUS)
		return v1 + v2;
	return v1 - v2;
}

// divid multiply
int parse_term(Ctx *ctx)
{
	int token_type;
	int v1 = parse_digit(ctx);
	lex(ctx);
	if (ctx->token_type != TOKEN_DIVID && ctx->token_type != TOKEN_MULTIPLY) {
		ctx->source--;
		return v1;
	}
	token_type = ctx->token_type;
	int v2 = parse_term(ctx);
	if (token_type == TOKEN_DIVID)
		return v1 / v2; // TODO v2 must not ZERO
	return v1 * v2;
}

int parse_digit(Ctx *ctx)
{
	lex(ctx);
	if (ctx->token_type != TOKEN_NUMBER)
	{
		fprintf(stderr, "token not NUMBER");
		exit(-1);
	}
	return ctx->tk_num;
}

int lex(Ctx *ctx)
{
#define isdigit(c) (c - '0' >= 0 && c - '0' <= 9)
	skip_space(ctx);
	ctx->tk_num = 0;
	if (isdigit(*ctx->source))
	{
		while (isdigit(*ctx->source))
		{
			ctx->token_type = TOKEN_NUMBER;
			ctx->tk_num = ctx->tk_num * 10 + (*ctx->source - '0');

			ctx->source++;
		}
		return 0;
	}
	else if (*ctx->source == '+')
	{
		ctx->token_type = TOKEN_PLUS;
		ctx->source++;
		return 0;
	}
	else if (*ctx->source == '-')
	{
		ctx->token_type = TOKEN_MINUS;
		ctx->source++;
		return 0;
	}
	else if (*ctx->source == '*')
	{
		ctx->token_type = TOKEN_MULTIPLY;
		ctx->source++;
		return 0;
	}
	else if (*ctx->source == '/')
	{
		ctx->token_type = TOKEN_DIVID;
		ctx->source++;
		return 0;
	}
	else
	{
		return -1;
	}
	return -1;
}

Ctx init_ctx(const char *source)
{
	Ctx ctx;
	ctx.source = source;
	ctx.index = 0;
	ctx.token_type = -1;
	ctx.tk_num = -1;
	return ctx;
}

int main(int argc __attribute__((__unused__)), char *argv[])
{
	char *source = argv[1];
	Ctx ctx = init_ctx(source);
	// lex(&ctx);
	fprintf(stdout, "result is: %d\n", parse_expression(&ctx));

	return 0;
}