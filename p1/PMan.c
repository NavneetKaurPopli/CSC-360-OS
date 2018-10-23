/**
 * Zhe Chen
 * CSC360 Fall 2018
 * P1: A Process Manager (PMan)
 * Main function file
 */
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
#include "PManUtils.h"

/**
 * main function
 * @return
 */
int main() {
    char *input;
    char *cmd;
    ProcNode *proc_list = malloc(sizeof(ProcNode));

    // error handler for malloc
    if (NULL == proc_list){
        printf("unexpected error: malloc() failed.");
        exit(-1);
    }

    for(;;) {
        input = readline("PMan: > ");
        if (0 == strcmp(input, ""))
            continue; // no input

        cmd = strtok(input, " "); // get command

        // exit the PMan
        if (0 == strcmp(cmd, "quit") || 0 == strcmp(cmd, "q"))
            break;

        if (0 == strcmp(cmd, "bg")) {
            char *tmp = strtok(NULL, " ");
            int i = 0;
            char *args[10];
            while (NULL != tmp && i < 10) {
                args[i] = malloc(strlen(tmp) + 1);
                if (NULL == args[i]) {
                    printf("unexpected error: malloc() failed.");
                    exit(-1);
                }
                strcpy(args[i], tmp);
                i++;
                tmp = strtok(NULL, " ");
            }
            args[i] = NULL;

            bg(&proc_list, args);

            // free memory after malloc()
            for (i=0; i<10; i++) {
                if (NULL == args[i])
                    break;
                free(args[i]);
            }
        }
        else if (0 == strcmp(cmd, "bglist")) {
            bglist(&proc_list);
        }
        else if (0 == strcmp(cmd, "bgkill")) {
            char *pidstr = strtok(NULL, " ");
            if (NULL != pidstr) {
                pid_t pid = atoi(pidstr);
                if (0 == pid) {
                    printf("Usage: bgkill <pid>\n");
                }
                else {
                    bgkill(pid);
                }
            }
            else {
                printf("Usage: bgkill <pid>\n");
            }
        }
        else if (0 == strcmp(cmd, "bgstop")) {
            char *pidstr = strtok(NULL, " ");
            if (NULL != pidstr) {
                pid_t pid = atoi(pidstr);
                if (0 == pid) {
                    printf("Usage: bgstop <pid>\n");
                }
                else {
                    bgstop(pid);
                }
            }
            else {
                printf("Usage: bgstop <pid>\n");
            }
        }
        else if (0 == strcmp(cmd, "bgstart")) {
            char *pidstr = strtok(NULL, " ");
            if (NULL != pidstr) {
                pid_t pid = atoi(pidstr);
                if (0 == pid) {
                    printf("Usage: bgstart <pid>\n");
                }
                else {
                    bgstart(pid);
                }
            }
            else {
                printf("Usage: bgstart <pid>\n");
            }
        }
        else if (0 == strcmp(cmd, "pstat")) {
            char *pidstr = strtok(NULL, " ");
            if (NULL != pidstr) {
                pid_t pid = atoi(pidstr);
                if (0 == pid) {
                    printf("Usage: pstat <pid>\n");
                }
                else {
                    pstat(pid);
                }
            }
            else {
                printf("Usage: pstat <pid>\n");
            }
        }
        else {
            // error message for unknown command
            printf("%s: command not found\n", cmd);
            continue;
        }
        // listener for the events of any process
        proc_listener(&proc_list);
    }

    // clean up zombie processes
    proc_clean(&proc_list);
    free(proc_list);
    return 0;
}