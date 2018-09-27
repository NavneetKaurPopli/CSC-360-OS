Zhe Chen
V00819544
CSC360 Fall 2018
P1: A Process Manager (PMan)


Description:

PMan is a shell-like process manager that can create program in the background, 
stop the process, start the process and, of cource, kill the process.
You will also be able to list all the running process and get status of each process.


Use Guide:

bg: In order to create the background program, you should call along with the program with its path and arguments if there is any.
e.g. bg ./inf qwe 15 ti run the program <inf> with its parameters <qwe 15>. if you run it directly in Linux shell: ./inf qwe 15

bglist: You can simply type the bglist command to get the list of currently running processes and total process count.

bgstop: By calling bgstop you will temporarily pause a process with pid of your choice.
e.g. bgstop <pid>

bgstart: By calling bgstart you will resumed a process with pid of your choice.
e.g. bgstart <pid>

bgkill: By calling bgkill you will terminate a process with pid of your choice.
e.g. bgkill <pid>

pstat: This will give your the following information related to the process with pid of yoru choice:
	1. comm: 	The filename of the executable, in parentheses
	2. state: 	Process state
	3. utime:	Amount of time that this process has been scheduled in user mode
	4. stime: 	Amount of time that this process has been scheduled in kernel mode
	5. rss:		Resident Set Size is the number of pages the process has in real memory
	6. voluntary ctxt switches:		Number of voluntary context switches (since Linux 2.6.23)
	7. nonvoluntary ctxt switches: 	Number of involuntary context switches (since Linux 2.6.23)
e.g. pstat <pid>
	
	
What's in the tar.gz file:

PMan.c
	main function
PManUtils.h
	Utilities function for PMan
ProcNode.h
	Linked list data structured process node to hold the background programs
inf.c 
	A simple program which prints a tag with time interval.I modified it to stop it from print to the console, instead, 
	it will print the outputs to a file called infprint.txt. I did this because I do not want my PMan to be interupted 
	while the program is running.

	
Running the PMan:

First you need to extract the p1.tar.gz into a folder using tar -zxvf p1.tar.gz. In that folder you will see a Makefile that is ready for you. 
In order to compile the source codes, you can simply put make -f Makefile or make in the command line.

Type ./PMan into the command line and it will execute the program and run the process manager.