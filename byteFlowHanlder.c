//
// Created by bbqualcosa on 5/7/19.
//



#include "byteFlowHanlder.h"

/**
 * Function to send file over TCP connections
 * @param socket        client sending socket
 * @param fd            file_to_send file descripto
 * @param path          path to file_to_send
 * @param isHeadOnly    true will send only the Header Information false otherwise
 * @return              result of byte_sent
 */
int sendFile(int socket, int fd, char *path, int isHeadOnly) {

    struct stat st;
    fstat(fd, &st);
    int file_size = st.st_size;                                                 // Get file's size

    char *mediaType = getContentType(path);                                     // Get mime type of the files

    int bytes_send = sendHeaderMessage(socket, "HTTP/1.1 200 OK", mediaType, file_size);

    if (!isHeadOnly) {
        if (bytes_send > 0)                                                     // if Header Message is sent successfully
        {   bytes_send = sendfile(socket, fd, NULL, file_size);                 // send file data
            while (bytes_send < file_size)                                      // If sent data less then file size
            {
                bytes_send = sendfile(socket, fd, NULL, file_size);             // Send again

                if (bytes_send <= 0)                                            // Connection break;
                {
                    bytes_send = sendErrorMessage(socket, 500, 0, NULL);        // Unexpected server error
                    return bytes_send;
                }
            }
        } else {
            bytes_send = sendErrorMessage(socket, 500, 0, NULL);                // Unexpected server error
            return bytes_send;
        }
 }

    return bytes_send;
}

/**
 * Function to send Directory Listings if available
 * @param socket         client sending socket
 * @param path           directory_path
 * @param dir_path       full directory_path
 * @param isHeadOnly     true will send only the Header Information false otherwise
 * @return               result of byte_sent
 */
int sendDirectory(int socket, char *path, char *dir_path, int isHeadOnly) {
    DIR *dir;
    struct dirent *entry;

    char buffer[MAX_BYTES];
    dir = opendir(path);                                                // Open the directory

    int bytes_send;
    int contentLength = 0;

    if (strncmp(&dir_path[strlen(dir_path) - 1], "/", 1) == 0)          // Removes Last forward slash
        strcpy(&dir_path[strlen(dir_path) - 1], "\0");

    char index[strlen(path) + 11];
    bzero(index,sizeof(index));
    strcat(index, path);
    strcat(index, "/index.html");


    if (dir != NULL) {
    int fdIdx = open(index, O_RDWR, 0666);
        if (fdIdx>0) {
            sendFile(socket,fdIdx,index,0);
        }else{
            // ------------ Calculate length of message to be send ------------
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0) continue;
                contentLength += strlen(dir_path) + 2 * strlen(entry->d_name) + 25;    // Calculated the lenght for the header
            }
            contentLength += 110 + strlen(dir_path);
            closedir(dir);

            dir = opendir(path);
            bytes_send = sendHeaderMessage(socket, "HTTP/1.1 200 OK", "text/html", contentLength);


            if (bytes_send > 0)                                                             // if the Header message is sent successfully
            {
                if (!isHeadOnly) {
                    snprintf(buffer, sizeof(buffer),
                             "<HTML><HEAD><TITLE>Directory Links</TITLE></HEAD><BODY><H1>Files in the directory %s</H1><ul>",
                             dir_path);

                    bytes_send = send(socket, buffer, strlen(buffer), 0);

                    if (bytes_send > 0) {
                        while ((entry = readdir(dir)) != NULL) {
                            if (strcmp(entry->d_name, ".") == 0) continue;

                            bzero(buffer, MAX_BYTES);

                            snprintf(buffer, sizeof(buffer), "<li><a href=\"%s/%s\">%s</a></li>", dir_path,
                                     entry->d_name,
                                     entry->d_name);

                            bytes_send = send(socket, buffer, strlen(buffer), 0);           // Send files one by one

                            if (bytes_send <= 0)                                            // Connection is broken
                                break;
                        }
                    } else {
                        bytes_send = sendErrorMessage(socket, 500, 0, NULL);                // Unexpected Error
                        return bytes_send;
                    }

                    bzero(buffer, MAX_BYTES);

                    snprintf(buffer, sizeof(buffer), "</ul></BODY></HTML>");
                    bytes_send = send(socket, buffer, strlen(buffer), 0);

                    closedir(dir);                                                          // Close dir

                    return bytes_send;
                }
            } else {
                closedir(dir);
                bytes_send = sendErrorMessage(socket, 500, 0,
                                              NULL);                                        // Unexpected server error
                return bytes_send;
            }
        }
    } else {
        if (errno == EACCES)                                                                //  Check errno value
        {
            perror("Permission Denied\n");
            bytes_send = sendErrorMessage(socket, 403, 0, NULL);
            return bytes_send;
        } else {
            perror("Directory Not Found\n");
            bytes_send = sendErrorMessage(socket, 404, 0, NULL);                            // Directory Not Found
            return bytes_send;
        }
    }


}


