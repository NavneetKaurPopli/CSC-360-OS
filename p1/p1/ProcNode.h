/**
 * Zhe Chen
 * V00819544
 * CSC360 Fall 2018
 * P1: A Process Manager (PMan)
 * Linked list data structured process node to hold the background programs
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

/**
 * create a process with pid and pname and state set to 1
 * @param pid
 * @param pname
 * @return
 */
ProcNode *create_proc(pid_t pid, char *pname) {
    ProcNode *process = malloc(sizeof(ProcNode));
    process->proc_id = pid;
    process->proc_name = pname;
    process->proc_state = 1;
    process->next = NULL;
    return process;
}

/**
 * add a process to the head of list
 * @param head
 * @param pid
 * @param pname
 */
void add_to_list(ProcNode **head, pid_t pid, char *pname) {
    ProcNode *pHead = *head; // pointer to that pointer

    if (NULL == pHead){
        pHead = create_proc(pid, pname);
        return;
    }

    while (NULL != pHead->next) {
        pHead = pHead->next;
    }

    ProcNode *temp = create_proc(pid, pname);
    pHead->next = temp;
}

/**
 * delete the process with pid from the list
 * @param head
 * @param pid
 */
void delete_from_list(ProcNode **head, pid_t pid) {
    ProcNode *curr = *head;
    ProcNode *prev = NULL;

    if(NULL == curr) {
        printf("error: cannot delete process that doesn't exist.");
        return;
    }

    while (NULL != curr) {
        if (curr->proc_id == pid) {
            if (NULL == prev) {
                curr = curr->next;
                return;
            }
            else {
                prev->next = curr->next;
                free(curr);
            }
        }
        prev = curr;
        curr = curr->next;
    }
}

/**
 * get the process with pid from the list, return NULL if not found
 * @param head
 * @param pid
 * @return
 */
ProcNode *get_proc(ProcNode **head, pid_t pid) {
    ProcNode *pHead = *head;
    ProcNode *curr = pHead;
    while (NULL != curr) {
        if (curr->proc_id == pid)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

/**
 * change the process state if 1 then 0 else 1
 * @param head
 * @param pid
 */
void set_state(ProcNode **head, pid_t pid, int state) {
    ProcNode *phead = *head;
    ProcNode *curr = get_proc(&phead, pid);
    curr->proc_state = state;
}

#endif //ASSIGNMENT1_PMAN_PROCNODE_H
