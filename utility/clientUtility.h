#include "utility.h"

#ifndef CLIENTUTILITY_H
#define CLIENTUTILITY_H

#include <signal.h>
#include <errno.h>

int showMainMenu(int client_fd,char* nickname);
void chooseNickname(int client_fd,char* nickname);
void endGame(int client_fd,char* nickname);
void showQuizThemes(int client_fd);
void playGame(int client_fd,char* nickname);
void showScore(int client_fd);
int checkComand(int client_fd,char* risp,char* nickname);
void sendAnswer(int client_fd,char* risp);
void exitGame(int client_fd);
uint32_t recvAllBytes(int client_fd, void *buf,uint32_t len);
uint32_t sendAllBytes(int client_fd, void *buf, uint32_t len);
char* recvString(int client_fd, void *buf);
int sendString(int client_fd, void *buf);
void removeSpaces(char* str);
void* safeMalloc(int size);

#endif
