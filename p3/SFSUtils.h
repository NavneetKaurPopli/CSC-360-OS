/**
 * Zhe Chen
 * CSC360 Fall 2018
 * P3: A Simple File System
 * Utility tool - Header file
 */
#ifndef ASSIGNMENT3_SFS_SFSUTILS_H
#define ASSIGNMENT3_SFS_SFSUTILS_H

#define SECTOR_SIZE 512

void get_os_info(char *name, char *ptr);

void get_disk_label(char *label, char *ptr);

int get_nth_FAT_entry(int n, char *p);

int get_total_disk_size(char *ptr);

int get_free_disk_size(char* ptr);

int get_disk_file_num(char *ptr, int offset);

int get_sector_per_FAT(char *ptr);

void get_directory_listing(char *ptr, int offset);

void directory_listing(char *dir_name, char *ptr, int offset);

int get_file_size(char *fname, char *ptr, int offset);

int get_first_logical_cluster(char *fname, char *ptr, int offset);

int make_filecopy(char *fname, int fsize, char *ptr1, char *ptr2);

int get_next_free_cluster(char *ptr);

int has_sub_dir(char **subdirs, int cur_pos, int des_pos, char *ptr, int offset);

void write_FAT_entry(int index, int value, char *ptr);

void update_directory(char *fname, int fsize, int flc, char *ptr, int offset);

int copy_file_to_disk(char *fname, int fsize, char *ptr1, char *ptr2, int offset);

#endif