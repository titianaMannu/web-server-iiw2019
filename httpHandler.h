// ------------ Sys Wide Libraries ------------ //
#include <fcntl.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>

// ------------ Project Wide Headers ------------ //
#include "byteFlowHanlder.h"
#include "cache/cache.h"
#include "errorHandler.h"


// ------------ Functions Prototypes  ------------ //
int checkHTTPVersion(char *msg);
int requestType(char *msg);
int handleGETRequest(int socket, char *msg, char *base_directory, key_t key, char *client_addr,
                     int isCache, int fdLock, char *quality_factor, char *userAgent);
int handleHEADRequest(int socket, char *msg, char *base_directory, key_t key, char *client_addr);
char *get_filename_ext(const char *filename);
char *replaceWord(const char *s, const char *oldW,
                  const char *newW);
