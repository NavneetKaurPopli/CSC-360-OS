/**
 * Zhe Chen
 * CSC360 Fall 2018
 * P3: A Simple File System
 * Utility tool - Implementation of the whole requirements
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "SFSUtils.h"

/**
 * get OS information
 * @param name
 * @param ptr
 */
void get_os_info(char *name, char *ptr) {
    int i;

    // starting byte:3, length:8 bytes (boot sector)
    for (i=0; i<8; i++) {
        name[i] = ptr[i+3];
    }
}

/**
 * get disk label
 * @param label
 * @param ptr
 */
void get_disk_label(char *label, char *ptr) {
    int i;

    // starting byte:43, length:11 bytes (boot sector)
    for (i=0; i<8; i++) {
        label[i] = ptr[i+43];
    }

    // root directory
    if (' ' == label[0]) {
        ptr += SECTOR_SIZE*19; // root directory pos

        while (0x00 != ptr[0]) {
            if (8 == ptr[11]) {
                for (i=0; i<8; i++) {
                    label[i] = ptr[i];
                }
            }

            ptr += 32;
        }
    }
}

/**
 * get nth FAT entry by any logical cluster number
 * @param n
 * @param ptr
 * @return
 */
int get_nth_FAT_entry(int n, char *ptr) {
    int result;
    int first_portion;
    int second_portion;
    ptr += SECTOR_SIZE; // skip the boot sector

    if (0 == n%2) {
        // n is even
        // lower 4 bits in location 1+(3*n)/2
        // and the 8 bits in location (3*n)/2
        first_portion = ptr[1+(3*n)/2] & 0x0F;
        second_portion = ptr[(3*n)/2] & 0xFF;
        result = (first_portion<<8) + second_portion;
    }
    else {
        // n is odd
        // high 4 bits in location (3*n)/2
        // and the 8 bits in location 1+(3*n)/2
        first_portion = ptr[(3*n)/2] & 0xF0;
        second_portion = ptr[1+(3*n)/2] & 0xFF;
        result = (first_portion>>4) + (second_portion<<4);
    }

    return result;
}

/**
 * get total disk size
 * @param ptr
 * @return
 */
int get_total_disk_size(char *ptr) {
    // starting byte:19, length: 2(total sector count)
    int total_sector_count = ptr[19] + (ptr[20]<<8);

    return total_sector_count*SECTOR_SIZE;
}

int get_free_disk_size(char *ptr) {
    int free_sector_count = 0;

    int i;
    // skip first two because they are reserved
    for (i=2; i<=2848; i++) {
        if (0x00 == get_nth_FAT_entry(i, ptr)) {
            // count unused which has value of 0x00
            free_sector_count++;
        }
    }

    return free_sector_count*SECTOR_SIZE;
}

/**
 * get number of files in the disk
 * @param ptr
 * @param offset
 * @return
 */
int get_disk_file_num(char *ptr, int offset) {
    int logical_cluster = offset/SECTOR_SIZE-31;
    int num = 0;

    int entries_count;

    int temp = logical_cluster;
    do {
        // logical cluster to physical address, point to the beginning
        //
        offset = SECTOR_SIZE * (temp+31);
        // one sector contains 512/32 = 16 entries
        // if curr dir is root(offset = 19 * 512) then 14 sectors (19~32)
        entries_count = (-12 == logical_cluster) ? -208 : 0;
        while (0x00 != ptr[offset+0] && 16 > entries_count) {
            if ('.' != ptr[offset+0] && 0xE5 != ptr[offset+0] && 0x0F != ptr[offset+11] && 0x00 == (ptr[offset+11]&0x02) && 0x00 == (ptr[offset+11]&0x08)) {
                if (0x10 == (ptr[offset+11]&0x10)) {
                    int first_logical_cluster = ((ptr[offset+27]&0xFF)<<8) + (ptr[offset+26]&0xFF);
                    int subdir_offset = SECTOR_SIZE*(31+first_logical_cluster); // locate physical address
                    num += get_disk_file_num(ptr, subdir_offset);
                }
                if (0x00 == (ptr[offset+11]&0x10)) {
                    num++;
                }
            }
            // next directory entry
            offset += 32;
            // one sector holds 16 entries
            entries_count++;
        }
        // check FAT tables
        // 0xFF0-0xFF6: Reserved cluster
        // 0xFF7: Bad cluster
        // 0xFF8-0xFFF: Last cluster in a file
        // so it needs to be less than 0xff0 to continue
        // 0x00: Unused
        temp = (1<temp) ? get_nth_FAT_entry(temp, ptr) : 0x00;
    } while (0x00 != temp && 0xff0 > temp);

    return num;
}

int get_sector_per_FAT(char *ptr) {
    int result = (ptr[23]<<8) + ptr[22];

    return result;
}

/**
 * a helper function for directory_listing
 * it lists out all the folders and directories in one sector
 * @param ptr
 * @param offset
 */
void  get_directory_listing(char *ptr, int offset) {
    int i;
    int logical_cluster = offset/SECTOR_SIZE-31;

    char *entry_name = malloc(sizeof(char));
    char *file_ext = malloc(sizeof(char));

    // one sector contains 512/32 = 16 entries
    // if curr dir is root(offset = 19 * 512) then 14 sectors (19~32)
    int entries_count = (-12 == logical_cluster) ? -208 : 0;

    while (0x00 != ptr[offset+0] && 16 > entries_count) {
        char entry_type;
        entry_name = (char *)realloc(entry_name, sizeof(char));
        file_ext = (char *)realloc(file_ext, sizeof(char));

        int file_size;
        int creation_year;
        int creation_month;
        int creation_day;
        int creation_hour;
        int creation_minute;

        /* check if directory entry is free (i.e., currently unused)
         * check if entry is either curr dir or root dir
         * check if the Attributes byte is 0x0F, part of long file name
         * check if directory entry is hidden
         * check if directory entry is volume label
         * */
        if ('.' != ptr[offset+0] && 0xE5 != ptr[offset+0] && 0x0F != ptr[offset+11] && 0x00 == (ptr[offset+11]&0x02) && 0x00 == (ptr[offset+11]&0x08)) {
            if (0x00 == (ptr[offset+11]&0x10)) {
                entry_type = 'F';
            }
            else {
                entry_type = 'D';
            }

            // get entry name
            for (i=0; i<8; i++) {
                if (' ' == ptr[offset+i]) {
                    break;
                }
                else {
                    entry_name[i] = ptr[offset+i];
                }
            }
            entry_name[i] = '\0';

            // get extension
            for (i=8; i<11; i++) {
                if (' ' == ptr[offset+i]) {
                    break;
                }
                else {
                    file_ext[i-8] = ptr[offset+i];
                }
            }
            file_ext[i-8] = '\0';

            // starting offset: 28, len: 4
            file_size = ((ptr[offset+31]&0xFF)<<24) + ((ptr[offset+30]&0xFF)<<16) + ((ptr[offset+29]&0xFF)<<8) + (ptr[offset+28]&0xFF);

            // offset:[    16   I    17   ]
            //   date: 0000 0000 0000 0000
            //   year:           xxxx xxx
            //  month: xxx               x
            //    day:    x xxxx
            creation_year = ((ptr[offset+17]&0b11111110)>>1) + 1980;
            // Month: middle 4 bits
            creation_month = (((ptr[offset+17]&0b00000001))<<3) + ((ptr[offset+16]&0b11100000)>>5);
            creation_day = (ptr[offset+16]&0b00011111);

            // offset:[    14   I    15   ]
            //   time: 0000 0000 0000 0000
            //   hour:           xxxx x
            //    min: xxx             xxx
            creation_hour = (ptr[offset+15]&0b11111000)>>3;
            creation_minute = ((ptr[offset+15]&0b00000111)<<3) + ((ptr[offset+14]&0b11100000)>>5);

            // if is file, add extension to the file name
            strncpy(entry_name, entry_name, 8);
            if ('F' == entry_type) {
                strcat(entry_name, ".");
                strncat(entry_name, file_ext, 3);
            }

            if ('F' == entry_type) {
                printf("%c %10d %20s %d-%02d-%02d %02d:%02d\n", entry_type, file_size, entry_name, creation_year, creation_month, creation_day, creation_hour, creation_minute);
            }
            else {
                printf("%c %10s %20s %d-%02d-%02d %02d:%02d\n", entry_type, "-", entry_name, creation_year, creation_month, creation_day, creation_hour, creation_minute);
            }
        }

        // next directory entry
        offset += 32;
        entries_count++;
    }

    // free memory space
    free(entry_name);
    free(file_ext);
}

/**
 * disklist
 * lists all the folders and directories in all folders starts from ROOT DIR
 * @param dir_name
 * @param ptr
 * @param offset
 */
void directory_listing(char *dir_name, char *ptr, int offset) {
    int logical_cluster = offset/SECTOR_SIZE-31;

    int *sub_dir_cluster_addr = malloc(sizeof(int) * 1000); // array that stores physical address of subdirs in curr dir
    char **sub_dir_name = malloc(sizeof(char *) * 1000); // array that stores name of subdirs in curr dir
    int sub_dir_count = 0;
    int entries_count;
    int i;
    char *entry_name = malloc(sizeof(char));

    // indexing all the sub directories
    int temp = logical_cluster;
    do {
        // logical cluster to physical address, point to the beginning
        //
        offset = SECTOR_SIZE * (temp+31);
        // one sector contains 512/32 = 16 entries
        // if curr dir is root(offset = 19 * 512) then 14 sectors (19~32)
        entries_count = (-12 == logical_cluster) ? -208 : 0;
        while (0x00 != ptr[offset+0] && 16 > entries_count) {
            entry_name = (char *)realloc(entry_name, sizeof(char));
            if ('.' != ptr[offset+0] && 0xE5 != ptr[offset+0] && 0x0F != ptr[offset+11] && 0x00 == (ptr[offset+11]&0x02) && 0x00 == (ptr[offset+11]&0x08)) {
                if (0x10 == (ptr[offset+11]&0x10)) {
                    // get entry name
                    for (i=0; i<8; i++) {
                        if (' ' == ptr[offset+i]) {
                            break;
                        }
                        else {
                            entry_name[i] = ptr[offset+i];
                        }
                    }
                    entry_name[i] = '\0';

                    // stores into arrays to be called later
                    strncpy(entry_name, entry_name, 8);
                    sub_dir_name[sub_dir_count] = strcat(strcat(strdup(dir_name), "/"), strdup(entry_name));
                    int first_logical_cluster = ((ptr[offset+27]&0xFF)<<8) + (ptr[offset+26]&0xFF);
                    int subdir_offset = SECTOR_SIZE*(31+first_logical_cluster); // locate physical address
                    sub_dir_cluster_addr[sub_dir_count] = subdir_offset;
                    sub_dir_count++;
                }
            }
            // next directory entry
            offset += 32;
            // one sector holds 16 entries
            entries_count++;
        }
        // check FAT tables
        // 0xFF0-0xFF6: Reserved cluster
        // 0xFF7: Bad cluster
        // 0xFF8-0xFFF: Last cluster in a file
        // so it needs to be less than 0xff0 to continue
        // 0x00: Unused
        temp = (1<temp) ? get_nth_FAT_entry(temp, ptr) : 0x00;
    } while (0x00 != temp && 0xff0 > temp);

    // starts print out info about current directory
    printf("%s\n==================\n", dir_name);
    do {
        offset = SECTOR_SIZE * (logical_cluster+31);
        get_directory_listing(ptr, offset);
        // check FAT tables
        logical_cluster = (1<logical_cluster) ? get_nth_FAT_entry(logical_cluster, ptr) : 0x00;
    } while (0x00 != logical_cluster && 0xff0 > logical_cluster);

    // print listing of all the subdirs in curr dir
    for (i=0; i<sub_dir_count; i++) {
        directory_listing(sub_dir_name[i], ptr, sub_dir_cluster_addr[i]);
        // free space
        free(sub_dir_name[i]);
    }

    // free memory
    free(entry_name);
    free(sub_dir_cluster_addr);
    free(sub_dir_name);
}

/**
 * get file size by file name
 * @param fname
 * @param ptr
 * @param offset
 * @return
 */
int get_file_size(char *fname, char *ptr, int offset) {
    int logical_cluster = offset/SECTOR_SIZE-31;
    int result = -1;
    int entries_count;
    char *file_name = malloc(sizeof(char));
    char *file_ext = malloc(sizeof(char));
    int i;

    do {
        // one sector contains 512/32 = 16 entries
        // if curr dir is root(offset = 19 * 512) then 14 sectors (19~32)
        entries_count = (-12 == logical_cluster) ? -208 : 0;

        offset = SECTOR_SIZE * (logical_cluster+31);
        while (0x00 != ptr[offset+0] && 16 > entries_count) {
            file_name = (char *)realloc(file_name, sizeof(char));
            file_ext = (char *)realloc(file_ext, sizeof(char));

            if ('.' != ptr[offset+0] && 0xE5 != ptr[offset+0] && 0x0F != ptr[offset+11] && 0x00 == (ptr[offset+11]&0x02) && 0x00 == (ptr[offset+11]&0x08)) {
                if (0x00 == (ptr[offset+11]&0x10)) {
                    // get file name
                    for (i=0; i<8; i++) {
                        if (' ' == ptr[offset+i]) {
                            break;
                        }
                        else {
                            file_name[i] = ptr[offset+i];
                        }
                    }
                    file_name[i] = '\0';

                    // get extension
                    for (i=8; i<11; i++) {
                        if (' ' == ptr[offset+i]) {
                            break;
                        }
                        else {
                            file_ext[i-8] = ptr[offset+i];
                        }
                    }
                    file_ext[i-8] = '\0';

                    // combine file name and extension
                    strncpy(file_name, file_name, 8);
                    strcat(file_name, ".");
                    strncat(file_name, file_ext, 3);

                    // check if is a match
                    if (0 == strcmp(fname, file_name)) {
                        // starting offset: 28, len: 4
                        result = ((ptr[offset+31]&0xFF)<<24) + ((ptr[offset+30]&0xFF)<<16) + ((ptr[offset+29]&0xFF)<<8) + (ptr[offset+28]&0xFF);
                        break;
                    }
                }
                else {
                    int first_logical_cluster = ((ptr[offset+27]&0xFF)<<8) + (ptr[offset+26]&0xFF);
                    int subdir_offset = SECTOR_SIZE*(31+first_logical_cluster);

                    result = get_file_size(fname, ptr, subdir_offset);
                    if (-1 != result) {
                        break;
                    }
                }
            }
            offset += 32;
            entries_count++;
        }

        if (-1 != result) {
            break;
        }
        logical_cluster = (1<logical_cluster) ? get_nth_FAT_entry(logical_cluster, ptr) : 0x00;
    } while (0x00 != logical_cluster && 0xff0 > logical_cluster);

    free(file_name);
    free(file_ext);

    return result;
}

/**
 * get first logical cluster of the file that points to its physical address by file name
 * @param fname
 * @param ptr
 * @param offset
 * @return
 */
int get_first_logical_cluster(char *fname, char *ptr, int offset) {
    int logical_cluster = offset/SECTOR_SIZE-31;
    int result = -1;
    int entries_count;
    char *file_name = malloc(sizeof(char));
    char *file_ext = malloc(sizeof(char));
    int i;

    do {
        // one sector contains 512/32 = 16 entries
        // if curr dir is root(offset = 19 * 512) then 14 sectors (19~32)
        entries_count = (-12 == logical_cluster) ? -208 : 0;
        offset = SECTOR_SIZE * (logical_cluster+31);
        while (0x00 != ptr[offset+0] && 16 > entries_count) {
            file_name = (char *)realloc(file_name, sizeof(char));
            file_ext = (char *)realloc(file_ext, sizeof(char));

            if ('.' != ptr[offset+0] && 0xE5 != ptr[offset+0] && 0x0F != ptr[offset+11] && 0x00 == (ptr[offset+11]&0x02) && 0x00 == (ptr[offset+11]&0x08)) {
                if (0x00 == (ptr[offset+11]&0x10)) {
                    // get file name
                    for (i=0; i<8; i++) {
                        if (' ' == ptr[offset+i]) {
                            break;
                        }
                        else {
                            file_name[i] = ptr[offset+i];
                        }
                    }
                    file_name[i] = '\0';

                    // get extension
                    for (i=8; i<11; i++) {
                        if (' ' == ptr[offset+i]) {
                            break;
                        }
                        else {
                            file_ext[i-8] = ptr[offset+i];
                        }
                    }
                    file_ext[i-8] = '\0';

                    // combine file name and extension
                    strncpy(file_name, file_name, 8);
                    strcat(file_name, ".");
                    strncat(file_name, file_ext, 3);

                    // check if is a match
                    if (0 == strcmp(fname, file_name)) {
                        // starting offset: 26, len: 2
                        result = ((ptr[offset+27]&0xFF)<<8) + (ptr[offset+26]&0xFF);
                        break;
                    }
                }
                else {
                    int first_logical_cluster = ((ptr[offset+27]&0xFF)<<8) + (ptr[offset+26]&0xFF);
                    int subdir_offset = SECTOR_SIZE*(31+first_logical_cluster);

                    result = get_first_logical_cluster(fname, ptr, subdir_offset);
                    // once get the result return it
                    if (-1 != result) {
                        break;
                    }
                }
            }
            offset += 32;
            entries_count++;
        }
        // once get the result return it
        if (-1 != result) {
            break;
        }
        logical_cluster = (1<logical_cluster) ? get_nth_FAT_entry(logical_cluster, ptr) : 0x00;
    } while (0x00 != logical_cluster && 0xff0 > logical_cluster);

    // free memory
    free(file_name);
    free(file_ext);

    return result;
}

/**
 * copy file from disk to local directory
 * @param fname
 * @param fsize
 * @param ptr1
 * @param ptr2
 * @return
 */
int make_filecopy(char * fname, int fsize, char *ptr1, char *ptr2) {
    int logical_cluster;
    // make sure no error occurred when getting the cluster number
    if (-1 == (logical_cluster = get_first_logical_cluster(fname, ptr1, SECTOR_SIZE*19))) {
        return -1;
    }

    int remaining_byte = fsize; // remaining byte to write to written file
    int offset; // physical address of the current cluster
    int i;

    do {
        offset = SECTOR_SIZE * (logical_cluster+31);

        for (i=0; i<SECTOR_SIZE; i++) {
            if (0 == remaining_byte) {
                // reach end of the written file
                break;
            }

            // write to the file byte by byte
            ptr2[fsize-remaining_byte] = ptr1[offset+i];
            remaining_byte--;
        }

        // get number of next cluster
        logical_cluster = get_nth_FAT_entry(logical_cluster, ptr1);
    } while (0x00 != logical_cluster && 0xff0 > logical_cluster);

    return 0;
}

/**
 * get next free cluster value in FAT table
 * @param ptr
 * @return
 */
int get_next_free_cluster(char *ptr) {
    int i;
    for (i=2; i<=2848; i++) {
        if (0x00 == get_nth_FAT_entry(i, ptr)) {
            // free cluster found, return its value
            return i;
        }
    }

    return -1; // if return -1, error: cannot find any free cluster
}

/**
 * check if curr directory contains subdirs[cur_pos] dir
 * if exists return the logical cluster value of this dir
 * @param subdirs
 * @param cur_pos
 * @param des_pos
 * @param ptr
 * @param offset
 * @return
 */
int has_sub_dir(char **subdirs, int cur_pos, int des_pos, char *ptr, int offset) {
    int logical_cluster = offset/SECTOR_SIZE-31;
    int entries_count;
    int i;

    char *dir_name = malloc(sizeof(char));

    do {
        // one sector contains 512/32 = 16 entries
        // if curr dir is root(offset = 19 * 512) then 14 sectors (19~32)
        entries_count = (-12 == logical_cluster) ? -208 : 0;
        offset = SECTOR_SIZE*(logical_cluster+31);

        while (0x00 != ptr[offset+0] && 16 > entries_count) {
            dir_name = (char *)realloc(dir_name, sizeof(char));

            if ('.' != ptr[offset+0] && 0xE5 != ptr[offset+0] && 0x0F != ptr[offset+11] && 0x00 == (ptr[offset+11]&0x02) && 0x00 == (ptr[offset+11]&0x08)) {
                // check if is a directory(folder) entry
                if (0x10 == (ptr[offset+11]&0x10)) {
                    // get directory name
                    for (i=0; i<8; i++) {
                        if (' ' == ptr[offset+i]) {
                            break;
                        }
                        else {
                            dir_name[i] = ptr[offset+i];
                        }
                    }
                    dir_name[i] = '\0';

                    // check if dir_name matches subdir
                    if (0 == strcmp(dir_name, subdirs[cur_pos])) {
                        if (cur_pos == des_pos) {
                            // base case where it reached the last subdir
                            return ((ptr[offset+27]&0xFF)<<8) + (ptr[offset+26]&0xFF);
                        } else {
                            // get offset(physical address) of this subdir
                            int first_logical_cluster = ((ptr[offset+27]&0xFF)<<8) + (ptr[offset+26]&0xFF);
                            int subdir_offset = SECTOR_SIZE*(31+first_logical_cluster);

                            return 0 + has_sub_dir(subdirs, cur_pos+1, des_pos, ptr, subdir_offset);
                        }
                    }
                }
            }

            // next entry
            offset += 32;
            entries_count++;
        }
        // get number of next cluster
        logical_cluster = get_nth_FAT_entry(logical_cluster, ptr);
    } while(0x00 != logical_cluster && 0xff0 > logical_cluster);

    return -1;
}

/**
 * write value to cluster index in FAT table of disk system
 * @param index
 * @param value
 * @param ptr
 */
void write_FAT_entry(int index, int value, char *ptr) {
    ptr += SECTOR_SIZE; // skip the boot sector

    if (0 == index%2) {
        // even
        ptr[1+(3*index)/2] = ((value>>8)&0x0F)|(ptr[1+(3*index)/2]&0xF0);
        ptr[(3*index)/2] = value&0xFF;
    }
    else {
        ptr[(3*index)/2] = ((value<<4)&0xF0)|(ptr[(3*index)/2]&0x0F);
        ptr[1+(3*index)/2] = (value>>4)&0xFF;
    }
}

/**
 * update the list of directory with new file inserted
 * @param fname
 * @param fsize
 * @param flc: first logical cluster of the target file
 * @param ptr: disk
 * @param offset: physical address of the directory
 */
void update_directory(char *fname, int fsize, int flc, char *ptr, int offset) {
    int logical_cluster = offset/SECTOR_SIZE-31;
    int target_cluster = logical_cluster;
    int next = target_cluster;
    // find last sector of dir listing
    while (-12 != logical_cluster && 0xff0 > next) {
        target_cluster = next;
        next = get_nth_FAT_entry(target_cluster, ptr);
    }
    //found offset of target sector
    int target_offset = SECTOR_SIZE*(target_cluster+31);

    int flag = -1;
    int entries_count = 0;
    int max_count = (-12 == logical_cluster) ? 224 : 16;

    while (max_count > entries_count) {
        if (0x00 == ptr[target_offset+0]) {
            // found free dir entry
            flag = 0;
            break;
        }
        target_offset += 32;
        entries_count++;
    }

    if (-1 == flag) {
        // no free space in the last sector
        // need to create new cluster and write to new sector
        int new_cluster = get_next_free_cluster(ptr);

        // handle error when not found new space in FAT table
        if (-1 == new_cluster) {
            printf("error: not enough free space\n");
            exit(EXIT_FAILURE);
        }

        write_FAT_entry(target_cluster, new_cluster, ptr);
        write_FAT_entry(new_cluster, 0xFFF, ptr);
        update_directory(fname, fsize, flc, ptr, SECTOR_SIZE*(new_cluster+31));
        return;
    }

    /* create new dir entry and insert into directory list */
    // target_offset = SECTOR_SIZE*(target_cluster+31);
    int i;
    int ext_pos = -1;
    // file name: 0~7
    for (i=0; i<8; i++) {
        if ('.' == fname[i]) {
            break;
        }
        else {
            ptr[target_offset+i] = fname[i];
        }
    }
    for (; i<8; i++) {
        // fill the rest with space
        ptr[target_offset+i] = ' ';
    }

    for (i=0; i<256; i++) {
        if ('.' == fname[i]) {
            ext_pos = i+1;
            break;
        }
    }

    // extension: 8~10
    for (i=0; i<3; i++) {
        ptr[target_offset+8+i] = fname[ext_pos+i];
    }

    // set file attributes: 11
    ptr[target_offset+11] = 0x00;

    // set creation time and date:: 14~15 and 16~17
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    int year = now->tm_year+1900; // year since 1900
    int month = now->tm_mon+1;
    int day = now->tm_mday; // day of month [1, 31]
    int hour = now->tm_hour;
    int minute = now->tm_min;
    // init
    ptr[target_offset+14] = 0;
    ptr[target_offset+15] = 0;
    ptr[target_offset+16] = 0;
    ptr[target_offset+17] = 0;
    // write
    ptr[target_offset+17] |= (year-1980)<<1; // high 7 bits
    ptr[target_offset+17] |= (month-((ptr[target_offset+16]&0b11100000)>>5))>>3; // middle 4 bits
    ptr[target_offset+16] |= (month-((ptr[target_offset+17]&0b00000001)<<3))<<5;
    ptr[target_offset+16] |= (day&0b00011111); // lower 5 bits
    ptr[target_offset+15] |= (hour<<3)&0b11111000; // high 5 bits
    ptr[target_offset+15] |= (minute-((ptr[target_offset+14]&0b11100000)>>5))>>3; // middle 6 bits
    ptr[target_offset+14] |= (minute-((ptr[target_offset+15]&0b00000111)<<3))<<5;
    // set first logical cluster value: 26~27
    ptr[target_offset+26] = flc&0xFF;
    ptr[target_offset+27] = (flc&0xFF00)>>8;
    // set file size: 28~31
    ptr[target_offset+28] = (fsize&0x000000FF);
    ptr[target_offset+29] = (fsize&0x0000FF00)>>8;
    ptr[target_offset+30] = (fsize&0x00FF0000)>>16;
    ptr[target_offset+31] = (fsize&0xFF000000)>>24;
}

/**
 * copy file in ptr2 into disk ptr1
 * update directory entry(link first free cluster value as FLC) -> copy file into free space
 * @param fname
 * @param fsize
 * @param ptr1: disk image
 * @param ptr2: file copy
 * @param offset: dir offset
 * @return
 */
int copy_file_to_disk(char *fname, int fsize, char *ptr1, char *ptr2, int offset) {
    int remaining_byte = fsize;
    int curr_cluster = get_next_free_cluster(ptr1);
    int cluster_offset;
    int i;

    // if did not find next free cluster value, return error value
    if (-1 == curr_cluster) {
        return -1;
    }

    // make sure the file does not exist in the directory
    if (-1 == get_first_logical_cluster(fname, ptr1, offset)) {
        // update info in directory
        update_directory(fname, fsize, curr_cluster, ptr1, offset);

        while (0 < remaining_byte) {
            if (-1 == curr_cluster) {
                return -1;
            }
            cluster_offset = SECTOR_SIZE*(curr_cluster+31); // physical address of the cluster index

            // write into sector by sector, one sector is corresponding to one cluster value
            for (i=0; i<SECTOR_SIZE; i++) {
                if (0 == remaining_byte) {
                    // finished copy
                    write_FAT_entry(curr_cluster, 0xFFF, ptr1);
                    return 0;
                }
                ptr1[i+cluster_offset] = ptr2[fsize-remaining_byte];
                remaining_byte--;
            }

            if (0 == remaining_byte) {
                write_FAT_entry(curr_cluster, 0xFFF, ptr1);
                return 0;
            }

            // place holder for current cluster so that it will
            // skip current one(because current one is filled!) and find the next cluster
            write_FAT_entry(curr_cluster, curr_cluster, ptr1);
            int next_cluster = get_next_free_cluster(ptr1);
            // write the value of the next cluster in position of current cluster
            write_FAT_entry(curr_cluster, next_cluster, ptr1);
            curr_cluster = next_cluster;
        }
    }
    else {
        printf("File already exists.\n");
    }

    return 0;
}
