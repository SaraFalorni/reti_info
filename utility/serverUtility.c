#include "serverUtility.h"

//inizializzazione mutex per players
pthread_mutex_t lockPlayers = PTHREAD_MUTEX_INITIALIZER;
struct Session current_session; 

void init_session() {
  current_session.players = NULL;
  current_session.num_players = 0;
  
  current_session.num_themes = quanti_temi();
  
  current_session.availableThemes = (char**)malloc(current_session.num_themes * sizeof(char*));
  
  for(int i = 0; i < current_session.num_themes; i++) {
    char buf[MAXCHAR_LINE];
    get_theme_name(i+1,buf);
    
    size_t len_themeName = strlen(buf)+1;
    current_session.availableThemes[i] = malloc(len_themeName);
    memcpy(current_session.availableThemes[i],buf,len_themeName);
    
  } 
  return;
}

void* client_handler(void* arg) {
  int client_fd = *(int*)arg;
  free(arg); //????
  
  //il primo msg che riceve è il nickname
  char nickname[MAXCHAR_NICKNAME];
  get_nickname(client_fd,nickname);
  while(1) {
  //una volta registrato il nuovo giocatore invia i temi disponibili
  send_themes(client_fd, nickname);
  }
  close(client_fd);
}

struct Player* find_last_player(struct Player* p) {
  if(p == NULL || p->next == NULL)
    return p;
  return find_last_player(p->next);
}

bool insert_player(char* nickname) {
  //inserimento in players con mutex per evitare errori
  pthread_mutex_lock(&lockPlayers);
  
  struct Player* new_player;
  new_player = current_session.players;
  
  /*while(new_player != NULL) {
    if( strcmp(nickname,new_player->nickname)) {
      pthread_mutex_lock(&lockPlayers);
      return false;
    }
    new_player = new_player->next;
  }*/
  
  if(get_player(current_session.players,nickname) != NULL) {
    pthread_mutex_unlock(&lockPlayers);
      return false;
  }
  new_player = (struct Player*)malloc(sizeof(struct Player));
  size_t len_nickname = strlen(nickname)+1;
  new_player->nickname = (char*)malloc(len_nickname);
  memcpy(new_player->nickname,nickname,len_nickname);  
  
  //INIZIALIZZAZIONE DEI TEMI CON POINTS = -1
  
  new_player->theme = (struct Theme*)malloc(current_session.num_themes * sizeof(struct Theme));
  for(int i = 0; i < current_session.num_themes ; i++){
    size_t len_themeName = strlen(current_session.availableThemes[i]) + 1;
    new_player->theme[i].name = (char*)malloc(len_themeName);
    strcpy(new_player->theme[i].name, current_session.availableThemes[i]);
    new_player->theme[i].points = -1;
  }
  new_player->next = NULL;
  current_session.num_players++;
  
  if(current_session.players == NULL)
    current_session.players = new_player;
  else {
    struct Player* last_player = find_last_player(current_session.players);
    last_player->next = new_player;
  }
    
  //sblocco il mutex
  pthread_mutex_unlock(&lockPlayers);
  
  
  //print_session(current_session);
  //giocatore inserito correttamente
  
  
  return true;
}


void get_nickname(int client_fd, char* name) {
  char nickname[MAXCHAR_NICKNAME];
  
  ssize_t bytes_received = recv(client_fd, nickname, MAXCHAR_NICKNAME, 0);
  
  if(bytes_received == -1) {
    perror("Errore in recv() per nickname");
    exit(EXIT_FAILURE);
  }
  
  if(insert_player(nickname)) {   
    //messaggio di ok a client
    if(send(client_fd, MSG_OK, MSG_LEN, 0) == -1) {
      perror("Errore in send() dell'ok al nickname");
      exit(EXIT_FAILURE);
    }    
  }
  else {
    //messaggio non ok al client
    if(send(client_fd, MSG_NO, MSG_LEN, 0) == -1) {
      perror("Errore in send() del no al nickname");
      exit(EXIT_FAILURE);
    } 
  }
  strcpy(name,nickname);
}

void send_themes(int client_fd, char* nickname) {
//manda un messaggio al client con il numero di temi e aspetta un feedback sulla ricezione di quest'ultimo
  char msg[MSG_LEN];
  

  if(send(client_fd, &current_session.num_themes, sizeof(current_session.num_themes), 0) == -1) {
      perror("Errore in send() del numero di temi");
      exit(EXIT_FAILURE);
    }
    ssize_t bytes_received = recv(client_fd, msg, MSG_LEN, 0);
  
   if(bytes_received == -1) {
      perror("Errore in recv() di conferma del numero di temi");
      exit(EXIT_FAILURE);
    }

  if(strcmp(msg,MSG_OK) == 0) {
       
    for(int i = 0; i < current_session.num_themes ; i++) {
      //controllo se il giocatore ha già giocato l'i-esimo tema
      //lock mutex
      pthread_mutex_lock(&lockPlayers);
      struct Player* ptr = get_player(current_session.players,nickname);
      
      //unlock mutex
      pthread_mutex_unlock(&lockPlayers);
      if(ptr->theme[i].points != -1) {
        int len = -1;
      
        if(send(client_fd, &len, sizeof(int),0)== -1) { //invio di -1 per indicare che non è un tema disponibile
          perror("Errore in send() della lunghezza del nome del tema");
          exit(EXIT_FAILURE);
        } 
      } 
      else {
        int len = strlen(current_session.availableThemes[i]) + 1;
      
        if(send(client_fd, &len, sizeof(int),0)== -1) { //invio lunghezza della stringa
          perror("Errore in send() della lunghezza del nome del tema");
          exit(EXIT_FAILURE);
        } 
        if(send(client_fd,current_session.availableThemes[i],len,0)== -1) { 
          perror("Errore in send() del nome del tema");
          exit(EXIT_FAILURE);
        }
      }
    } 
  }
  int themeChosen;
  if(recv_all_bytes(client_fd,&themeChosen,sizeof(int)) <= 0) {
      perror("Errore nella ricezione della stringa");
      exit(EXIT_FAILURE);
    }
  
  playquiz(themeChosen, nickname,client_fd);
}

void playquiz(int themeChosen, char* nickname, int client_fd) {
  char bufQ[MAXCHAR_LINE], bufFile[MAXCHAR_LINE];
  get_filename_from_index(bufFile,themeChosen, &current_session);
  for(int i = 0; i < NUM_Q ; i++) {
      //recupero la domanda dal file
      read_q(bufFile,bufQ,i);
      int lenQ = strlen(bufQ) + 1;
      
      if(send(client_fd, &lenQ, sizeof(int),0)== -1) { //invio lunghezza della domanda
        perror("Errore in send() della lunghezza della domanda");
        exit(EXIT_FAILURE);
      } 
      if(send(client_fd,bufQ,lenQ,0)== -1) { 
        perror("Errore in send() della domanda");
        exit(EXIT_FAILURE);
      }
      
      //riceve la risposta ( sempre ricevendo prima il numero di byte)
      int lenR;
    if(recv_all_bytes(client_fd,&lenR, sizeof(int)) <= 0) {
      perror("Errore nella ricezione della lunghezza della risposta");
      exit(EXIT_FAILURE);
    }
    
    char* bufR = malloc(lenR);
    if(recv_all_bytes(client_fd,bufR,lenR) <= 0) {
      perror("Errore nella ricezione della risposta");
      exit(EXIT_FAILURE);
    }
    
    //calcolo punteggio
    int p = updatePoints(i,bufR,themeChosen,nickname);
    printf("risposta: %s punteggio domanda: %d\n",bufR,p);
    //manda feedback sulla risposta data al client
    if(p == 1) {   
    //messaggio corretta al client
    if(send(client_fd, MSG_OK, MSG_LEN, 0) == -1) {
      perror("Errore in send() del risposta corretta");
      exit(EXIT_FAILURE);
    }    
  }
  else {
    //messaggio non corretta al client
    if(send(client_fd, MSG_NO, MSG_LEN, 0) == -1) {
      perror("Errore in send() del risposta errata");
      exit(EXIT_FAILURE);
    } 
  }  
  }//chiude for
}

//funzione che data una risposta torna 1 se è giusta o 0 altrimenti aggiornando il punteggio nella relativa struttura dati
int updatePoints(int numq,char* bufR,int themeChosen,char* nickname) {
  char bufFile[MAXCHAR_LINE];
  get_filename_from_index(bufFile,themeChosen, &current_session);
  //lock sul mutex
  pthread_mutex_lock(&lockPlayers);
  struct Player* ptr = get_player(current_session.players, nickname);
    int i = 0;
    while(strcmp(ptr->theme[i].name,current_session.availableThemes[themeChosen]) != 0 && i < current_session.num_themes ) {
    i++;
  } 
  if(ptr->theme[i].points == -1) //se è la prima domanda
      ptr->theme[i].points = 0;
      
  if(check_answer(bufFile, bufR,numq)) {
    ptr->theme[i].points++;
    //unlock mutex
    pthread_mutex_unlock(&lockPlayers);
    return 1;
  }
  else  {
    //unlock mutex
    pthread_mutex_unlock(&lockPlayers);
    return 0;
  }   
    

}

//dato il nickname ritorna un puntatore al giocatore, NULL se il nickname non è presente
//l'uso di questa funzione deve essere fatto all'interno di un blocco critico
struct Player* get_player(struct Player* current_player,char* nickname) {
  if(current_player == NULL)
    return NULL;
    
  if(strcmp(current_player->nickname, nickname) == 0)
    return current_player;
    
  return get_player(current_player->next, nickname);
}

