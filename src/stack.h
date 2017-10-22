#ifndef _STACK_H_
#define _STACK_H_

//the default stack size
#define STACK_START 10

typedef struct {
    unsigned int _max;
    unsigned int top;
    void **items;
} Stack;

void stack_Create(Stack *s);
void stack_Cleanup(Stack *s);

void stack_Push(Stack *s, void *item);
void *stack_Pop(Stack *s);
void *stack_Peek(Stack *s);

void stack_Clear(Stack *s);

#endif