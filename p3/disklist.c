/**
 * Zhe Chen
 * CSC360 Fall 2018
 * P3: A Simple File System
 * Part II - disklist
 *
 * In part II, you will write a program, with the routines already implemented
 * for part I, that displays the contents of the root directory and all
 * sub-directories (possibly multi-layers) in the file system.
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
        printf("Usage: ./disklist <file system image>\n");
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

    int root_dir_offset = SECTOR_SIZE*19; // starts from root directory
    char *root_dir_name = "ROOT";
    directory_listing(root_dir_name, memblock, root_dir_offset);

    // clean
    close(fd);
    munmap(memblock, length);

    return EXIT_SUCCESS;
}

