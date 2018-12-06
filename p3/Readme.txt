Zhe(Kevin)
CSC360 Fall 2018
P3: A Simple File System (SFS)

Description:

A simple file system that reads data from FAT 12 file system used by MS-DOS. Implementation includes
reading data, listing folders and files in every directories, copying file stored in the FAT 12 to 
local current directory, and copying file stored in the local directory to FAT 12 disk file system.

The File Allocation Table(FAT) is a table stored on a hard disk or floppy disk that indicates the status 
and location of all data clusters that are on the disk. The FAT12 is the file system on a floppy disk. The 
number 12 is derived from the fact that the FAT consists of 12-bit entries. For the floppy disk, the number 
of sectors in a cluster is one. Also, the size of a sector(and hence a cluster) is 512 bytes for a floppy disk.

FAT12 Disk Organization:
Disk-Sectors		Name-of-section
0			Boot Sector
1~18			FAT tables
19~32			Root Directory
33~2879			Data Area

File cluster: 2~2848

What is inside of the tar.gz file:

test_image
	A folder that contains some FAT12 file system images for testing purpose
test_file
	A folder that contains some text files for testing diskput function
SFSUtils.h SFSUtils.c
	Utility tool function that holds the main implementations of this assignment
diskinfo.c
	A program that displays information about the file system.
disklist.c
	A program that displays the contents of the root directory and all other sub-directories in the 
	file system. Contents include entry type('F': file, 'D': directory), size in byte, full name, creation 
	date and time.
diskget.c
	A program that copies a file from the FAT12 file system to the current directory. If specified file cannot 
	be found in the file system, it will output 'File not found' message. This specified file can be in any 
	directory(root directory and sub-directories).
diskput.c
	A program that copies a file from the current directory into a specified directory(root directory or sub-directories) 
	of the FAT12 File system. format: ./diskput disk.IMA /subdir1/subdir2/copyfile.txt
	
How to run:

First you need to extract the p3.tar.gz into a folder using tar -zxvf p3.tar.gz. In that folder you will see a Makefile 
that is ready for you. 
In order to compile the source codes, you can simply put 'make -f Makefile' or 'make' in the console.
Then you could try commands below: 
	Part I: 
		./diskinfo <file system image> example: ./diskinfo disk.IMA will print out the data information
	Part II:
		./disklist <file system image> example: ./disklist disk.IMA will print out contents of all the directories
	Part III:
		./diskget <file system image> <file to copy> example: ./diskget disk.IMA foo.txt will copy the foo.txt file in the disk 
		to the current directory in Linux or Mac.
	Part IV:
		./diskput <file system image> [<file> | <file with full path>] example: ./diskput disk.IMA /subdir1/subdir2/foo.txt will copy the foo.txt file 
		stored in the current directory to the file system with specified directory 'root/subdir1/subdir2'.
	
	
	
	
