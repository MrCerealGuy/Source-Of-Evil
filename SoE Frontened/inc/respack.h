#ifndef __RESPACK_H__
#define __RESPACK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_ENTRIES 1000
#define MAX_FILE_LENGTH   256

int create_archive(const char* list_file, const char* pak_file);
int extract_file(const char* pak_file, const char* ex_file, const char* file_out);

#endif