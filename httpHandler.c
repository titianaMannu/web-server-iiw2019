#include "httpHandler.h"

/**
 * Checks the HTTP Version
 * @param msg
 * @return
 */
int checkHTTPVersion(char *msg) {
    int version = -1;

    if (strncmp(msg, "HTTP/1.1", 8) == 0) {
        version = 1;
    } else if (strncmp(msg, "HTTP/1.0", 8) ==
               0)                // Server can also handle 1.0 requests in the same way as it does to handle 1.1 requests
    {
        version = 1;                                        // Hence setting same version as 1.1
    } else
        version = -1;

    return version;
}

/**
 * Check the Method Name
 * @param msg
 * @return
 */
int requestType(char *msg) {
    int type = -1;

    if (strncmp(msg, "GET\0", 4) == 0)
        type = 1;
    else if (strncmp(msg, "POST\0", 5) == 0)
        type = 2;
    else if (strncmp(msg, "HEAD\0", 5) == 0)
        type = 3;
    else
        type = -1;

    return type;
}


/**
 * Handles the GET Requests
 * @param socket            client socket
 * @param msg               http message
 * @param base_directory    base_directory of the webServer
 * @param key               Key for the Log Queue
 * @param client_addr       client_ip_addres
 * @param isCache           check if it can be cached or not
 * @param fdLock            fileDescriptor for the Lock file
 * @param quality_factor    string wich represents the "q" paramaters value
 * @param userAgent         UserAgent String
 * @return
 */
int handleGETRequest(int socket, char *msg, char *base_directory, key_t key, char *client_addr, int isCache, int fdLock,
                     char *quality_factor, char *userAgent) {

     // Fix for the encoding of " " in html
     if(strstr(msg,"%20")){
        msg =  replaceWord(msg,"%20"," ");
         printf("File: %s",msg);
     }


    char file[strlen(msg)];
    bzero(file, sizeof(file));
    if (strcmp(&msg[strlen(msg) - 1], "\r") == 0) {

        strncpy(file, msg, strlen(msg) - 1);                    // copy the entire message to the "file" var without the "\r"
        msg = file;
    } else {
        strncpy(file, msg, strlen(msg));                        // copy the message to to the "file" var
    }

    char file_path[500];
    char dir_path[500];
    char abs_path[500];
    bzero(dir_path, sizeof(dir_path));
    bzero(file_path, sizeof(file_path));
    bzero(abs_path, sizeof(abs_path));

    int fd;                                                                 // File descriptor

    int bytes_send;

    if (strlen(msg) == 0 || strncmp(file, "/", 1) != 0)                     // Error
    {
        sendErrorMessage(socket, 400, key, client_addr);                    // 400 Bad Request
        return 1;
    }

    if (strlen(msg) == 1)                                                   // Default file open index.html
    {
        strcpy(file_path, base_directory);
        strcat(file_path, "/index.html");
    } else {
        strcpy(file_path, base_directory);                  // concatenate requested file name in base_directory
        strcat(file_path, msg);
        strcpy(abs_path, base_directory);                   // concatenate requested file name in base_directory
        strcat(abs_path, msg);
        strcpy(dir_path, msg);
    }


    char getMessage[255];
    memset(getMessage, 0, 255);
    sprintf(getMessage, "Requested File: %s", file_path);
    serverLog(key, 1, 0, client_addr, getMessage);


    struct stat s;
    if ((stat(file_path, &s) == 0 && S_ISDIR(s.st_mode)))                       // if the give File Path is a directory
    {
        bytes_send = sendDirectory(socket, file_path, dir_path, 0);             // Send directory links

        return bytes_send;
    }

    fd = open(file_path, O_RDONLY);                                             // Otherwise open requested file
    if (fd == -1) {
        if (errno == EACCES) {
           sendErrorMessage(socket, 403, key, client_addr);                    // Permission Denied
            return 1;
        } else {
           sendErrorMessage(socket, 404, key, client_addr);                    // File not found
            return 1;
        }
    }

    char *fileExt = NULL;
    if (isCache == 1) {
        fileExt = get_filename_ext(file_path);
        for (int i = 0; fileExt[i]; i++) {
            if (isalpha(fileExt[i])) {
                fileExt[i] = tolower(fileExt[i]);
            }
        }

    }

    if (isCache == 0 || strlen(msg) == 1) {                     //we don't need caching for index.html (case strlen(msg)=1
        bytes_send = sendFile(socket, fd, file_path, 0);        // Send file content
    } else if (// ------------ Content Adaptation  ------------ //
                  isCache == 1 &&
               (  strcmp(fileExt, "jpg") == 0 || strcmp(fileExt, "jpeg") == 0
               || strcmp(fileExt, "png") == 0 || strcmp(fileExt, "tiff") == 0)
               ) {

        mbInfoPtr mbInfo = getUserAgentProperties(userAgent);
        cache_main(fdLock, quality_factor, socket, abs_path, mbInfo);
    } else {
        bytes_send = sendFile(socket, fd, file_path, 0);
    }
    close(fd);                                                // Close file

    return bytes_send;

}
/**
 * Handles the HEAD Requests
 * @param socket            client socket
 * @param msg               http message
 * @param base_directory    base_directory of the webServer
 * @param key               Key for the Log Queue
 * @param client_addr       client_ip_addres
 * @return
 */
int handleHEADRequest(int socket, char *msg, char *base_directory, key_t key, char *client_addr) {
    // Fix for the encoding of " " in html
    if(strstr(msg,"%20")){
        msg =  replaceWord(msg,"%20"," ");
        printf("File: %s",msg);
    }

    char file[strlen(msg)];
    bzero(file, sizeof(file));
    if (strcmp(&msg[strlen(msg) - 1], "\r") == 0) {

        strncpy(file, msg, strlen(msg) - 1);                // copy the entire message to the "file" var without the "\r"
        msg = file;
    } else {
        strncpy(file, msg, strlen(msg));                    // copy the entire message to the "file" var
    }

    char file_path[500];
    char dir_path[500];
    bzero(dir_path, sizeof(dir_path));
    bzero(file_path, sizeof(file_path));

    int fd;                                                                 // File descriptor

    int bytes_send;

    if (strlen(msg) == 0 || strncmp(file, "/", 1) != 0)                     // Error
    {
        //    printf("message Error!");
        sendErrorMessage(socket, 400, key, client_addr);                    // 400 Bad Request
        return 1;
    }

    if (strlen(msg) == 1)                                                   // Default file open index.html
    {
        strcpy(file_path, base_directory);
        strcat(file_path, "/index.html");
    } else {
        strcpy(file_path, base_directory);                                  // concatenate requested file name in base_directory
        strcat(file_path, msg);
        strcpy(dir_path, msg);
    }


    char getMessage[255];
    memset(getMessage, 0, 255);
    sprintf(getMessage, "Requested HEADER for file: %s", file_path);
    serverLog(key, 1, 0, client_addr, getMessage);

    struct stat s;
    if ((stat(file_path, &s) == 0 && S_ISDIR(s.st_mode)))                       // Given File Path is a directory
    {
        //  printf("Send directory links\n");
        bytes_send = sendDirectory(socket, file_path, dir_path, 1);             // Send directory links

        return bytes_send;
    }

    fd = open(file_path, O_RDONLY);                                             // Otherwise open requested file

    if (fd == -1) {
        if (errno == EACCES) {
            perror("Permission Denied\n");
            sendErrorMessage(socket, 403, key, client_addr);                    // Permission Denied
            return 1;
        } else {
            perror("File does not exist HEAD\n");
            sendErrorMessage(socket, 404, key, client_addr);                    // File not found
            return 1;
        }
    }

    bytes_send = sendFile(socket, fd, file_path, 1);                            // Send file's content

    close(fd);                                                                  // Close file

    return bytes_send;

}

/**
 * Auxiliary function which extarcts from the file name it's extension
 * @param filename string
 * @return file extension chars vector
 */
char *get_filename_ext(const char *filename) {
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

/**
 * Auxiliary function to check and handle http encodings
 * @param s         original string
 * @param oldW      oldWord
 * @param newW      newWorld
 * @return          pointer tu the new string
 */
char *replaceWord(const char *s, const char *oldW,
                  const char *newW)
{
    char *result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);

    // Counting the number of times old word
    // occur in the string
    for (i = 0; s[i] != '\0'; i++)
    {
        if (strstr(&s[i], oldW) == &s[i])
        {
            cnt++;

            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }

    // Making new string of enough length
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);

    i = 0;
    while (*s)
    {
        // compare the substring with the result
        if (strstr(s, oldW) == s)
        {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }

    result[i] = '\0';
    return result;
}