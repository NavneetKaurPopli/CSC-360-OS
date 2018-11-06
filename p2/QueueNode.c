/**
 * Zhe Chen
 * V00819544
 * CSC360 Fall 2018
 * P2: Airline Check-in System
 * FIFO queue node for customer
 * Source file
 */

#include "QueueNode.h"

/**
 * create a new queue with initial customer c
 * @param c
 * @return
 */
QueueNode *create_queue(struct Customer c) {
    QueueNode *queue = malloc(sizeof(QueueNode));
    queue->customer = c;
    queue->next = NULL;

    return queue;
}

/**
 * add customer to the tail of the queue
 * @param queue
 * @param c
 */
void insert(QueueNode **queue, struct Customer c) {
    QueueNode *pQueue = *queue;

    while (NULL != pQueue->next) {
        pQueue = pQueue->next;
    }

    QueueNode *temp = create_queue(c);

    // add to the tail of the queue
    pQueue->next = temp;
}

/**
 * pop the head customer from the queue
 * @param queue
 * @return
 */
struct Customer delete(QueueNode **queue) {
    QueueNode *pQueue = *queue;

    if (0 == get_size(pQueue)) {
        printf("error: trying to remove from an empty queue.\n");
        exit(0);
    }

    struct Customer cus = pQueue->customer;
    QueueNode *node = pQueue;
    *queue = pQueue->next;
    free(node);

    return cus;
}

/**
 * return the length of the queue
 * @param queue
 * @return
 */
int get_size(QueueNode *queue) {
    QueueNode *curr = queue;
    int count = 0;

    while (NULL != curr) {
        if (0 != curr->customer.customer_id){
            count++;
        }
        curr = curr->next;
    }

    return count;
}

void print_queue(QueueNode *queue) {
    QueueNode *curr = queue;

    while (NULL != curr) {
        printf("id(%d)\n", curr->customer.customer_id);
        curr = curr->next;
    }
    printf("queue size: %d\n", get_size(queue));
}
