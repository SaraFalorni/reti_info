#include "utility.h"
#include "serverUtility.h"
#include "generalFunctions.h"
#include "PlayersFunctions.h"
#include "rankingsFunctions.h"
#include "gameFunctions.h"

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

//inizializzazione mutex per players
pthread_mutex_t lockPlayers = PTHREAD_MUTEX_INITIALIZER;
//inizializzazione della uova sessione di gioco
struct Session current_session; 
