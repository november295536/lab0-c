#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    while (!list_empty(l)) {
        element_t *node = list_first_entry(l, element_t, list);
        list_del(&node->list);
        q_release_element(node);
    }

    free(l);
}

/*
 * Attempt to create a new element_t.
 * Argument s points to the string to be stored.
 * Return pointer to new element_t if successful.
 * Return null if failed.
 */
static inline element_t *element_create(char *s)
{
    element_t *node = malloc(sizeof(element_t));
    char *copy = strdup(s);
    if (!node || !copy) {
        free(node);
        free(copy);
        return NULL;
    }
    node->value = copy;
    return node;
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *node = element_create(s);
    if (!node)
        return false;
    list_add(&node->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *node = element_create(s);
    if (!node)
        return false;
    list_add_tail(&node->list, head);
    return true;
}

static inline void cpynstr(char *des, const char *source, size_t bufsize)
{
    if (des) {
        strncpy(des, source, bufsize - 1);
        des[bufsize - 1] = 0;
    }
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *node = list_first_entry(head, element_t, list);
    list_del(&node->list);
    cpynstr(sp, node->value, bufsize);
    return node;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *node = list_last_entry(head, element_t, list);
    list_del(&node->list);
    cpynstr(sp, node->value, bufsize);
    return node;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int len = 0;
    struct list_head *p;

    list_for_each (p, head)
        len++;
    return len;
}

static inline struct list_head *find_mid(struct list_head *head)
{
    struct list_head *mid = head, *p;
    int i = 1;

    for (p = head->next; p && p != head; p = p->next) {
        if (i % 2) {
            mid = mid->next;
        }
        i++;
    }
    return mid;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *mid = find_mid(head);

    list_del(mid);
    q_release_element(list_entry(mid, element_t, list));
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;

    element_t *pos, *tmp;
    list_for_each_entry (pos, head, list) {
        if (pos->list.next == head)
            break;
        bool delete_cur = false;
        for (tmp = list_entry(pos->list.next, element_t, list);
             strcmp(pos->value, tmp->value) == 0;
             tmp = list_entry(pos->list.next, element_t, list)) {
            list_del(&tmp->list);
            q_release_element(tmp);
            delete_cur = true;
            // cppcheck-suppress knownConditionTrueFalse
            if (pos->list.next == head)
                break;
        }

        if (delete_cur) {
            tmp = list_entry(pos->list.prev, element_t, list);
            list_del(&pos->list);
            q_release_element(pos);
            pos = tmp;
        }
    }
    return true;
}

/*
 * Swap two list_head.
 */
static inline void list_swap(struct list_head *head1, struct list_head *head2)
{
    if (head1->next == head2) {
        list_del(head2);
        list_add(head2, head1->prev);
    } else if (head2->next == head1) {
        list_del(head1);
        list_add(head1, head2->prev);
    } else {
        struct list_head *head1_prev = head1->prev, *head2_prev = head2->prev;
        list_del(head1);
        list_del(head2);
        list_add(head2, head1_prev);
        list_add(head1, head2_prev);
    }
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    int i = 0;
    struct list_head *cur;
    list_for_each (cur, head) {
        if (i % 2) {
            list_swap(cur->prev, cur);
            cur = cur->next;
        }
        i++;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *front = head, *back = head, *tmp;
    while (true) {
        front = front->next;
        back = back->prev;
        if (front == back)
            break;
        list_swap(front, back);
        tmp = front;
        front = back;
        back = tmp;
        if (front->next == back)
            break;
    }
}

static struct list_head *merge_two_lists(struct list_head *head1,
                                         struct list_head *head2)
{
    struct list_head *head = NULL, **node, **ptr = &head;
    for (node = NULL; head1 && head2; *node = (*node)->next) {
        element_t *e1 = list_entry(head1, element_t, list);
        element_t *e2 = list_entry(head2, element_t, list);
        node = (strcmp(e1->value, e2->value) < 0) ? &head1 : &head2;
        *ptr = *node;
        ptr = &(*ptr)->next;
    }
    *ptr = (struct list_head *) ((u_int64_t) head1 | (u_int64_t) head2);

    return head;
}

static struct list_head *merge_sort(struct list_head *head)
{
    if (!head->next)
        return head;
    struct list_head *mid = find_mid(head);
    mid->prev->next = NULL;

    head = merge_sort(head);
    mid = merge_sort(mid);

    return merge_two_lists(head, mid);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *first_node = head->next;
    head->prev->next = NULL;
    struct list_head *sorted_list = merge_sort(first_node);

    head->next = sorted_list;
    struct list_head *cur = head;
    while (cur->next) {
        cur->next->prev = cur;
        cur = cur->next;
    }
    cur->next = head;
    head->prev = cur;
}
