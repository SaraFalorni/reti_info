#include "serverUtility.h"

void init_session(struct Session* current_session) {
  current_session->players = NULL;
  current_session->num_players = 0;
  
  current_session->num_themes = quanti_temi();
  printf("numero di temi disponibili current_session: %d\n",current_session->num_themes);//da togliere
  
  current_session->availableThemes = (char**)malloc(current_session->num_themes * sizeof(char*));
  
  for(int i = 0; i < current_session->num_themes; i++) {
    char buf[MAXCHAR_LINE];
    get_theme_name(i+1,buf);
    
    size_t len_themeName = strlen(buf)+1;
    current_session->availableThemes[i] = malloc(len_themeName);
    memcpy(current_session->availableThemes[i],buf,len_themeName);
    
  }
  
  return;
}

bool insert_player(char* nickname,struct Session* current_session) {
  struct Player* p;
  p = current_session->players;
  while(p != NULL) {
    if( strcmp(nickname,p->nickname))
      return false;
    p = p->next;
  }
  p = (struct Player*)malloc(sizeof(struct Player));
  size_t len_nickname = strlen(nickname)+1;
  p->nickname = (char*)malloc(len_nickname);
  memcpy(p->nickname,nickname,len_nickname);  
  
  //INIZIALIZZAZIONE DEI TEMI CON POINTS = -1
  
  p->theme = (struct Theme*)malloc(current_session->num_themes * sizeof(struct Theme));
  for(int i = 0; i < current_session->num_themes ; i++){
  size_t len_themeName = strlen(current_session->availableThemes[i]) + 1;
    p->theme[i].name = (char*)malloc(len_themeName);
  strcpy(p->theme[i].name, current_session->availableThemes[i]);
  p->theme[i].points = -1;
  }
  p->next = NULL;
  current_session->players = p;
  
  //print_session(current_session);
  //giocatore inserito correttamente
  
  
  return true;
}


void get_nickname(int client_fd,struct Session* current_session, char* name) {
  char nickname[MAXCHAR_NICKNAME];
  
  ssize_t bytes_received = recv(client_fd, nickname, MAXCHAR_NICKNAME, 0);
  
  if(bytes_received == -1) {
    perror("Errore in recv() per nickname");
    exit(EXIT_FAILURE);
  }
  
  if(insert_player(nickname,current_session)) {   
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

void send_themes(int client_fd,struct Session* current_session, char* nickname) {
//manda un messaggio al client con il numero di temi e aspetta un feedback sulla ricezione di quest'ultimo
  char msg[MSG_LEN];
  

  if(send(client_fd, &current_session->num_themes, sizeof(current_session->num_themes), 0) == -1) {
      perror("Errore in send() del numero di temi");
      exit(EXIT_FAILURE);
    }
    ssize_t bytes_received = recv(client_fd, msg, MSG_LEN, 0);
  
   if(bytes_received == -1) {
      perror("Errore in recv() di conferma del numero di temi");
      exit(EXIT_FAILURE);
    }

  if(strcmp(msg,MSG_OK) == 0) {
       
    for(int i = 0; i < current_session->num_themes ; i++) {
      int len = strlen(current_session->availableThemes[i]) + 1;
      
      if(send(client_fd, &len, sizeof(int),0)== -1) { //invio lunghezza della stringa
        perror("Errore in send() della lunghezza del nome del tema");
        exit(EXIT_FAILURE);
      } 
      if(send(client_fd,current_session->availableThemes[i],len,0)== -1) { 
        perror("Errore in send() del nome del tema");
        exit(EXIT_FAILURE);
      } 
    }
    
  }
  int themeChosen;
  if(recv_all_bytes(client_fd,&themeChosen,sizeof(int)) <= 0) {
      perror("Errore nella ricezione della stringa");
      exit(EXIT_FAILURE);
    }
  
  playquiz(themeChosen, nickname,current_session, client_fd);
}

void playquiz(int themeChosen, char* nickname,struct Session* current_session, int client_fd) {
  char bufQ[MAXCHAR_LINE], bufFile[MAXCHAR_LINE];
  get_filename_from_index(bufFile,themeChosen, current_session);
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
    int p = updatePoints(i,bufR,themeChosen,nickname,current_session);
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
int updatePoints(int numq,char* bufR,int themeChosen,char* nickname,struct Session* current_session) {
  char bufFile[MAXCHAR_LINE];
  get_filename_from_index(bufFile,themeChosen, current_session);
  struct Player* ptr = get_player(current_session, nickname);
    int i = 0;
    while(strcmp(ptr->theme[i].name,current_session->availableThemes[themeChosen]) != 0 && i < current_session->num_themes ) {
    i++;
  } 
  if(ptr->theme[i].points == -1) //se è la prima domanda
      ptr->theme[i].points = 0;
      
  if(check_answer(bufFile, bufR,numq)) {
    ptr->theme[i].points++;
    return 1;
  }
  else     
    return 0;

}

//dato il nickname ritorna un puntatore al giocatore
struct Player* get_player(struct Session* current_session,char* nickname) {
  struct Player* p = current_session->players;
  
  while(strcmp(p->nickname,nickname) != 0) {
    if(p == NULL)
      break;
    p = p->next;
  } 
  return p;
}

