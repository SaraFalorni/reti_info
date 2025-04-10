#include "utility.h"

#ifndef CLIENTUTILITY_H
#define CLIENTUTILITY_H

void showMainMenu(int client_fd,char* nickname);
void chooseNickname(int client_fd,char* nickname);
void endGame(int client_fd,char* nickname);
void showQuizThemes(int client_fd);
void playGame(int client_fd,char* nickname);
void showScore(int client_fd);
bool checkComand(int client_fd,char* risp,char* nickname);
void send_answer(int client_fd,char* risp);
void exitGame(int client_fd);
int recv_all_bytes(int socket, void *buf, int len);

#endif
