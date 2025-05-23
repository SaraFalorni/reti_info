#ifndef PLAYERSFUNCTIONS_H
#define PLAYERSFUNCTIONS_H

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "QAfunctions.h"
#include "generalFunctions.h"
#include "rankingsFunctions.h"
#include "gameFunctions.h"

struct Player* findLastPlayer(struct Player* p);
int insertPlayer(char* nickname,int client_fd) ;
struct Player* getPlayer(struct Player* current_player,char* nickname) ;
struct Player* getPlayerIndex(struct Player* current_player,int index) ;
struct Player* copyPlayer(struct Player* current_player,int index_theme,int client_fd) ;
void deletePlayer(char* nickname) ;

#endif
