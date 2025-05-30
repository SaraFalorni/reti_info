#ifndef SERVERUTILITY_H
#define SERVERUTILITY_H

#include "utility.h"

#define SERVER_IP "127.0.0.1"
#define NUM_Q 5 //numero di domande per ogni tema
#define NUM_THEMES 5 //numero di temi disponibili 
#define MAX_CLIENT_IN_THREAD 15
#define MAX_CLIENTSETS 15

enum ClientState {WaitingForNickname, WaitingForTheme, PlayingQuiz};

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

struct Buffer {
  char* buffer; //contenitore per la stringa
  int totLen;//lunghezza totale da ricevere o mandare
  int progress; //numero di bytes già mandati o già ricevuti
};

//informazioni utili per ricostruire lo stato del client per permettere il suo avanzamento
struct ClientInfo {
  int client_fd; //socket, identificativo del client
  char* nickname; //per fare accopiamento con il player corrispondente
  int currentTheme; //indice del tema corrente, inizializzato a -1. Fa riferimento all'indice di currentSession.availableThemes
  int currentQ; //indice della domanda corrente, inizializzato a -1. Fa riferimento all'indice di currentSession.availableThemes[currentTheme].quiz
  bool isResponding; //se true il server sta aspettando la risposta dal client
  struct Buffer sendBuf;
  struct Buffer recvBuf;
  enum ClientState state; //stato in cui si trova il client 
};

struct ClientSet {
  struct ClientInfo clients[MAX_CLIENT_IN_THREAD];
  int numClients;
  bool FDUpdateNeeded; //se true va aggiornato l'insieme di socket per select
  pthread_t thread;
};

// mutex per players
extern pthread_mutex_t lockPlayers;
//mutex per i set
extern pthread_mutex_t lockSets;
//nuova sessione di gioco
extern struct Session current_session; 

#endif

