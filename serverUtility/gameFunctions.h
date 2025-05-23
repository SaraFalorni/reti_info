#ifndef GAMEFUNCTIONS_H
#define GAMEFUNCTIONS_H

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "QAfunctions.h"
#include "generalFunctions.h"
#include "PlayersFunctions.h"
#include "rankingsFunctions.h"

void getNickname(int client_fd, char* name);
void sendThemes(int client_fd, char* nickname) ;
int recvThemes(int client_fd, char* nickname);

void sendQuestion(struct ClientInfo* client);
void recvCommand(struct ClientInfo* client);
void recvResponse(struct ClientInfo* client);

void playQuiz(int themeChosen, char* nickname, int client_fd) ;

int updatePoints(int numq,char* bufR,int themeChosen,char* nickname) ;
int checkCommand(int client_fd,char* msg);
void doShowScore(int client_fd) ;
void doEndquiz(int client_fd);
uint32_t recvAllBytes(int client_fd, void *buf, uint32_t len);
uint32_t sendAllBytes(int client_fd, void *buf, uint32_t len);
uint32_t recvStringLen(int client_fd);
int sendString(int client_fd, void *buf);

#endif
