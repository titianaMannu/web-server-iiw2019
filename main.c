#include "main.h"




//  TODO: finish summary

//  TODO: accept png images FATTO
//  TODO: ( fare tutto lowercase ) FATTO
//  TODO: create log file in MAIN FATTO
//  TODO: pass cache dims trough shell FATTO
//  TODO: rivedere cosa succede nella cache ( trasformre in un while l'eliminazione dei file ) FATTO
//  TODO: rivedere il nome dei file per cartelle diverse ( sostitiire "/" con "_" ) FATTO
//  TODO: make head-request changes like GET CRASHA FATTO
//  TODO: sistemare le recv come vuole LoPresti FATTO
//  TODO: sistemare l'index.html nelle directory listings FATTO
//  TODO: rivedere il LOG ( pid, tid, foormattazione ecc... ) FATTO
//  TODO: %20 -> " " FATTO
//  TODO: Child variables? vogliamo fare un confinment? vogliamo liberarcene? vogliamo farglielo comunque printare? FATTO
//  TODO: Child variable through commadn line FATTO
//  TODO: Code Cleanup ( Comments and codes itself )    FATTO

/**
 * @param argc
 * Main Function
 * @param argv
 * @return int
 */
int main(int argc, char *argv[]) {
    initServer(argc, argv);         // initialize the server
    startAccept();                  // start accept endless loop
    return 0;
}

/**
 * Main endless loop which takes care of the accept
 * procedures like client's sockets creations
 */
void startAccept() {
    while (1) {
        bzero((char *) &client_addr, sizeof(client_addr));                              // Clears struct client_addr
        client_len = sizeof(client_addr);                                               // initialize client_len var

        newSocket = accept(socketId, (struct sockaddr *) &client_addr, &client_len);    // Accepts connection
        if (newSocket < 0) {
            serverLog(key, 0, 3, "server", "Error in accepting connection");
            exit(1);
        }


        //  ------------ Getting client's IP address and port number    ------------ //

        struct sockaddr_in *client_pt = (struct sockaddr_in *) &client_addr;
        struct in_addr ip_addr = client_pt->sin_addr;
        char client_ip_addr[INET_ADDRSTRLEN];                                           // INET_ADDRSTRLEN is the default ip address size
        inet_ntop(AF_INET, &ip_addr, client_ip_addr, INET_ADDRSTRLEN);

        // For each log entry we're zeroing the buffer
        memset(msg, 0, strlen(msg));
        sprintf(msg, "New Client connected with port no.: %d and ip address: %s", ntohs(client_addr.sin_port),
                client_ip_addr);
        serverLog(key, 0, 0, "server", msg);

        // ------------ Initialize of the threads specific paramteres ------------ //

        if (child < thresh) {
            child++;
            pthread_t tid;
            struct pthread_param *param = malloc(sizeof(struct pthread_param));
            param->socket = newSocket;
            strcpy(param->ip_address, client_ip_addr);

            if (pthread_create(&(param->tid), NULL, (void *) respondClient, param)) // Thread spawning
            {
                perror("Error spawning thread");
                exit(EXIT_FAILURE);
            }

        }else {
                close(newSocket);
                while (child > conf) {
                    child = numberOfCurrentThreads();
                    if (child > conf) {
                        sleep(10);
                    }
                }

        }

    }
}

/**
 * Initialization of the server, this function takes care
 * of the comand line argument and the initialization of
 * the Message queue
 * @param argc argument count
 * @param argv argument chars vector
 */
void initServer(int argc, char *argv[]) {
    initMsgQueue();
    //mkdir("cache_dir", 0666); //questo comando è disabilitato altrimenti sono richiesti i privilegi di amministratore!!!
    //system("rm cache_dir/* -f");

    // Default baseDirectory Creation
    base_directory = (char *) malloc(45 * sizeof(char));
    strcpy(base_directory, "webfiles");

    // Applying commadn line settings, if any
    checkArgs(argc, argv);

    // Log message
    memset(msg, 0, 255);
    sprintf(msg, "Setting Server Port : %d and Base Directory: %s", port, base_directory);
    serverLog(key, 0, 0, "server", msg);

    // ------------ Creating the listening socket ------------ //
    socketId = socket(AF_INET, SOCK_STREAM, 0);
    if (socketId < 0) {
        serverLog(key, 0, 3, "server", strerror(errno));
        exit(1);
    }

    int reuse = 1;
    if (setsockopt(socketId, SOL_SOCKET, SO_REUSEPORT, (const char *) &reuse, sizeof(reuse)) <
        0)   // The port used by the server can be used
        serverLog(key, 0, 3, "server",
                  strerror(errno));                                            // immediately it terminates by another
    // instance of itself or another program

    // ------------ Binding socket with given port number   ------------ //
    // ------------ and the server is set to connect with   ------------ //
    // ------------ any ip address on the iternet           ------------ //

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socketId, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        serverLog(key, 0, 3, "server", strerror(errno));
        exit(1);
    }

    // Log message
    memset(msg, 0, 255);
    sprintf(msg, "Binding successful on port: %d", port);
    serverLog(key, 0, 0, "server", msg);


    // ------------ Listening for connections and accept upto MAX_CLIENTS in queue ------------//

    int status = listen(socketId, MAX_CLIENTS);
    if (status < 0) {
        serverLog(key, 0, 3, "server", strerror(errno));
        exit(1);
    }

    // Setting Value of thresholds
    if (MAX_CLIENTS >= 100) {
        thresh = MAX_CLIENTS - 10;
    } else if (MAX_CLIENTS < 100) {
        thresh = MAX_CLIENTS;
    }

    // Setting Value of Confidence
    if (thresh < 50) {
        conf = thresh / 2;
    } else if (thresh >= 50) {
        conf = thresh - 20;
    }
}

/**
 * Checks and apply settings passed through command line
 * @param argc  arguments count
 * @param argv  arguments chars vector
 */
void checkArgs(int argc, char *argv[]) {
    //   ------------ Arguments Checks   ------------  //
    int params = 1; // skips the "/" argument
    // default message if any error is present in the arguments
    const char wrongUsage[90] = "Wrong Arguments! Usage:  [-p PortNumber] [-b BaseDirectory] [--cache-on xyz] [--log-on]";

    for (; params < argc; params++) {

        if (strcmp(argv[params], "-p") == 0) {                  //  PortNumber

            params++;
            if (params < argc) {
                if (isStringADigit(argv[params])) {
                    port = atoi(argv[params]);
                    continue;
                } else {
                    printf("%s", wrongUsage);
                    exit(1);
                }
            } else {
                printf("%s", wrongUsage);
                exit(1);
            }
        } else if (strcmp(argv[params], "--max-clients") == 0) {                  //  PortNumber

            params++;
            if (params < argc) {
                if (isStringADigit(argv[params])) {
                    MAX_CLIENTS = atoi(argv[params]);
                    continue;
                } else {
                    printf("%s", wrongUsage);
                    exit(1);
                }
            } else {
                printf("%s", wrongUsage);
                exit(1);
            }
        } else if (strcmp(argv[params], "--cache-on") == 0) {   //  Cache on/off
            params++;
            if (fakeFd == -1) {
                perror("Error Cache Lock File");
                exit(1);
            }
            serverLog(key, 0, 0, "server", "Cache Enabled!");
            int result = mkdir("cache_dir", 0666);
            if (result == -1 && errno != EEXIST) {
                serverLog(key, 0, 3, "server", "There Was an error creating the Cache Directory, sir!");
                exit(1);
            } else {
                serverLog(key, 0, 1, "server", "Cache Directory already Exists");

            }
            isCacheEnable = 1;
            if (params < argc) {
                if (isStringADigit(argv[params])) {             // if on the size must be specified
                    setCacheDim(atoi(argv[params]));
                    cacheDim = atoi(argv[params]);
                    continue;
                } else {
                    printf("%s", wrongUsage);
                    exit(1);
                }
            } else {
                printf("%s", wrongUsage);
                exit(1);
            }

        } else if (strcmp(argv[params], "--log-on") == 0) {     // Log on/off
            setLogActive(1);
            serverLog(key, 0, 3, "server", "Server Log Activated!");
        } else if (strcmp(argv[params], "-b") == 0) {           // Base directory
            params++;

            if (params < argc) {
                struct stat s;
                if (!(stat(argv[params], &s) == 0 && S_ISDIR(s.st_mode))) {
                    serverLog(key, 0, 3, "server", " No such directory exist!");
                    exit(1);
                }

                temp_directory = argv[params];

                int k = strlen(temp_directory) - 1;

                if (strncmp(&temp_directory[k], "/", 1) == 0)  // Removing / from the last
                    strcpy(&temp_directory[k], "\0");

                char *temp = (char *) realloc(base_directory, sizeof(char) * strlen(temp_directory));
                base_directory = temp;
                strcpy(base_directory, temp_directory);

                continue;
            } else {
                printf("%s", wrongUsage);
                exit(1);
            }
        } else {
            printf("%s", wrongUsage);
            exit(1);
        }

        if (isCacheEnable == 0) {
            serverLog(key, 0, 0, "server", "Cache Disabled!");
            isCacheEnable = 0;
        } else {
            bzero(msg, sizeof(msg));
            sprintf(msg, "Cache On With Size: %d", getCacheDim());
            serverLog(key, 0, 0, "server", msg);
            setmsqKey(key);
        }

    }

    bzero(msg, sizeof(msg));
    sprintf(msg, "Max Client setted at: %d", MAX_CLIENTS);
    serverLog(key, 0, 0, "server", msg);
    setmsqKey(key);
}

/**
 * Initialize Message Queue, it starts the specific thread
 */
void initMsgQueue() {
    // Initialize Queue IDs & Queue Server Thread

    int fd2 = open("log.txt", O_CREAT, 0666);
    if (fd2 < 0) {
        perror("Error creating log file");
        exit(errno);
    }
    close(fd2);

    fakeFd = open("lock", O_RDWR | O_CREAT, 0666);
    if (fakeFd < 0) {
        perror("Error creating lock file");
        exit(errno);
    }


    key = ftok("log.txt", 65); // we use the log.txt file for initialize the message que
    if (key == -1) {           // just for self reference
        perror("error");
    }

    pthread_t tid;
    int ret_tid = pthread_create(&tid, NULL, &logger_routine, (void *) (intptr_t) key); // Thread spawning
    if (ret_tid) {
        perror("ERRORE ");
        exit(1);
    }
}


/**
 * This function allows the thread to handle client requests.
 * @param params struct that passes arguments to the trhead:
 *
 *   struct pthread_param{
 *
 *   int socket
 *
 *   char ip_address[16];
 *
 *   pthread_t tid;
 *
 * };
 *
 */
void respondClient(void *_params) {

    // Recasting parameters to the correct data type
    struct pthread_param *params = (struct pthread_param *) _params;
    // setting thread "global" parameters
    setCacheDim(cacheDim);
    // Setting RECV Timeout
    struct timeval timeval;
    timeval.tv_sec = 30;
    int result = setsockopt(params->socket, SOL_SOCKET,SO_RCVTIMEO, (const char*)&timeval,sizeof(timeval));
    if(result ==-1){
        perror("Cannot set RECV Timeout ");
        pthread_exit((void*)1);
    }

    // Log Message
    memset(msg, 0, strlen(msg));
    sprintf(msg, "Thread %d Created with TID = %lu", child, pthread_self());
    serverLog(key, 0, 0, "server", msg);

    int bytes_send;                                                         // Bytes Transferred
    char buffer[MAX_BYTES];                                                 // Creating buffer of 4kb for a client
    bzero(buffer, MAX_BYTES);                                               // Zeroing the Buffer
    bytes_send = recv(params->socket, buffer, MAX_BYTES, 0);                // Receive Command

    // Endless loop for managing the reqeusts
    while (bytes_send > 0) {
        // ------------ Content Adaptations Variable ------------ //
        char quality[5];
        char shadowCopy[strlen(buffer)];
        memset(shadowCopy, 0, strlen(buffer));
        memset(quality, 0, 5);
        strcpy(shadowCopy, buffer);

        char *target = getUAC(shadowCopy); // UserAgentChecks
        char *message[3];

        if (strlen(buffer) > 0) {
            // Routine to get q= parameters
            char *pon = strstr(shadowCopy, "q=");
            if (pon != NULL) {
                char qualityChar[4];
                strncpy(qualityChar, pon + 2, 4);
                sprintf(quality, "%.2f", strtof(qualityChar, NULL)); //get quality from request*

            } else {
                sprintf(quality, "1.0");
            }


            message[0] = strtok(buffer, " \t\n");                    // stores Request Method
            int type = requestType(message[0]);
            if (type == 1)                                              // GET Request
            {
                message[1] = strtok(NULL, " \t\n");                     // stores request file path
                message[2] = strtok(NULL, " \t\n");                     // stores HTTP version if specified
                if (message[2] == NULL || !strstr(message[2], "HTTP")) { // TODO: check if alters something
                    message[2] = malloc(10);
                    bzero(message[2], 10);
                    strcpy(message[2], "HTTP/1.1\r");                   // defaults to HTTP/1.1
                }

                if (strlen(message[2]) && checkHTTPVersion(message[2]) == 1)
                    bytes_send = handleGETRequest(params->socket, message[1], base_directory, key,
                                                  params->ip_address,
                                                  isCacheEnable,
                                                  fakeFd, quality, target);    // Handle GET request

                else
                    sendErrorMessage(params->socket, 505, key, params->ip_address);    // Incorrect HTTP version
            } else if (type == 2)                                        // POST Request
            {
                sendErrorMessage(params->socket, 501, key, params->ip_address);
            } else if (type == 3)                                        // HEAD Request
            {

                message[1] = strtok(NULL, " \t\n");                         // stores request file path
                message[2] = strtok(NULL, " \t\n");                         // stores HTTP version if specified
                if (message[2] == NULL || !strstr(message[2], "HTTP")) {
                    message[2] = malloc(10);
                    bzero(message[2], 10);
                    strcpy(message[2], "HTTP/1.1\r");                       // defaults to HTTP/1.1
                }

                if (strlen(message[2]) && checkHTTPVersion(message[2]) == 1)
                    bytes_send = handleHEADRequest(params->socket, message[1], base_directory, key,
                                                   params->ip_address);    // Handle HEAD request

                else
                    sendErrorMessage(params->socket, 505, key, params->ip_address);    // Incorrect HTTP version

            } else                                                    // Unknown Method Request
            {
                sendErrorMessage(params->socket, 501, key, params->ip_address);
            }
        } else {
            sendErrorMessage(params->socket, 400, key,
                             params->ip_address);                            // 400 Bad Request
        }

        bzero(buffer, MAX_BYTES);
        bytes_send = recv(params->socket, buffer, sizeof(buffer), 0);        // Recieve Next Request from Cliemt
        // Persistence implementation
    }

    if (bytes_send < 0) { // Error in recieving from client
        memset(msg, 0, strlen(msg));
        sprintf(msg, "Error in receiving from client: %s", params->ip_address);
        serverLog(key, 1, 2, "server", msg);
    } else if (bytes_send == 0) { // Timeout-No bytes recieved
        close(params->socket);
        // Log Message
        memset(msg, 0, strlen(msg));
        sprintf(msg, "Thread %d closed", child);
        serverLog(key, 0, 1, "server", msg);
    }
}

/**
 * This function takes the complete request and extracts The user agent
 * @param shadowCopy: HTTP request string
 * @return user agent string
 */
char *getUAC(char *shadowCopy) {
    // Auxiliary vars
    const char *PATTERN1 = "User-Agent: ";
    const char *PATTERN2 = "\n";
    char *start, *end, *target;

    start = strstr(shadowCopy, PATTERN1);
    if (start) {
        start += strlen(PATTERN1);
        end = strstr(start, PATTERN2);
        if (end) {
            target = malloc(end - start + 1);
            memcpy(target, start, end - start);
            target[end - start] = '\0';
            return target;
        }
    }
}