#include <zconf.h>

#ifndef CACHE_LOG_H
#define CACHE_LOG_H

#endif //CACHE_LOG_H



void send_msg_to_queue(char *text, key_t key);
int initialize_queue(key_t key);
void *logger_routine(void *key);
char *createLogText(int procType, int msgType, char *ip_addr_client, char *text);
void serverLog(key_t key, int procType, int msgType, char *ip_addr_client, char *text);
void setLogActive(int val);
void checkLog();
void setmsqKey(key_t key);
key_t getmsqKey();