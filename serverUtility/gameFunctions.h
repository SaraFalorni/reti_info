#ifndef GAMEFUNCTIONS_H
#define GAMEFUNCTIONS_H

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "QAfunctions.h"
#include "generalFunctions.h"
#include "PlayersFunctions.h"
#include "rankingsFunctions.h"

int getNickname(struct ClientInfo* client);
int sendThemes(struct ClientInfo* client, char* nickname) ;
int recvThemes(int client_fd, char* nickname);
int sendQuestion(struct ClientInfo* client);
int recvCommand(struct ClientInfo* client);
int recvResponse(struct ClientInfo* client);
int updatePoints(int numq,char* bufR,int themeChosen,char* nickname) ;
int checkCommand(struct ClientInfo* client,char* msg);
int doShowScore(struct ClientInfo* client) ;
int doEndquiz(struct ClientInfo* client);
int recvAllBytes(struct ClientInfo* client, void *buf, uint32_t len);
int sendAllBytes(struct ClientInfo* client, void *buf, uint32_t len);
//int recvStringLen(int client_fd); cancellare
int sendString(struct ClientInfo* client, void *buf);
//int recvString(struct ClientInfo* client, void **buf); cancellare

#endif
