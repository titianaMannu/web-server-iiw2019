#include "log.h"
#include "get_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#define MSGSIZE  255

int LOG_ACTIVE = 0;
key_t msqKey;


struct msgbuf {
    long msqtype;
    char mtext[MSGSIZE];
};

/**
 * used to activate the log
 * @param val
 */
void setLogActive(int val){
    LOG_ACTIVE = val;
}

void setmsqKey(key_t key){
    msqKey = key;
}

key_t getmsqKey(){
    return msqKey;
}

void checkLog(){
    printf("---->%d\n", LOG_ACTIVE);
}

/**
 * If log is active call send_to_msg to send a mesage to the message queue.
 * @param key messagge queue key
 * @param procType  tell us the type of the process which asks for log
 * @param msgType
 * @param ip_addr_client
 * @param text  messagge to write on the log file
 */
void serverLog(key_t key, int procType, int msgType, char *ip_addr_client, char *text){
    if (LOG_ACTIVE){
        send_msg_to_queue(createLogText(procType, msgType, ip_addr_client, text), key);
    }
 //
}

/**
 * Log format inspired by Apache Log
 * @param procType
 * @param msgType messagge queue key
 * @param ip_addr_client tell us the type of the process which asks for log
 * @param text  messagge to write on the log file
 * @return a formatted mesage to write on Log file
 */
char *createLogText(int procType, int msgType, char *ip_addr_client, char *text) {

    char *logText, *time, procTypeS[10], msgTypeS[10];

    memset(procTypeS,0,10);
    memset(msgTypeS,0,10);

    logText = (char *) malloc(MSGSIZE);
    if (logText == NULL) {
        perror("Error in malloc");
        exit(EXIT_FAILURE);
    }
    memset(logText,0,MSGSIZE);

    time = (char *) malloc(MSGSIZE);
    if (time == NULL) {
        perror("Error in malloc");
        exit(EXIT_FAILURE);
    }

    // process type
    switch(procType){
        case 0:
            strcpy(procTypeS,"Core");
            break;
        case 1:
            strcpy(procTypeS,"Worker");
            break;
    }

    // message type
    switch(msgType){
        case 0:
            strcpy(msgTypeS,"Info");
            break;
        case 1:
            strcpy(msgTypeS,"Warning");
            break;
        case 2:
            strcpy(msgTypeS,"Error");
            break;
        case 3:
            strcpy(msgTypeS,"Critical");
            break;
    }

    //  [Fri Sep 09 10:42:29.902022 2011] [core:error] [pid 35708:tid 4328636416] [client 72.15.99.187] File does not exist: /usr/local/apache2/htdocs/favicon.ico
    //  [time] [childProcess: logType] [pid xxxxxx: tid yyyyyyy] [ client: xxx.xxx.xxx.xxx] log_message
    //  [%s] [
    time = _get_time();
    sprintf(logText, "[%-18s]\t[%-6s:%-8s]\t[tid %lu]\t[origin:%-15s]\t%s\t\t\n",
            time,
            procTypeS,
            msgTypeS,
            pthread_self(),
            ip_addr_client,text);
    //DEBUG
    printf("[%-18s]\t[%-6s:%-8s]\t[tid %lu]\t[origin:%-15s]\t%s\t\t\n",
            time,
            procTypeS,
            msgTypeS,
            pthread_self(),
            ip_addr_client,text);

    free(time);
    return logText;
}

/**
 * This is a server function to extract messages from queue and to write them into the log file
 * @param key messagge queue key
 * @return void *
 */
void *logger_routine(void *key){
    int msqidint = initialize_queue((intptr_t) key);    //queue opening
    struct msgbuf *_msgbuffer = malloc(sizeof(struct msgbuf));
    if(_msgbuffer==NULL){
        perror("ERROR:");
        exit(1);
    }


    int fOp =  open("log.txt", O_WRONLY | O_APPEND | O_CREAT, 0666); //opens file in append mode if exists else creates a new one
    if (fOp==-1){
        perror("file could not be opened");
        exit(EXIT_FAILURE);
    }

    while(1){
        //take a message from queue
            memset(_msgbuffer->mtext,0,MSGSIZE);
            if(msgrcv(msqidint, _msgbuffer, MSGSIZE, 0, MSG_NOERROR)){
               int byte_wrote = write(fOp, _msgbuffer->mtext, strlen(_msgbuffer->mtext));//print on file log.txt
        }
    }

}

/**
 * If doesn't exist any queue it creates a new one; else return the queue id
 * @param key messagge queue key
 * @return queue id
 */
int initialize_queue(key_t key) {
    int msqid = msgget(key, IPC_CREAT | 0666);
    if (msqid == -1){
        perror("error in msgget");
        exit(EXIT_FAILURE);
    }
    return msqid;
}


/**
 * This function sends a message on the queue.
 * In this way we have non blocking log writing non blocking
 * @param text messagge to write on the log file
 * @param key messagge queue key
 */
void send_msg_to_queue(char *text, key_t key) {
    int msqid = initialize_queue(key); //get the queue id

    struct msgbuf *messagebuffer = malloc(sizeof(struct msgbuf));
    memset(messagebuffer->mtext,0,MSGSIZE);
    strcpy(messagebuffer->mtext,text);
    messagebuffer->msqtype = 1;

    int res = msgsnd(msqid, messagebuffer, strlen(text), 0); //send a message on the queue
    if (res == -1){
        perror("error in msgsnd");
        exit(EXIT_FAILURE);
    }
}