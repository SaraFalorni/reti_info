#ifndef GENERALFUNCTIONS_H
#define GENERALFUNCTIONS_H

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "QAfunctions.h"
#include "rankingsFunctions.h"
#include "PlayersFunctions.h"
#include "gameFunctions.h"

 void initSession();
 void initQuizThemes();
 void initThemePrompt(int numTheme);
 void showOverview();
 void initClientSet(struct ClientSet* set);//creazione dei client set per gestire client multipli
 bool assignClientToSet(int client_fd, struct ClientSet* sets, int* numSets);
 void* clientHandler(void* arg);

#endif
