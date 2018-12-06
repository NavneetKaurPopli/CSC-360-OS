/**
 * Zhe Chen
 * CSC360 Fall 2018
 * P3: A Simple File System
 * Part IV - diskput
 *
 * You will write a program that copies a file from the current Linux directory
 * into specified directory (i.e., the root directory or a subdirectory) of the
 * file system. If the specified file is not found, you should output the message
 * File not found. and exit. If the specified directory is not found in the file
 * system, you should output the message The directory not found. and exit. If
 * the file system does not have enough free space to store the file, you should
 * output the message No enough free space in the disk image. and exit.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "SFSUtils.h"

#define MAX_CHAR_SIZE 255 // char encoding maximum limit

int main(int argc, char *argv[]) {
    if (3 != argc) {
        printf("Usage: ./diskput <file system image> [<file> | <file with full path>] e.g. /subdir1/subdir2/foo.txt\n");
        return EXIT_FAILURE;
    }

    char *memblock; // disk
    char *memblock2; // file to be copied
    int fd;
    int fd2;
    struct stat sb; // stat buffer
    struct stat sb2;
    char inputs[MAX_CHAR_SIZE] = ""; // stores file (and full path)

    strncpy(inputs, argv[2], 255);
    inputs[254] = '\0';

    // file system image with write and read
    if (-1 == (fd = open(argv[1], O_RDWR))) {
        printf("error: cannot read from file system image.\n");
        return EXIT_FAILURE;
    }

    if (-1 == fstat(fd, &sb)) {
        printf("error: cannot retrieve information about file system image.\n");
        close(fd);
        return EXIT_FAILURE;
    }

    // holds subdir(s)
    char **subdir = malloc(sizeof(char *) * 1000);
    // get file name with extension in current directory
    char *file_name;

    char *temp;
    int subdir_len = 0;

    // format: /subdir1/subdir2/foo.txt
    char *delim = "/";
    temp = strtok(inputs, delim);

    while (NULL != temp) {
        subdir[subdir_len] = temp;
        temp = strtok(NULL, delim);
        subdir_len++;
    }

    // last one is the file
    file_name = strdup(subdir[subdir_len-1]);
    // subdir count - 1
    subdir_len--;

    if (-1 == (fd2 = open(file_name, O_RDONLY))) {
        printf("File not found.\n");
        close(fd);
        return EXIT_FAILURE;
    }

    if (-1 == fstat(fd2, &sb2)) {
        printf("error: cannot retrieve information about %s.\n", file_name);
        close(fd);
        close(fd2);
        free(subdir);
        return EXIT_FAILURE;
    }

    // get size
    uint64_t disk_len = (uint64_t)sb.st_size;
    uint64_t file_len = (uint64_t)sb2.st_size;

    /*
     * map file system image and file to copy using mmap() to create a new mapping in the
     * virtual address space of the calling process
     * */
    if (MAP_FAILED == (memblock = mmap(NULL, disk_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0))) {
        printf("error: cannot map system image to the memory space.\n");
        close(fd);
        close(fd2);
        free(subdir);
        return EXIT_FAILURE;
    }

    if (MAP_FAILED == (memblock2 = mmap(NULL, file_len, PROT_READ, MAP_SHARED, fd2, 0))) {
        printf("error: cannot map file to the memory space.\n");
        munmap(memblock, disk_len);
        close(fd);
        close(fd2);
        free(subdir);
        return EXIT_FAILURE;
    }

    // check if disk has enough free space
    if (file_len <= get_free_disk_size(memblock)) {
        // destination directory First Logical Cluster value
        int dest_dir_FLC;

        // check if the specified directory is found in the file system
        int root_dir_offset = SECTOR_SIZE*19;
        if (0 != subdir_len && -1 == (dest_dir_FLC = has_sub_dir(subdir, 0, subdir_len-1, memblock, root_dir_offset))) {
            printf("The directory not found.\n");
            munmap(memblock, disk_len);
            munmap(memblock2, file_len);
            close(fd);
            close(fd2);
            free(subdir);
            return EXIT_FAILURE;
        }

        int start_offset = (0 == subdir_len) ? root_dir_offset : SECTOR_SIZE*(dest_dir_FLC+31);

        // start copying file to the disk
        if (-1 == copy_file_to_disk(file_name, (int)file_len, memblock, memblock2, start_offset)){
            printf("error: cannot copy file to disk.\n");
            munmap(memblock, disk_len);
            munmap(memblock2, file_len);
            close(fd);
            close(fd2);
            free(subdir);
            return EXIT_FAILURE;
        }
    }
    else {
        // not enough free space to copy this file
        printf("No enough free space in the disk image.\n");
        munmap(memblock, disk_len);
        munmap(memblock2, file_len);
        close(fd);
        close(fd2);
        free(subdir);
        return EXIT_FAILURE;
    }


    munmap(memblock, disk_len);
    munmap(memblock2, file_len);
    close(fd);
    close(fd2);
    free(subdir);
    return EXIT_SUCCESS;

}