/**
 * Zhe Chen
 * CSC360 Fall 2018
 * P3: A Simple File System
 * Part I - diskinfo
 *
 * In part I, you will write a program that displays information about the file system.
 * In order to complete part I,11 you will need to understand the file system structure
 * of MS-DOS, including FAT Partition Boot Sector, FAT File12 Allocation Table, FAT Root
 * Folder, FAT Folder Structure, and so on.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "SFSUtils.h"

int main(int argc, char *argv[]) {
    if (2 != argc) {
        printf("Usage: ./diskinfo <file system image>\n");
        return EXIT_FAILURE;
    }

    /*
     * map file system image using mmap() to create a new mapping in the
     * virtual address space of the calling process
     * */

    char *memblock;
    int fd;
    struct stat sb; // stat buffer

    if (-1 == (fd = open(argv[1], O_RDONLY))) {
        printf("error: cannot read from file system image.\n");
        return EXIT_FAILURE;
    }

    if (-1 == fstat(fd, &sb)) {
        printf("error: cannot retrieve information about file system image.\n");
        return EXIT_FAILURE;
    }

    uint64_t length = (uint64_t)sb.st_size;
    if (MAP_FAILED == (memblock = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0))) {
        printf("error: cannot map file to the memory space.\n");
        return EXIT_FAILURE;
    }

    /*
     * read information stored in the file system image
     * */
    char *os_name = malloc(sizeof(char));
    char *disk_label = malloc(sizeof(char));
    int total_size = get_total_disk_size(memblock);
    int free_size = get_free_disk_size(memblock);
    // first sector(directory entry) in the root directory, offset
    int root_dir_entry = SECTOR_SIZE*19;
    int disk_file_num =get_disk_file_num(memblock, root_dir_entry);
    int FAT_copy_num = memblock[16]; // Number of FATs in boot sector
    int sector_per_FAT = get_sector_per_FAT(memblock);
    get_os_info(os_name, memblock);
    get_disk_label(disk_label, memblock);

    /*
     * print out information
     * */
    printf("OS Name: %s\n", os_name);
    printf("Label of the disk: %s\n", disk_label);
    printf("Total size of the disk: %d bytes\n", total_size);
    printf("Free size of the disk: %d bytes\n", free_size);
    printf("\n==============\n");
    printf("The number of files in the disk (including all files in the root directory and files in all subdirectories): %d\n", disk_file_num);
    printf("\n=============\n");
    printf("Number of FAT copies: %d\n", FAT_copy_num);
    printf("Sectors per FAT: %d\n", sector_per_FAT);

    /*
     * free memory space
     * */
    close(fd);
    free(os_name);
    free(disk_label);
    munmap(memblock, length);

    return EXIT_SUCCESS;
}