/**
 * Zhe Chen
 * CSC360 Fall 2018
 * P3: A Simple File System
 * Part III - diskget
 *
 * In part III, you will write a program that copies a file from the file system
 * to the current directory in Linux. If the specified file cannot be found in
 * the file system, you should output the message File not found. and exit.
 *
 * Note: In order to find a file in the file system, the root directory and
 * all subdirectories (possibly multi-layers) should be searched.
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
    if (3 != argc) {
        printf("Usage: ./diskget <file system image> <file>\n");
        return EXIT_FAILURE;
    }

    /*
     * map file system image using mmap() to create a new mapping in the
     * virtual address space of the calling process
     * */

    char *memblock;
    char *memblock_written;
    int fd;
    int fd2;
    off_t ret;
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
        close(fd);
        return EXIT_FAILURE;
    }

    int root_dir_offset = SECTOR_SIZE*19; // starts from root directory
    int file_size = get_file_size(argv[2], memblock, root_dir_offset);

    if (-1 == file_size) {
        printf("File not found.\n");
    }
    else {
        // create a local file to store the file
        if (-1 == (fd2 = open(argv[2], O_RDWR | O_CREAT, 0666))) {
            printf("error: cannot create file to local directory.\n");
            close(fd);
            return EXIT_FAILURE;
        }

        if ((off_t)-1 == (ret = lseek(fd2, file_size-1, SEEK_SET))) {
            printf("error: lseek() failed.\n");
            close(fd);
            close(fd2);
            munmap(memblock, length);
            return EXIT_FAILURE;
        }

        if ((off_t)-1 == (ret = write(fd2, "", 1))) {
            printf("error: write() failed.\n");
            close(fd);
            close(fd2);
            munmap(memblock, length);
            return EXIT_FAILURE;
        }

        if (MAP_FAILED == (memblock_written = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, fd2, 0))) {
            printf("error: cannot map local file to the memory space.\n");
            close(fd);
            close(fd2);
            munmap(memblock, length);
            return EXIT_FAILURE;
        }

        if (-1 == make_filecopy(argv[2], file_size, memblock, memblock_written)) {
            printf("error: cannot make file copy to local directory.\n");
            close(fd);
            close(fd2);
            munmap(memblock, length);
            munmap(memblock_written, file_size);
            return EXIT_FAILURE;
        }

        // clean
        close(fd2);
        munmap(memblock_written, file_size);
    }

    // clean
    close(fd);
    munmap(memblock, length);

    return EXIT_SUCCESS;

}