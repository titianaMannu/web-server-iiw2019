#include <string.h>
#include <bits/types/time_t.h>
#include <time.h>
#include <sys/socket.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>

#define DELTA 10



char* getContentType(char *path);
int sendHeaderMessage(int socket, char *head, char *media, int file_size);

// maxpid ( cat /proc/sys/kernel/pid_max ) - ( current_pids_number ( ps -a ) + current_child_number + k)
long getSpawnableProcesses(int current_child_number);
long maxNumberOfProcesses();
long numberOfCurrentThreads();
int isStringADigit(char *string);
long numberOfCurrentThreads();