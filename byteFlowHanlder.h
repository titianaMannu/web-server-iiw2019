//
// Created by bbqualcosa on 5/7/19.
//
// ------------ Sys Wide Libraries ------------ //
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <errno.h>

// ------------ Project Wide Headers ------------ //
#include "utils.h"
#include "errorHandler.h"
#include "const.h"
#include "log/log.h"


int sendFile(int socket, int fd, char *path, int isHeadOnly);
int sendDirectory(int socket, char *path, char *dir_path, int isHeadOnly);
