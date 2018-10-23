/**
 * Zhe Chen
 * CSC360 Fall 2018
 * P1: A Process Manager (PMan)
 * Linked list data structured process node to hold the background programs
 * Header file
 */
#ifndef ASSIGNMENT1_PMAN_PROCNODE_H
#define ASSIGNMENT1_PMAN_PROCNODE_H
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/**
 * define a linked list for process node
 * proc_id is the unique key
 */
typedef struct ProcNode {
    char *proc_name;
    pid_t proc_id;
    int proc_state; // process state where 1: running, 0: stopped
    struct ProcNode *next;
} ProcNode;


ProcNode *create_proc(pid_t pid, char *pname);

void add_to_list(ProcNode **head, pid_t pid, char *pname);

void delete_from_list(ProcNode **head, pid_t pid);

ProcNode *get_proc(ProcNode **head, pid_t pid);

void set_state(ProcNode **head, pid_t pid, int state);

#endif //ASSIGNMENT1_PMAN_PROCNODE_H
