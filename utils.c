#include "utils.h"
/**
 * This function gets the content type of the requested file from its extension.
 * @param path the path of the file
 * @return myme type of the file
 */
char* getContentType(char *path)
{
    char *dot = strrchr(path, '.');						// return the address of last '.' found in string
    char * extension;

    if(!dot || dot == path)
        extension = "";
    else
        extension = dot + 1;

    if(strncmp(extension, "html", 4) == 0 || strncmp(extension, "htm", 3) == 0)
        return "text/html";
    else if(strncmp(extension, "txt", 3) == 0)
        return "text/plain";
    else if(strncmp(extension, "jpeg", 4) == 0 || strncmp(extension, "jpg", 3) == 0)
        return "image/jpeg";
    else if(strncmp(extension, "JPEG", 4) == 0 || strncmp(extension, "JPG", 3) == 0)
        return "image/jpeg";
    else if(strncmp(extension, "PNG", 3) == 0 || strncmp(extension, "png", 3) == 0)
        return "image/png";
    else if(strncmp(extension, "gif", 3) == 0)
        return "image/gif";
    else if(strncmp(extension, "tiff", 4) == 0)
        return "image/tiff";
    else if(strncmp(extension, "pdf", 3) == 0)
        return "Application/pdf";
    else
        return "application/octet-stream";

}

/**
 * Sends header of http response message with HTTP 1.1 syntax
 * @param socket descriptor of the socket
 * @param head the first line of HTTP response message
 * @param media the myme-type of file
 * @param file_size dimension of file
 * @return number of bytes sent
 */
int sendHeaderMessage(int socket, char *head, char *media, int file_size)
{
    char keep_alive[] 	  = "\r\nConnection: keep-alive";
    char content_type[]   = "\r\nContent-Type: ";
    char content_length[] = "\r\nContent-Length: ";
    char date[] 		  = "\r\nDate: ";
    char server_name[]	  = "\r\nServer: Catenacci Internauti";
    char new_line[] 	  = "\r\n\r\n";

    char cLength[20];
    snprintf(cLength,sizeof(cLength), "%d",file_size);		// Content Length: convert int to string

    char currentTime[50];
    time_t now = time(0);

    struct tm data = *gmtime(&now);
    strftime(currentTime,sizeof(currentTime),"%a, %d %b %Y %H:%M:%S %Z", &data);	// Get current time

    char *header = (char*)calloc(strlen(head) + strlen(keep_alive) + strlen(content_type) + strlen(media) + strlen(content_length) + strlen(cLength) + strlen(date) + strlen(currentTime) + strlen(server_name) + strlen(new_line) + 20, sizeof(char));

    strcpy(header, head);
    strcat(header, content_type);
    strcat(header, media);
    strcat(header, content_length);
    strcat(header, cLength);
    strcat(header, keep_alive);
    strcat(header, date);
    strcat(header, currentTime);
    strcat(header, server_name);
    strcat(header, new_line);

    int bytes_send = send(socket, header, strlen(header), 0);

    free(header);

    return bytes_send;
}
/**
 * Check if the given string is made of digits
 * @param string the string to check
 * @return 1 if is made of digit, 0 otherwise
 */
int isStringADigit(char *string){
    int lenght = strlen(string)-1;
    for(int j = 0; j<=lenght;j++){
        if(!isdigit(string[j])){
            return 0;
        }
    }
    return 1;
}

/**
 * This methods counts the threads spawned by the main process.
 * @return Current number of thread spawned by the process.
 */
long numberOfCurrentThreads() {
    int ret;
    char buffer[20];
    long current_number_of_threads;
    FILE *file;
    char command[50];

    //Counts number of active thread
    sprintf(command, "ps -T -p %d | wc -l", getpid());
    //printf("%s", command);

    file = popen(command, "r");
    if (file == NULL) {
        perror("Failed to count threads \n" );
        exit(1);
    }

    /* Read the output a line at a time - output it. */
    while (fgets(buffer, sizeof(buffer)-1, file) != NULL) {
        //printf("%s", buffer);
    }

    //convert to long
    current_number_of_threads = strtol(buffer, NULL, 10) - 1; //Doesn't count header line

    ret = pclose(file);
    if (ret == -1){
        perror("File close error\n");
        exit(1);
    }

    return current_number_of_threads;
}


