/**
 * Zhe Chen
 * V00819544
 * CSC360 Fall 2018
 * P2: Airline Check-in System
 * Main function file
 *
 * An airline check-in system simulation program which simulates four clerks with two
 * lines one for business class and one for economy class. clerks will serve customers
 * in business class prior to the economy class. Customers information are store in
 * customers.txt file and this program will read the file and starts the simulation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include "QueueNode.h"

#define MAX_CHAR_SIZE 255 // char encoding maximum limit
#define NQUEUES 3 // for some reason 0th element does not work in my Windows 10 CLion IDE, so I increased
#define NCLERKS 5 // size by one for queue and clerk and let the code to skip 0th element and it worked...

/* global variables */
QueueNode *class_type_queue[NQUEUES]; // 1 for economy(0) and 2 for business(1)

// mutexes for two queues and four clerks
pthread_mutex_t queue_mutex[NQUEUES];
pthread_mutex_t clerk_mutex[NCLERKS];
// conditional variables for two queues and four clerks
pthread_cond_t queue_convar[NQUEUES];
pthread_cond_t clerk_convar[NCLERKS];
// mutex for writing average time of each class
// it is possible to have an array to hold these two, but I am too lazy to refactor them
pthread_mutex_t business_time_mutex;
pthread_mutex_t economy_time_mutex;

// initial timestamp when program starts
double init_time;

// flag for customer to identify which clerk is sending signal and which queue it is signalling
int clerk_signal;
int signal_queue_id; 
// signal for each clerk thread to exit
// calculate average waiting time for all business-class customers
double avg_business_wtime;
// calculate average waiting time for all economy-class customers
double avg_economy_wtime;

/* clerk thread */
void *clerk(void *);

/* customer thread */
void *customer(void *);

/* get timestamp whenever it is called */
double get_timestamp();

/* mutex and conditional variable initialization */
void mutex_convar_init();

/**
 * main function.
 * read file, create clerk threads, create customer threads, calculate
 * average waiting time for both classes.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    FILE *fp;
    int customer_num;
    char inputs[MAX_CHAR_SIZE];

    if (NULL == (fp=fopen(argv[1], "r"))) {
        printf("File not found...\n");
        exit(0);
    }

    if (NULL == fgets(inputs, sizeof(inputs), fp)) {
        printf("Illegal content...\n");
        exit(0);
    }

    customer_num = atoi(inputs);
    struct Customer customer_queue[customer_num];

    // feed customer info into the array
    int i;
    int customer_id;
    int class_type;
    int arrival_time;
    int service_time;
    for (i=0; i<customer_num; i++) {
        // do error checking for each element
        if (NULL == fgets(inputs, sizeof(inputs), fp)) {
            printf("error: missing customer information.\n");
            return -1;
        }
        if (0 > (customer_id = atoi(strtok(inputs, ":")))) {
            printf("error: illegal customer id.\n");
            return -1;
        }
        if (0 > (class_type = atoi(strtok(NULL, ",")))) {
            printf("error: illegal class type.\n");
            return -1;
        }
        if (0 > (arrival_time = atoi(strtok(NULL, ",")))) {
            printf("error: illegal arrival time.\n");
            return -1;
        }
        if (0 > (service_time = atoi(strtok(NULL, ",")))) {
            printf("error: illegal service time.\n");
            return -1;
        }

        customer_queue[i].customer_id = customer_id;
        customer_queue[i].class_type = class_type;
        customer_queue[i].arrival_time = arrival_time;
        customer_queue[i].service_time = service_time;
    }

    // initialize mutexes and conditional variables before use
    mutex_convar_init();

    // set default signal to 0 means no clerks is sending signal
    // 1 to 4 is the id of each clerk
    clerk_signal = 0;
    signal_queue_id = 0;

    // set initial average waiting time to 0.0
    avg_business_wtime = 0.0;
    avg_economy_wtime = 0.0;

    // number of customers in each class
    int business_num = 0;
    int economy_num = 0;
    struct timeval init_tv;
    gettimeofday(&init_tv, NULL);
    init_time = (init_tv.tv_sec + (double) init_tv.tv_usec / 1000000);

    // create clerks
    pthread_t clerk_thread[NCLERKS];
    int clerk_id[NCLERKS];
    for (i=1; i<NCLERKS; i++) {
        clerk_id[i] = i;
        // create customer thread and pass its customer info in the thread function
        if (0 != pthread_create(&clerk_thread[i], NULL, clerk, (void *)&clerk_id[i])) {
            printf("error: failed creating clerk thread id: %d.\n", clerk_id[i]);
            return -1;
        }
    }

    // create customer threads all at once
    pthread_t customer_thread[customer_num];
    for (i=0; i<customer_num; i++) {
        // create customer thread and pass its customer info in the thread function
        if (0 != pthread_create(&customer_thread[i], NULL, customer, (void *)&customer_queue[i])) {
            printf("error: failed creating customer thread id: %d.\n", customer_queue[i].customer_id);
            return -1;
        }

        // calculating size of each class while doing error checking for customer class type
        if (0 == customer_queue[i].class_type) {
            economy_num++;
        }
        else if (1 == customer_queue[i].class_type) {
            business_num++;
        }
        else {
            printf("error: wrong class type.\n");
            return -1;
        }
    }

    // wait for all customer threads to finish
    // not sure if this is needed since the attr for customer is not PTHREAD_CREATE_JOINABLE
    for (i=0; i<customer_num; i++) {
        if (0 != pthread_join(customer_thread[i], NULL)) {
            printf("error: failed joining customer thread id: %d.\n", customer_queue[i].customer_id);
            return -1;
        }
    }

    // calculate and print to console the average waiting times for this simulation
    printf("The average waiting time for all customers in the system is: %.2f seconds. \n", (avg_economy_wtime+avg_business_wtime)/customer_num);
    printf("The average waiting time for all business-class customers is: %.2f seconds. \n", avg_business_wtime/business_num);
    printf("The average waiting time for all economy-class customers is: %.2f seconds. \n", avg_economy_wtime/economy_num);

    return 0;
}

/**
 * thread that behaves as customer, param contains the customer info
 * @param param
 * @return
 */
void *customer(void *param) {
    struct Customer customer = *((struct Customer *)param);
    int customer_queue = customer.class_type+1;

    // use usleep() to simulate customer arrival time
    // example arrival time: 6 -> 0.6 secs -> (0.6 * 1,000,000) usecs -> usleep(6 * 100,000);
    if (-1 == usleep(customer.arrival_time * 100000)) {
        printf("error: usleep() failed.\n");
        exit(-1);
    }

    printf("A customer arrives: customer ID %2d. \n", customer.customer_id);

    // lock the queue
    if (0 != pthread_mutex_lock(&queue_mutex[customer_queue])) {
        printf("error: pthread_mutex_lock queue_mutex failed.\n");
        exit(-1);
    }

    // when a clerk signals the queue but the queue is lokced by other (customer or clerk)thread
    // so the customer who is waiting on the signal will not be able to wake up since it failed to lock the mutex
    // in order to avoid this deadlock, I wrote this ugly solution to help clerk to signal the queue until 
    // that customer wakes up
    if (0 != signal_queue_id) {
        // unlock ths queue first
        if (0 != pthread_mutex_unlock(&queue_mutex[customer_queue])) {
            printf("error: pthread_mutex_unlock queue_mutex failed.\n");
            exit(-1);
        }

        // until the customer wakes up, keeps signal
        while (0 != signal_queue_id) {
            // help other clerk to signal the queue
            if(0 != pthread_cond_signal(&queue_convar[signal_queue_id])){
                printf("error: pthread_cond_signal failed to signal queue id %d.\n", signal_queue_id);
                exit(-1);
            }
        }

        // the customer has woke up, you can now lock the queue or wait in a line
        if (0 != pthread_mutex_lock(&queue_mutex[customer_queue])) {
            printf("error: pthread_mutex_lock queue_mutex failed.\n");
            exit(-1);
        }
    }

    /* critical section for customer to enter a queue according to its class type */
    int queue_size;

    if (NULL == class_type_queue[customer_queue]) {
        class_type_queue[customer_queue] = create_queue(customer);
    }
    else {
        insert(&class_type_queue[customer_queue], customer);
    }

    // start waiting time
    double start_time = get_timestamp();

    queue_size = get_size(class_type_queue[customer_queue]);

    int ctype = customer.class_type;
    printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", ctype, queue_size);

    // wait for this customer's turn
    // when queue is signalled, wake up, lock the mutex, check if is the head of that queue
    do {
        if (0 != pthread_cond_wait(&queue_convar[customer_queue], &queue_mutex[customer_queue])) {
            printf("error: pthread_cond_wait failed to wait for queue id %d.\n", customer_queue);
            exit(-1);
        }
    }while (customer.customer_id != class_type_queue[customer_queue]->customer.customer_id && 0 != clerk_signal);
    signal_queue_id = 0;

    // now is the turn, remove the customer from the queue
    struct Customer removed_customer = delete(&class_type_queue[customer_queue]);
    // error handling, ideally would never happen
    if (removed_customer.customer_id != customer.customer_id) {
        printf("error: removed wrong customer id %d.\n", removed_customer.customer_id);
        exit(-1);
    }

    // and then unlock the queue, let other customer or clerk use it
    if (0 != pthread_mutex_unlock(&queue_mutex[customer_queue])) {
        printf("error: pthread_mutex_unlock queue_mutex failed.\n");
        exit(-1);
    }

    /* critical section to read the clerk signal */
    int serve_id = clerk_signal;
    if (0 == serve_id) {
        printf("error: customer id %d is trying to read clerk_signal when no clerk is sending it.\n", customer.customer_id);
        exit(-1);
    }
    // accepts clerk's signal and set it back to default
    clerk_signal = 0;

    // calculate waiting time
    if (0 == customer.class_type) {
        pthread_mutex_lock(&economy_time_mutex);
        /* critical section to write to the wait time */
        avg_economy_wtime += get_timestamp() - start_time;
        pthread_mutex_unlock(&economy_time_mutex);
    }else {
        pthread_mutex_lock(&business_time_mutex);
        /* critical section to write to the wait time */
        avg_business_wtime += get_timestamp() - start_time;
        pthread_mutex_unlock(&business_time_mutex);
    }

    start_time = get_timestamp();

    printf("A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n", start_time, customer.customer_id, serve_id);

    // use usleep() to simulate customer service time
    if (-1 == usleep(customer.service_time * 100000)) {
        printf("error: usleep() failed.\n");
        exit(-1);
    }

    double end_time = get_timestamp();
    printf("A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", end_time, customer.customer_id, serve_id);

    // lock clerk
    if (0 != pthread_mutex_lock(&clerk_mutex[serve_id])) {
        printf("error: pthread_mutex_lock failed to lock clerk id %d.\n", serve_id);
        exit(-1);
    }
    // signal the clerk who is serving that it is finished
    if (0 != pthread_cond_signal(&clerk_convar[serve_id])) {
        printf("error: pthread_cond_signal failed to signal clerk id %d.\n", serve_id);
        exit(-1);
    }
    // lock clerk
    if (0 != pthread_mutex_unlock(&clerk_mutex[serve_id])) {
        printf("error: pthread_mutex_unlock failed to unlock clerk id %d.\n", serve_id);
        exit(-1);
    }

    pthread_exit(0);
    return NULL;
}

void *clerk(void *param) {
    int clerk_id = *((int *)param);
    int queue_to_signal;
    int i;

    while(1) {
        // lock queues
        for (i=1; i<NQUEUES; i++) {
            if (0 != pthread_mutex_lock(&queue_mutex[i])) {
                printf("error: pthread_mutex_lock queue_mutex failed.\n");
                exit(-1);
            }
        }

        // solution to avoid deadlock
        if (0 != signal_queue_id) {
            for (i=1; i<NQUEUES; i++) {
                if (0 != pthread_mutex_unlock(&queue_mutex[i])) {
                    printf("error: pthread_mutex_unlock queue_mutex failed.\n");
                    exit(-1);
                }
            }
            
            while (0 != signal_queue_id) {
                // help other clerk to signal the queue
                if(0 != pthread_cond_signal(&queue_convar[signal_queue_id])){
                    printf("error: pthread_cond_signal failed to signal queue id %d.\n", signal_queue_id);
                    exit(-1);
                }
            }

            for (i=1; i<NQUEUES; i++) {
                if (0 != pthread_mutex_lock(&queue_mutex[i])) {
                    printf("error: pthread_mutex_lock queue_mutex failed.\n");
                    exit(-1);
                }
            }
        }

        /* critical section to find which queue to signal */
        queue_to_signal = -1;

        // check size of business class first
        for (i=NQUEUES-1; i>0; i--) {
            if (0 != get_size(class_type_queue[i])) {
                queue_to_signal = i;
                break;
            }
        }

        // unlock unwanted queue
        for (i=1; i<NQUEUES; i++) {
            if (i != queue_to_signal) {
                if (0 != pthread_mutex_unlock(&queue_mutex[i])) {
                    printf("error: pthread_mutex_unlock queue_mutex failed.\n");
                    exit(-1);
                }
            }
        }


        // if both queues are empty, no customer arrive
        // the clerk will back to the top and do the size checking process
        if (-1 == queue_to_signal) {
            continue;
        }

        // ask for permission to write to clerk_signal
        // if it is not 0, means other clerk is sending signal
        // will have to wait for other to finish sending
        while(0 != clerk_signal) {}

        // let customer know  which clerk is sending signal to the queue
        clerk_signal = clerk_id;
        signal_queue_id = queue_to_signal;

        // unlock the queue that was signalled
        if (0 != pthread_mutex_unlock(&queue_mutex[queue_to_signal])) {
            printf("error: pthread_mutex_unlock queue_mutex failed.\n");
            exit(-1);
        }

        // signal the queue
        if(0 != pthread_cond_signal(&queue_convar[queue_to_signal])){
            printf("error: pthread_cond_signal failed to signal queue id %d.\n", queue_to_signal-1);
            exit(-1);
        }

        // lock clerk mutex
        if (0 != pthread_mutex_lock(&clerk_mutex[clerk_id])) {
            printf("error: pthread_mutex_lock clerk_mutex failed.\n");
            exit(-1);
        }

        /* critical section to wait for customer to signal clerk that it has finished service */
        if (0 != pthread_cond_wait(&clerk_convar[clerk_id], &clerk_mutex[clerk_id])) {
            printf("error: pthread_cond_wait clerk_convar failed.\n");
            exit(-1);
        }

        // unlock clerk mutex
        if (0 != pthread_mutex_unlock(&clerk_mutex[clerk_id])) {
            printf("error: pthread_mutex_unlock clerk_mutex failed.\n");
            exit(-1);
        }
    }

    pthread_exit(0);
    return NULL;
}

/**
 * a function to get current timestamp
 * @return
 */
double get_timestamp() {
  struct timeval tv;
  gettimeofday(&tv, NULL);

  return (tv.tv_sec + (double)tv.tv_usec / 1000000) - init_time;
}

/**
 * initialization of mutex and conditional variable that protects clerks and queues
 */
void mutex_convar_init() {
    int i;
    for (i=0; i<NCLERKS; i++) {
        if (NQUEUES > i) {
            /* init queue mutex */
            if (0 != pthread_mutex_init(&queue_mutex[i], NULL)) {
                printf("error: queue mutex %d(id) init failed\n", i);
                exit(-1);
            }
            /* init queue conditional variable */
            if (0 != pthread_cond_init(&queue_convar[i], NULL)) {
                printf("error: queue conditional variable %d(id) init failed\n", i);
                exit(-1);
            }
        }
        /* init clerk mutex */
        if (0 != pthread_mutex_init(&clerk_mutex[i], NULL)) {
            printf("error: clerk mutex %d(id) init failed\n", i);
            exit(-1);
        }
        /* init clerk conditional variable */
        if (0 != pthread_cond_init(&clerk_convar[i], NULL)) {
            printf("error: clerk conditional variable %d(id) init failed\n", i);
            exit(-1);
        }
    }

    /* init calculate time mutex for business class and economy */
    if (0 != pthread_mutex_init(&business_time_mutex, NULL)) {
        printf("error: caltime mutex %d(id) init failed\n", i);
        exit(-1);
    }
    if (0 != pthread_mutex_init(&economy_time_mutex, NULL)) {
        printf("error: caltime mutex %d(id) init failed\n", i);
        exit(-1);
    }
}
