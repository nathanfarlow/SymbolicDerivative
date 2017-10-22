#include "ast.h"

#include <stdlib.h> //for malloc
#include <string.h> //for memcpy
#include <stdio.h> //for sprintf

identifier_t identifiers[AMOUNT_TOKENS] = {
	{NODE_NUMBER, TOK_NUMBER, 0, 0},
	{NODE_SYMBOL, TOK_SYMBOL, 0, 0},
	{NODE_BINARY, TOK_ADD, 1, 0x70},
	{NODE_BINARY, TOK_SUBTRACT, 1, 0x71},
	{NODE_BINARY, TOK_MULTIPLY, 1, 0x82},
	{NODE_BINARY, TOK_DIVIDE, 1, 0x83},
	{NODE_BINARY, TOK_FRACTION, 2, {0xEF, 0x2E}},
	{NODE_BINARY, TOK_NEGATE, 1, 0xB0},
	{NODE_BINARY, TOK_POWER, 1, 0xF0},
	{-1, TOK_OPEN_PAR, 0, 0},
	{-1, TOK_CLOSE_PAR, 0, 0},
};

double num_ToDouble(num_t num) {
	char buffer[20] = { 0 };

	memcpy(buffer, num.number, num.length);
	return atof(buffer);
}

num_t num_FromDouble(double d) {
	char buffer[20] = { 0 };
	num_t ret;

	sprintf(buffer, "%.17g", d);
	
	ret.length = strlen(buffer);
	ret.number = malloc(ret.length);

	memcpy(ret.number, buffer, ret.length);

	return ret;
}

void num_Cleanup(num_t num) {
	free(num.number);
}