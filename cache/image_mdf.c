#include "image_mdf.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <wand/magick_wand.h>

static char *CACHE_DIR_PATH = "cache_dir"; //executable is in cmake_build_debug;  adjust this value on error

/**
 * This function gets the quality string that represents a double with 2 decimal digits
 * and returns an integer representing the quality compress ratio of the image
 * @param q_string string representing the quality, taken from http request
 * @return quality integer between 1 and 100
 */
int refactor_quality(char *q_string) {
    double q;
    int qual;
    char *err;

    errno = 0;
    q = (double) strtod(q_string, &err);
    if (errno != 0) {
        perror("Error in strtod");
        exit(EXIT_FAILURE);
    }

    qual = (int) q * 100;
    return qual;
}

/**
 * This function compresses the photo to a quality factor and, only if the device is a mobile device,
 * resizes the photo.
 *
 * @param image_path path of original photo
 * @param new_path path to save photo in cache
 * @param q quality integer
 * @param mobile_info struct with information of mobile device
 */
void refactor_photo(char *image_path, char *new_path, int q, mbInfoPtr mobile_info) {
    char buf[4096];
    char *cwd = getcwd(buf, 4096); //get current working directory
    MagickWand *mw = NULL;
    MagickBooleanType result;
    MagickWandGenesis();
    mw = NewMagickWand();

    //sprintf(path, "%s/../images/%s", cwd, name);

    // Opens this photo using ImageMagick
    MagickReadImage(mw, image_path);

    // updates photo using ImageMagick
    result = MagickSetImageCompressionQuality(mw, q);

    int result2 = MagickTrue; // if device is not mobile, result2 is true anyway

    // If device is a mobile device, gets information of  screen size.
    if (strncmp(mobile_info->isMobile, "True", 4) == 0) {
        int height = atoi(mobile_info->pxlHeight);
        int width = atoi(mobile_info->pxlWidth);
        result2 = MagickScaleImage(mw, width, height);
        //on_error_log("MagickResizeImage: image_magick_elaboration");
    }

    // Write the modified image in a new path if compression and eventual resizing are successful
    if (result == MagickTrue && result2 == MagickTrue) {
        char path2[90];
        sprintf(path2, "%s", new_path);
        MagickWriteImage(mw, path2);
    } else {
        printf("ERRORE IMAGEMAGICK");
    }

    // Frees memory
    if (mw) {
        mw = DestroyMagickWand(mw);
    }

    MagickWandTerminus();
}
/**
 * This functions replace a char with another in a string.
 * @param source the original string
 * @param dest the new string
 * @param c_torepl character that need to be replaced
 * @param new_c character that will replace c_torepl
 */
void replace_char(char *source, char *dest, char c_torepl, char new_c) {
    for (int i = 0; i < strlen(source); i++) {
        // check if i-th char is equal to c_torepl
        if (source[i] == c_torepl) {
            dest[i] = new_c; // replace it
        } else{
            dest[i] = source[i]; // copy otherwise
        }
    }
    dest[strlen(source)] = '\0';
}