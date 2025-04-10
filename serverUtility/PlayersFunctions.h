#ifndef PLAYERSFUNCTIONS_H
#define PLAYERSFUNCTIONS_H

#include "utility.h"
#include "../utility/serverUtility.h"
#include "../serverUtility/QAfunctions.h"
#include "../serverUtility/generalFunctions.h"
#include "../serverUtility/rankingsFunctions.h"
#include "../serverUtility/gameFunctions.h"

struct Player* find_last_player(struct Player* p);
bool insert_player(char* nickname) ;
struct Player* get_player(struct Player* current_player,char* nickname) ;
struct Player* get_player_index(struct Player* current_player,int index) ;
struct Player* copy_player(struct Player* current_player,int index_theme) ;
void delete_player(char* nickname) ;

#endif
