#ifndef UTILITY_H
#define UTILITY_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h> //toupper()
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>

#define MAXCHAR_LINE 1024
#define MAXCHAR_NICKNAME 20
#define BACKLOG 5
#define MSG_LEN 3
#define MSG_OK "OK"
#define MSG_NO "NO"
#define MSG_RK "RK" //messaggio che indica che il client richiede show score
#define SHOWSCORE "show score"
#define ENDQUIZ "endquiz"
#define MSG_EX "EX" //messaggio che indica che il client richiede endquiz
#define NUM_Q 5 //numero di domande per ogni tema
#define NUM_SEPARATOR 50

#endif
