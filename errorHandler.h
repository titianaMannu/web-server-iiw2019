// ------------ Sys Wide Libraries ------------ //
#include <bits/types/time_t.h>
#include <time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

// ------------ Project Wide Headers ------------ //
#include "log/log.h"

// ------------ Functions Prototypes  ------------ //
int sendErrorMessage(int socket, int status_code, key_t key, char *client_addr);
