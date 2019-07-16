// ------------ Sys Wide Libraries ------------ //
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// ------------ Project Wide Headers ------------ //
#include "const.h"
#include "errorHandler.h"
#include "httpHandler.h"
#include "log/log.h"


// ------------ Global Vars ------------ //

// Integers
int isCacheEnable   = 0;
int fakeFd;
int child           = 0;
int port            = 8080;									// Default Port
int socketId;                                               // Server Socket ID
int newSocket;
socklen_t client_len;
int cacheDim;
int thresh;
int conf;
int MAX_CLIENTS     = 100000;

// Chars
char *temp_directory;
char *base_directory;                                       // Base directory of server
char msg[255];                                              // Message buffer for the log

// Specific
key_t key;                                                  // Key for loggin queue
mbInfoPtr mobile_info;                                      // mobile_info struct pointet
struct sockaddr_in server_addr, client_addr;                // struct specific for Berkeley C Networking API

// Custom Struct
struct pthread_param{
    int socket;
    char ip_address[16];
    pthread_t tid;
};

// ------------ Functions Prototypes  ------------ //
char *getUAC(char *shadowCopy);
void respondClient(void* _params);
void initMsgQueue();
void checkArgs(int argc, char *argv[]);
void initServer(int argc, char *argv[]);
void startAccept();