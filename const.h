//
// Created by bbqualcosa on 5/7/19.
//


#define MAX_BYTES 4096
//#define MAX_CLIENTS 100000


/*

    message[1] = strtok(NULL, " \t\n");                    // stores request file path
    message[2] = strtok(NULL, " \t\n");                    // stores HTTP version
if (message[2] != NULL) {
    if (message[2] == NULL) {
        message[2] = malloc(10);
        bzero(message[2], 10);
        strcpy(message[2], "HTTP/1.1\r");
    }
    if (strlen(message[2]) && checkHTTPVersion(message[2]) == 1)
        bytes_send = handleHEADrequest(params->socket, message[1], base_directory, key,
                                       params->ip_address);        // Handle HEAD request

    else
        sendErrorMessage(params->socket, 505, key, params->ip_address);    // Incorrect HTTP version
} else {
    bytes_send = handleHEADRequest(params->socket, message[1], base_directory, key,
                                   params->ip_address);
}*/



