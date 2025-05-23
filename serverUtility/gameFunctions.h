#ifndef GAMEFUNCTIONS_H
#define GAMEFUNCTIONS_H

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "QAfunctions.h"
#include "generalFunctions.h"
#include "PlayersFunctions.h"
#include "rankingsFunctions.h"

int getNickname(int client_fd, char* name);
int sendThemes(int client_fd, char* nickname) ;
int recvThemes(int client_fd, char* nickname);

int sendQuestion(struct ClientInfo* client);
int recvCommand(struct ClientInfo* client);
int recvResponse(struct ClientInfo* client);

void playQuiz(int themeChosen, char* nickname, int client_fd) ;

int updatePoints(int numq,char* bufR,int themeChosen,char* nickname) ;
int checkCommand(struct ClientInfo* client,char* msg);
int doShowScore(struct ClientInfo* client) ;
void doEndquiz(struct ClientInfo* client);
uint32_t recvAllBytes(int client_fd, void *buf, uint32_t len);
uint32_t sendAllBytes(int client_fd, void *buf, uint32_t len);
uint32_t recvStringLen(int client_fd);
int sendString(int client_fd, void *buf);

#endif
