#ifndef SERVERUTILITY_H
#define SERVERUTILITY_H

#include "utility.h"

#define SERVER_IP "127.0.0.1"
#define NUM_Q 5 //numero di domande per ogni tema
#define NUM_THEMES 5 //numero di temi disponibili 
#define MAX_CLIENT_IN_THREAD 15
#define MAX_CLIENTSETS 15

struct Player {
    char* nickname;
    int* themePoints;//ogni tema è identificato dall'indice nell'array current_session.availableThemes
    struct Player* next;
    bool* themeCompleted; //true se i-esimo quiz è stato completato
};

struct Prompt {//insieme di domanda e possibile risposte
  char* question; //testo della domanda
  char** answer;//array di possibili risposte giuste
  int numAnswers;//numero di risposte giuste accettabili
};

struct Theme {
  char* name;
  struct Prompt quiz[NUM_Q];
};

struct Session {
  struct Player* players;
  int num_players;
  struct Theme availableThemes[NUM_THEMES];//ogni tema è individuato dall'indice in questo array
  int numThemes;//numero di temi
};

struct ClientSet {
  int clientSockets[MAX_CLIENT_IN_THREAD];
  int numClients;
  pthread_t thread;
};

// mutex per players
extern pthread_mutex_t lockPlayers;
//nuova sessione di gioco
extern struct Session current_session; 

#endif

