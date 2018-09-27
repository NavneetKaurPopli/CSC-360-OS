/**
 * Zhe Chen
 * V00819544
 * CSC360 Fall 2018
 * P1: A Process Manager (PMan)
 * Utilities function for PMan
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

/**
 * run a program in the background with fork()
 * @param proc_list
 * @param args
 * @param len
 */
void bg(ProcNode **proc_list, char **args, int len) {
    ProcNode *pProc_list = *proc_list; // pointer to proc_list pointer
    pid_t pid = fork(); // create child process

    if (0 == pid) {
        // it is in child process, execute the command
        char *tmp[len+1];
        int i;

        // reformat the argv in order for the execvp to run the program with parameters
        for (i=0; i<len; i++) {
            tmp[i] = malloc(strlen(args[i]) + 1);
            strcpy(tmp[i], args[i]);
        }
        tmp[i] = NULL; // last one should be NULL in order for execvp to run

        if (-1 == execvp(tmp[0], tmp)) {
            perror("unexpected error: failed creating background process.\n");
            exit(-1);
        }
    }
    else if (pid > 0) {
        // if path name starts with "./" then (pwd)/exe
        char *pname = (args[0][0]=='.'&&args[0][1]=='/')?strcat(getcwd(0,0),&args[0][1]):args[0];
        add_to_list(&pProc_list, pid, pname);
        printf("creating background process with pid: %d...\n", (int)pid);
        sleep(1);
    }
    else {
        // failed creating the child
        printf("unexpected error: failed creating child container.\n");
    }
}

/**
 * print out the processes running the background
 * @param proc_list
 */
void bglist(ProcNode **proc_list) {
    ProcNode *pProc_list = *proc_list;
    int count = 0;
    ProcNode *curr = pProc_list;

    while (NULL != curr) {
        if (1 == curr->proc_state) {
            printf("%d: %s\n", curr->proc_id, curr->proc_name);
            count++;
        }
        curr = curr->next;
    }

    printf("Total background jobs: %d\n", count);
}

/**
 * pause the program with pid
 * @param pid
 */
void bgstop(pid_t pid) {
    if (0 <= kill(pid, SIGSTOP)) {
        printf("stopping process with pid: %d...\n", (int)pid);
        sleep(1); // trying to make things to look more real
    }
    else {
        printf("bgstop(): process with pid: %d does not exist.\n", (int)pid);
    }
}

/**
 * resume the program with pid
 * @param pid
 */
void bgstart(pid_t pid) {
    if (0 <= kill(pid, SIGCONT)) {
        printf("starting process with pid: %d...\n", (int)pid);
        sleep(1);
    }
    else {
        printf("bgstart(): process with pid: %d does not exist.\n", (int)pid);
    }
}

/**
 * terminate the program with pid
 * @param pid
 */
void bgkill(pid_t pid) {
    if (0 <= kill(pid, SIGTERM)) {
        printf("killing process with pid: %d...\n", (int)pid);
        sleep(1);
    }
    else {
        printf("bgkill() failed.\n");
    }
}

/**
 * easy tool found online for checking if pre is a prefix of str
 * https://stackoverflow.com/questions/4770985/how-to-check-if-a-string-starts-with-another-string-in-c
 * @param pre
 * @param str
 * @return
 */
int startsWith(const char *pre, const char *str) {
    if (0 == strncmp(pre, str, strlen(pre)))
        return 1; // true
    else
        return 0; // false
}

/**
 * list information related to process with pid
 * @param pid
 */
void pstat(pid_t pid) {
    FILE *fp;
    char path_name[100];
    if (0 > sprintf(path_name, "/proc/%d", (int)pid)) {
        printf("unexpected error: cannot create path name.\n");
        return;
    }

    if (NULL == (fp=fopen(strcat(path_name,"/stat"), "r"))) {
        printf("Error: Process %d does not exist.\n", (int)pid);
        return;
    }

    // in /proc/[pid]/stat we want item 2(comm) 3(state) 14(utime) 15(stime) 24(rss)
    // http://man7.org/linux/man-pages/man5/proc.5.html
    int i = 0;
    char stat_file[4000];
    fgets(stat_file, 4000, fp);

    char *tmp;
    tmp = strtok(stat_file, " ");
    char *stat_field[100];

    while (NULL != tmp) {
        stat_field[i] = malloc(strlen(tmp) + 1);
        strcpy(stat_field[i], tmp);
        i++;
        tmp = strtok(NULL, " ");
    }

    printf("comm: \t\t%s\n", stat_field[1]); // item#-1
    printf("state: \t\t%s\n", stat_field[2]);
    printf("utime: \t\t%s\n", stat_field[13]);
    printf("stime: \t\t%s\n", stat_field[14]);
    printf("rss: \t\t%s\n", stat_field[23]);

    fclose(fp);

    // /status
    if (NULL == (fp=fopen(strcat(path_name,"us"), "r"))) {
        // printf("%s\n", path_name);
        printf("Error: Process %d does not exist.\n", (int)pid);
        return;
    }

    // in /proc/[pid]/status we want voluntary_ctxt_switches nonvoluntary_ctxt_switches
    char line[256];

    while(fgets(line, sizeof(line), fp)) {
        if (1 == startsWith("voluntary_ctxt_switches:", line))
            printf("%s",line);
        if (1 == startsWith("nonvoluntary_ctxt_switches:", line))
            printf("%s",line);
    }
}

/**
 * indicate to the user when background jobs have terminated/paused/resumed
 * @param proc_list
 */
void proc_listener(ProcNode **proc_list) {
    ProcNode *pProc_list = *proc_list;
    pid_t pid;
    int p_state;
    for(;;) {
        /*
         * -1 means wait for any process
         * If 0, then waits until the specified child return
         * WNOHANG - return immediately if no child has exited
         * WUNTRACED - also returns if a child has stopped
         * WCONTINUED - also returns if a stopped child has been resumed by SIGCONT
         */
        pid = waitpid(-1, &p_state, WNOHANG | WUNTRACED | WCONTINUED);

        if (0 < pid) {
            if (WIFEXITED(p_state)) {
                delete_from_list(&pProc_list, pid);
                printf("process with pid: %d has exited.\n", (int)pid);
            }
            else if (WIFSIGNALED(p_state)) {
                delete_from_list(&pProc_list, pid);
                printf("process with pid: %d has been terminated.\n", (int)pid);
            }
            else if (WIFSTOPPED(p_state)) {
                set_state(&pProc_list, pid, 0);
                printf("process with pid: %d has been paused.\n", (int)pid);
            }
            else if (WIFCONTINUED(p_state)) {
                set_state(&pProc_list, pid, 1);
                printf("process with pid: %d has been resumed.\n", (int)pid);
            }
            else {
                // do nothing
            }
        }
        else {
            // when no process is terminated/paused/resumed, enters here and continue PMan
            break;
        }
    }
}

/**
 * clean function that kills all the zombie process when user quits the PMan
 * @param proc_list
 */
void proc_clean(ProcNode **proc_list) {
    ProcNode *pProc_list = *proc_list;
    ProcNode *curr = pProc_list;

    while(NULL != curr) {
        bgkill(curr->proc_id);
        curr = curr->next;
    }
}

#endif //ASSIGNMENT1_PMAN_PMANUTILS_H
