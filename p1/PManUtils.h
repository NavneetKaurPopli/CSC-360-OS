/**
 * Zhe Chen
 * CSC360 Fall 2018
 * P1: A Process Manager (PMan)
 * Utilities function for PMan
 * Header file
 */
#ifndef ASSIGNMENT1_PMAN_PMANUTILS_H
#define ASSIGNMENT1_PMAN_PMANUTILS_H
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "ProcNode.h"

void bg(ProcNode **proc_list, char **argv);

void bglist(ProcNode **proc_list);

void bgstop(pid_t pid);

void bgstart(pid_t pid);

void bgkill(pid_t pid);

int startsWith(const char *pre, const char *str);

void pstat(pid_t pid);

void proc_listener(ProcNode **proc_list);

void proc_clean(ProcNode **proc_list);

#endif //ASSIGNMENT1_PMAN_PMANUTILS_H
