#include <stdio.h>
#include <fcntl.h>
#include "cache.h"
int CACHE_DIM = 15;
static char *CACHE_DIR_PATH = "cache_dir"; //executable is in cmake_build_debug;  adjust this value on error
char *get_filecache_path(char *file_path, float quality_factor, mbInfoPtr mobile_info);

char *fileLine(int lineNumber);

void setCacheDim(int dim){
    CACHE_DIM = dim;
}

int getCacheDim(){
    return CACHE_DIM;
}

void checkCacheSize(){
    printf("--->cache size %d", CACHE_DIM);
}


/**
 * take the filename of a photo
 * return a file descriptor if the file is in cache
 * -1 if it isn't
 * @param filename
 * @return file descriptor
 */
int find_file_in_cache(char *filename, char *quality_factor, mbInfoPtr mobile_info) {
    char *file_path;
    file_path = get_filecache_path(filename, strtof(quality_factor, NULL), mobile_info);
    int fd = open(file_path, O_RDWR | O_APPEND, 0666);
    if (fd == -1){
        //file not exists in cache
        perror("file not found in cache");
    }else{
        if(write(fd, "\0", 1)==-1){
            perror("ERROR");
        } // add at the end of the file "EOF" (mock modification to implement LRU)
    }
    close(fd);
    fd = open(file_path, O_RDONLY, 0666);
    return fd;
}

/**
 * take the requested quality factor and modify the image using functions declared in image_mdf.h
 * if cache is full we need to call remove_cache_file
 * @param quality_factor
 * @return pathname of the new file
 */
char *update_image_quality(char *quality_factor, char *fullPath, mbInfoPtr mobile_info) {
    int q;
    char *new_path;

    q = refactor_quality(quality_factor); //get an integer factor quality requested from MagicWand
    new_path = get_filecache_path(fullPath, strtof(quality_factor, NULL), mobile_info); //get new file_cache path

    refactor_photo(fullPath, new_path, q, mobile_info);
    return new_path;
}

/**
 * removes the least modified file into the cache
 * return 0 on success and -1 if not
 * we use LRU (Least Recently Used) policy to find the right file to remove
 * Implementation: we use the bash command <ls -t -r > files.txt> to obtain a list of all file on the cache directory.
 * The returned list is sort by modification time, oldest first.
 *
 */
int remove_cache_file(){
    char *file_to_rm;
    char filename[20] = "files.txt";
    char *command = malloc(100);
    char *cache_path = malloc(100);

    if (cache_path == NULL || command == NULL){
        perror("Unable to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    // returns the path of our cache. In fact we want a list of the files in that directory.
    cache_path = get_filecache_path(NULL, 0, NULL);

    sprintf(command, "ls %s -t -r > %s", cache_path, filename); //fill a file with a list of files already in cache
    if (system(command) == -1){
        //error in file system
        perror("");
        return -1;
    }

    int count = file_line_num(filename); //get how much files we have in cache
    if (count < CACHE_DIM){
        //if cache isn't full we don't need to delete anything
        free(command);
        free(cache_path);
        return 0;
    }

    //Continues removing
    int line = 0;
    while (count >= CACHE_DIM) {
        file_to_rm = first_file(filename, line); // removes least recent file

        if (file_to_rm == NULL) {
            //cahce is empty nothing to delete
            perror("there isn't any file in the directory");
            return -1;
        }

        file_to_rm[strlen(file_to_rm) - 1] = '\0';
        sprintf(command, "rm -f \"%s/%s\"", cache_path, file_to_rm);
        if (system(command) == -1) { //execution of the command
            //error in file system
            perror("");
            return -1;
        }
        line++;
        count--;
    }
    free(command);
    free(cache_path);
    //least recently used file removed! Success
    return 0;
}


/**
 * on success returns the filepath of the requested file, NULL if not
 * if it hasn't already been in cache we need to call update_image_quality
 * @param filename
 * @param fd
 * @return
 */
void cache_main(int fd, char *quality_factor, int socket, char *fullPath, mbInfoPtr mobile_info) {
    int cache_fd;
    char *new_path;

    struct flock lock= {};
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_type = F_RDLCK;
    if (fcntl(fd, F_SETLKW, &lock)){ //ask for a shared lock.
        exit(-3);
    }
    /*critical section start point*/

    cache_fd = find_file_in_cache(fullPath, quality_factor, mobile_info);
    if (cache_fd > 0){//cache hit
        char message[256];
        bzero(message,256);
        char *path = get_filecache_path(fullPath, refactor_quality(quality_factor), mobile_info);
        sprintf(message,"Cache Hit at: %s",path);
        serverLog(getmsqKey(),1,1,"server",message);
        sendFile(socket, cache_fd, path, 0);
    }
    /*end of critical section*/
    lock.l_type = F_UNLCK; // vedi se si può fare il trylock su questo
    if ( fcntl(fd, F_SETLKW, &lock) ) { /* unlock */
        exit(-3);
    }

    if (cache_fd == -1){ //cache miss
        /*critical section start point*/
        serverLog(getmsqKey(),1,1,"server","Cache miss!");
        lock.l_type = F_WRLCK; //esclusive lock for writing op.
        if (fcntl(fd, F_SETLKW, &lock)){
            perror("ERRORE NEL WRITE LOCK");
            exit(-3);
        }

        remove_cache_file(); //delete least used file if cache is full
        new_path = update_image_quality(quality_factor, fullPath, mobile_info);
        cache_fd = open(new_path, O_RDONLY | O_APPEND, 0666);
        if (cache_fd == -1 ) {
           exit(EXIT_FAILURE);
        }
        sendFile(socket,cache_fd,new_path,0);
        /*end of critical section*/
        lock.l_type = F_UNLCK;
        if ( fcntl(fd, F_SETLKW, &lock) ) { /* unlock */
           exit(-3);
        }
    }
}


/**Returns the first line of the file.
 *In this case it is the least used file in cache (LRU)
 */
char *first_file(char *filepath, int line) {
    FILE *fPtr;
    size_t len_file = 100;

    char *name = malloc(len_file);
    if (name == NULL){
        perror("Unable to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    fPtr = fopen(filepath, "r");
    if (fPtr == NULL){
        perror("Error in fopen");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i <= line; i++){
        len_file = getline(&name, &len_file, fPtr);
        if ((int) len_file == -1){//file is empty nothing to delete
            perror("there isn't any file in the directory");
            name = NULL;
        }
    }
    fclose(fPtr);
    return name;
}

/**
 * Returns the lines of the file by counting newlines
 * @param filepath
 * @return
 */
int file_line_num(char *filepath){
    FILE *fPtr;
    int chr, lines;
    lines = 0;

    fPtr = fopen(filepath, "r");
    if (fPtr == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    while (!feof(fPtr)){
        chr = fgetc(fPtr);
        if (chr == '\n'){
            lines++;
        }
    }
    fclose(fPtr);
    return lines;
}



/**
 * @param file_path
 * @param quality_factor
 * @return path of the file in cache if it exists; if it doesn't exist we return the cache path.
 */
char *get_filecache_path(char *file_path, float quality_factor, mbInfoPtr mobile_info) {
    static char *mod = "mdf";
    char *newname;
    char buf[4096];
    char *cwd = getcwd(buf, 4096); //get current working directory
    if (file_path != NULL && strcmp(file_path, "") > 0) {
        char *newStr = malloc(strlen(file_path) + 1);
        if (newStr == NULL){
            perror("unable to allocate memory");
            exit(EXIT_FAILURE);
        }
        memset(newStr, 0, strlen(file_path));
        replace_char(file_path, newStr, '/', '_'); // refactoring the file path
        newname = malloc(4096);
        if (newname == NULL) {
            perror("Malloc error:");
        }
        memset(newname, 0, 4096);
        if (strcmp(mobile_info->isMobile, "True") == 0) {  // a mobile device
            sprintf(newname, "%s/%s/%s_%.2f_%s_%s_%s", // cachedirpath/mdf_xxx_height_width_filename.xxx
                    cwd,
                    CACHE_DIR_PATH,
                    mod,
                    quality_factor,
                    mobile_info->pxlHeight,
                    mobile_info->pxlWidth,
                    newStr);
        } else { // cachedirpath/mdf_xxx_filename.xxx not a mobile device
            sprintf(newname, "%s/%s/%s_%.2f_%s", cwd, CACHE_DIR_PATH, mod, quality_factor, newStr);
        }

        free(newStr); //deallocation
    } else {
        //if we have params like NULL or an empty filename this function return oly the cache directory path
        newname = malloc(4096);
        if (newname == NULL) {
            perror("Malloc2 error:");
        }
        memset(newname, 0, 4096);
        sprintf(newname, "%s", CACHE_DIR_PATH);
    }
    return newname;
}
