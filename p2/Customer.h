/**
 * Zhe Chen
 * V00819544
 * CSC360 Fall 2018
 * P2: Airline Check-in System
 * Public structure of customer
 * Header file
 */

#ifndef P2_CUSTOMER_H
#define P2_CUSTOMER_H

/**
 * define a structure for customer
 *
 * customer_id: unique id for each customer
 * class_type: 0 economy class, 1 business class
 * arrival_time: when the customer will arrive
 * service_time: time required to serve the customer
 * start_time: when the customer starts check-in
 * clerk_id: clerk who served this customer
 */
struct Customer {
    int customer_id;
    int class_type;
    int arrival_time;
    int service_time;
};

#endif //P2_CUSTOMER_H
