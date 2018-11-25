Zhe Chen
CSC360 Fall 2018
P2: Airline Check-in System

Description:

An airline check-in system simulation program which simulates four clerks with two
lines one for business class and one for economy class. clerks will serve customers
in business class prior to the economy class. 

Customers information are store in customers.txt file and this program will read 
the file and starts the simulation.

Customer has following attributes:
	1. Class Type:: It indicates whether the customer belongs to business class or economy class.
	2. Arrival Time: It indicates when the customer will arrive.
	3. Service Time: It indicates the time required to serve the customer (i.e., from the time when the customer is
	   27 picked up by a clerk to the time when the clerk finishes serving the customer).

File Format:
	The input file is a text file and has a simple format. The first line contains the total number of customers that will
	be simulated. After that, each line contains the information about a single customer, such that:
		1. The first character specifies the unique ID of customers.
		2. A colon(:) immediately follows the unique number of the customer.
		3. Immediately following is an integer equal to either 1 (indicating the customer belongs to business class) or 0
		   (indicating the customer belongs to economy class).
		4. A comma(,) immediately follows the previous number.
		5. Immediately following is an integer that indicates the arrival time of the customer.
		6. A comma(,) immediately follows the previous number.
		7. Immediately following is an integer that indicates the service time of the customer.
		8. A newline (\n) ends a line.

What's in the tar.gz file:

ACS.c
	main function
Customerh
	file stores data structure for simulation of customer
QueueNode.h QueueNode.c
	First-InFirst-Out Queue structureed using Linked list to hold customer in business and economy class 
customers.txt
	file that contains customer information to the purpose of simulating customer arrival and service time
	
How to run:

First you need to extract the p2.tar.gz into a folder using tar -zxvf p2.tar.gz. In that folder you will see a Makefile that is ready for you. 
In order to compile the source codes, you can simply put 'make -f Makefile' or 'make' in the console.

Type './ACS customers.txt' and it will start the simulation and eventually print average waiting times.
