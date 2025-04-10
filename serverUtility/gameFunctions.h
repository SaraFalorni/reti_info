#ifndef GAMEFUNCTIONS_H
#define GAMEFUNCTIONS_H

#include "utility.h"
#include "../utility/serverUtility.h"
#include "../serverUtility/QAfunctions.h"
#include "../serverUtility/generalFunctions.h"
#include "../serverUtility/PlayersFunctions.h"
#include "../serverUtility/rankingsFunctions.h"

void get_nickname(int client_fd, char* name);
void send_themes(int client_fd, char* nickname) ;
void playquiz(int themeChosen, char* nickname, int client_fd) ;
int updatePoints(int numq,char* bufR,int themeChosen,char* nickname) ;
bool check_comand(int client_fd,char* msg);
void do_show_score(int client_fd) ;
void do_endquiz(int client_fd);
int recv_all_bytes(int socket, void *buf, int len);

#endif
