#include "stack.h"

#include <stdlib.h>

void stack_Create(Stack *s) {
	s->top = 0;
	s->_max = STACK_START;
	s->items = malloc(STACK_START * sizeof(void*));
}

void stack_Cleanup(Stack *s) {
	free(s->items);
}

void stack_Push(Stack *s, void *item) {
	if (s->top >= s->_max) {
		s->_max *= 2;
		s->items = realloc(s->items, s->_max * sizeof(void*));
	}

	s->items[s->top++] = item;
}

void *stack_Pop(Stack *s) {
	if (s->top <= 0) {
		s->top = 0;
		return NULL;
	}

	return s->items[--s->top];
}

void *stack_Peek(Stack *s) {
	if (s->top <= 0) {
		s->top = 0;
		return NULL;
	}

	return s->items[s->top - 1];
}

void stack_Clear(Stack *s) {
	s->top = 0;
}