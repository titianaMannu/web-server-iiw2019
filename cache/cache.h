#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "image_mdf.h"
#include "../byteFlowHanlder.h"

#ifndef CACHE_CACHE_H
#define CACHE_CACHE_H

#endif //CACHE_CACHE_H


int find_file_in_cache(char *filename, char *quality_factor, mbInfoPtr mobile_info);
char *update_image_quality(char *quality_factor, char *fullPath, mbInfoPtr mobile_info);
int remove_cache_file();
void cache_main(int fd, char *quality_factor, int socket, char *fullPath, mbInfoPtr mobile_info);
char *first_file(char *filepath, int line);
int file_line_num(char *filepath);
void setCacheDim(int dim);
void checkCacheSize();
int getCacheDim();