#ifndef SERVERUTILITY_H
#define SERVERUTILITY_H

#include "utility.h"

#define SERVER_IP "127.0.0.1"

struct Player {
    char* nickname;
    int* themePoints;//ogni tema è identificato dall'indice nell'array current_session.availableThemes
    struct Player* next;
    bool* themeCompleted; //true se i-esimo quiz è stato completato
};

struct Session {
  struct Player* players;
  int num_players;
  char** availableThemes;
  int num_themes;
};

#endif

