#include "include/stack.h"

#include <stdio.h>
#include <stdlib.h>

#include "include/ast.h"

struct stack *stack_init()
{
    struct stack *s = malloc(sizeof(struct stack));
    struct list *head = malloc(sizeof(struct list));
    head->next = NULL;
    head->data = NULL;

    s->head = head;
    s->size = 0;

    return s;
}

void destroy_stack(struct stack *s)
{
    struct list *curr = s->head;
    while (curr)
    {
        struct list *tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    free(s);
}

void stack_push(struct stack **s, void *e)
{
    struct list *new_head = malloc(sizeof(struct list));
    new_head->data = e;
    new_head->next = (*s)->head;
    (*s)->head = new_head;
    (*s)->size++;
}

void *stack_pop(struct stack **s)
{
    if (!(*s)->head) // if empty stack
        return NULL;

    struct list *head = (*s)->head;
    void *pop = head->data;

    (*s)->head = (*s)->head->next;
    (*s)->size--;

    free(head);
    return pop;
}

void *stack_peek(struct stack *s)
{
    if (!s->head)
        return NULL;

    return s->head->data;
}