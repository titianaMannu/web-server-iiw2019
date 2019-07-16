#ifndef CACHE_IMAGE_MDF_H
#define CACHE_IMAGE_MDF_H

#endif //CACHE_IMAGE_MDF_H

#include "../deviceDetection/51DegreesWorker.h"

int refactor_quality(char *q_string);
void refactor_photo(char *image_path, char *new_path, int q, mbInfoPtr mobile_info);
void replace_char(char *source, char *dest, char c_torepl, char new_c);