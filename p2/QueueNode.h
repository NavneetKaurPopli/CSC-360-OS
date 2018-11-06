/**
 * Zhe Chen
 * V00819544
 * CSC360 Fall 2018
 * P2: Airline Check-in System
 * FIFO queue node for customer
 * Header file
 */

#ifndef P2_QUEUENODE_H
#define P2_QUEUENODE_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "Customer.h"

/**
 * define a linked list style FIFO queue for customer queue
 * each node contains a customer
 */
typedef struct QueueNode {
    struct Customer customer;
    struct QueueNode *next;
} QueueNode;

QueueNode *create_queue(struct Customer c);

void insert(QueueNode **queue, struct Customer c);

struct Customer delete(QueueNode **queue);

int get_size(QueueNode *queue);

void print_queue(QueueNode *queue);

#endif //P2_QUEUENODE_H
