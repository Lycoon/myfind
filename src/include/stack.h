#ifndef STACK_H
#define STACK_H

#include <stddef.h>

struct list
{
    void *data;
    struct list *next;
};

struct stack
{
    struct list *head;
    size_t size;
};

struct stack *stack_init();
void destroy_stack(struct stack *s);
void stack_push(struct stack **s, void *e);
void *stack_pop(struct stack **s);
void *stack_peek(struct stack *s);

#endif /* !STACK_H */
