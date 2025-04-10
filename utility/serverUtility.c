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
  
  show_overview();
  return;
}

void show_overview() {
    printf("Trivia Quiz\n");

    for(int i = 0 ; i < NUM_SEPARATOR; i++)
        printf("+");
        
    printf("\nTemi:\n");
    for(int i = 0 ; i < current_session.num_themes ; i++) {
      printf("%d - %s\n",i+1,current_session.availableThemes[i]);
    }
    
    for(int i = 0 ; i < NUM_SEPARATOR; i++)
        printf("+");
        
   pthread_mutex_lock(&lockPlayers); 
    printf("\nPartecipanti (%d)\n",current_session.num_players);
    for(int i = 0 ; i < current_session.num_players ; i++) {
      printf("- %s\n", get_player_index(current_session.players,i)->nickname );
    }  
   pthread_mutex_unlock(&lockPlayers);
}

void* client_handler(void* arg) {
  int client_fd = *(int*)arg;
  free(arg); //????
    
  //il primo msg che riceve è il nickname
  char nickname[MAXCHAR_NICKNAME];
  get_nickname(client_fd,nickname);
  
  struct Player** rankings = get_theme_rankings();
  print_rankings(rankings);
  print_completed_quiz(rankings);
  
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

  if(get_player(current_session.players,nickname) != NULL) {
    pthread_mutex_unlock(&lockPlayers);
      return false;
  }
  new_player = (struct Player*)malloc(sizeof(struct Player));
  size_t len_nickname = strlen(nickname)+1;
  new_player->nickname = (char*)malloc(len_nickname);
  memcpy(new_player->nickname,nickname,len_nickname);  
  
  //INIZIALIZZAZIONE DEI TEMI CON POINTS = -1 e completed con false
  
  new_player->themePoints = malloc(current_session.num_themes * sizeof(int));
  new_player->themeCompleted = malloc(current_session.num_themes * sizeof(bool));
  for(int i = 0; i < current_session.num_themes ; i++){
    new_player->themePoints[i] = -1;
    new_player->themeCompleted[i] = false;
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

  return true;
}


void get_nickname(int client_fd, char* name) {
  char nickname[MAXCHAR_NICKNAME];
  
  while(1) {
    if(recv(client_fd, nickname, MAXCHAR_NICKNAME, 0) == -1) {
    perror("Errore in recv() per nickname");
    exit(EXIT_FAILURE);
    }
  
    if(insert_player(nickname)) {   
      //messaggio di ok a client
      if(send(client_fd, MSG_OK, MSG_LEN, 0) == -1) {
        perror("Errore in send() dell'ok al nickname");
        exit(EXIT_FAILURE);
      }    
      break;
    }
    else {
      //messaggio non ok al client
      if(send(client_fd, MSG_NO, MSG_LEN, 0) == -1) {
        perror("Errore in send() del no al nickname");
        exit(EXIT_FAILURE);
        } 
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
      if(ptr->themePoints[i] != -1) {
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
      
      //il primo messaggio che riceve è per indicare se il client ha normalmente risposto o richiesto schowscore o endquiz
      char msg[MSG_LEN];
      if(recv_all_bytes(client_fd,msg,MSG_LEN) <= 0) {
        perror("Errore nella ricezione della lunghezza della risposta");
        exit(EXIT_FAILURE);
      }
      //se check_comand torna true vuol dire che è stata fatta una show score invece di rispondere, quindi va ripetuta la domanda precedente
      if(check_comand(client_fd,msg)) {
        i--;
        continue;
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
  //se arriva a questo punto il quiz è stato completato
  pthread_mutex_lock(&lockPlayers);
  get_player(current_session.players,nickname)->themeCompleted[themeChosen] = true;
  pthread_mutex_unlock(&lockPlayers);
}

//funzione che data una risposta torna 1 se è giusta o 0 altrimenti aggiornando il punteggio nella relativa struttura dati
int updatePoints(int numq,char* bufR,int themeChosen,char* nickname) {
  char bufFile[MAXCHAR_LINE];
  get_filename_from_index(bufFile,themeChosen, &current_session);
  //lock sul mutex
  pthread_mutex_lock(&lockPlayers);
  struct Player* ptr = get_player(current_session.players, nickname);
  if(ptr->themePoints[themeChosen] == -1) //se è la prima domanda
      ptr->themePoints[themeChosen] = 0;
      
  if(check_answer(bufFile, bufR,numq)) {
    ptr->themePoints[themeChosen]++;
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

//dato un intero ritorna un puntatore al giocatore in quella posizione nella lista curre_session.players
//l'uso di questa funzione deve essere fatto all'interno di un blocco critico
struct Player* get_player_index(struct Player* current_player,int index) {
  if(current_player == NULL || index < 0)
    return NULL;
    
  if(index == 0)
    return current_player;
    
  return get_player_index(current_player->next, index-1);
}


//restituisce la classifica per tutti i temi 
//il tema è individuato dall'indice (corrispondente al numero di riga che ha nel documento ./txt/indiceTemi.txt)
//l'uso di questa funzione deve essere fatto all'interno di un blocco critico
struct Player** get_theme_rankings() {
   
  // array di liste (classifiche) per tema, l'indice dell'array identifica il tema
  struct Player** rankings = (struct Player**)malloc(current_session.num_themes * sizeof(struct Player*));
     
  // per ordinare le classifiche utilizza Counting sort
  //crea un array di 5+1 elementi i cui indici identificano il punteggio ottenuto nel quiz per quel tema
  //ogni elemento è una lista ai giocatori che hanno ottenuto quel punteggio
  struct Player*** buckets = (struct Player***)malloc(current_session.num_themes * sizeof(struct Player**));
  struct Player*** bucket_tails = (struct Player***)malloc(current_session.num_themes * sizeof(struct Player**));//puntatore all'ultimo elemento, per inserimento ordinato
 
  // Inizializzazione degli array di supporto
  for (int i = 0; i < current_session.num_themes; i++) {
    //i è l'indice del tema
    buckets[i] = (struct Player**)malloc((NUM_Q+1) * sizeof(struct Player*));
    bucket_tails[i] = (struct Player**)malloc((NUM_Q+1) * sizeof(struct Player*));

    for (int k = 0; k < (NUM_Q+1); k++) {
    //k è l'indice del punteggio
        buckets[i][k] = NULL;//i-esimo tema k-esimo punteggio
        bucket_tails[i][k] = NULL;
    }
  }
  pthread_mutex_lock(&lockPlayers);
  // Scansione dei giocatori
  struct Player* current_player = current_session.players;
  
  while (current_player != NULL) { //per ogni giocatore
    // Per ogni tema, creiamo copie del giocatore solo se il punteggio è valido
    for (int i = 0; i < current_session.num_themes; i++) {
      //i è l'indice del tema    

      // Consideriamo solo i giocatori che hanno giocato a quell quiz, cioè quelli con punteggio diverso da -1
      if (current_player->themePoints[i] != -1) {
      
        //copia il giocatore solo con il punteggio del relativo tema
        struct Player* player_copy = copy_player(current_player,i);
             
        // Inserisce nel bucket in base al punteggio
        if (buckets[i][current_player->themePoints[i]] == NULL) {
          //se è il primo giocatore inserito
          buckets[i][current_player->themePoints[i]] = player_copy;
          bucket_tails[i][current_player->themePoints[i]] = player_copy;
        } 
        else {
          //se è già presente un giocatore nella lista del bucket
          bucket_tails[i][current_player->themePoints[i]]->next = player_copy;
          bucket_tails[i][current_player->themePoints[i]] = player_copy;
        }
      }
  }
  current_player = current_player->next;//giocatore successivo
  }
  
  //ordinamento delle classifiche dai bucket a rankings
  get_final_rankings(rankings,buckets,bucket_tails );
  
  pthread_mutex_unlock(&lockPlayers);
  
  // Pulizia memoria delle strutture temporanee
  for (int theme = 0; theme < current_session.num_themes; theme++) {
      free(buckets[theme]);
      free(bucket_tails[theme]);
  }
  free(buckets);
  free(bucket_tails);
 
  return rankings;
}

//funzione ausiliaria di get_theme_rankings() per creare le classifiche copia un giocatore con solo il punteggio relativo a un tema
struct Player* copy_player(struct Player* current_player,int index_theme) {
  // Crea una copia del giocatore
  struct Player* player_copy = (struct Player*)malloc(sizeof(struct Player));
 
  player_copy->nickname = malloc(strlen(current_player->nickname));
  strcpy(player_copy->nickname,current_player->nickname);

  //basta memorizzare il punteggio relativo a solo quel tema
  player_copy->themePoints = malloc(sizeof(int));
  
  player_copy->themePoints[0] = current_player->themePoints[index_theme];
  
  //memorizza anche se il tema è stato completato o meno (serve per la funzione print_completed_quiz
  //basta memorizzare il punteggio relativo a solo quel tema
  player_copy->themeCompleted = malloc(sizeof(bool));
  player_copy->themeCompleted[0] = current_player->themeCompleted[index_theme];
  
  player_copy->next = NULL;
  return player_copy;
}

//funzione ausiliaria di get_theme_rankings() per creare le classifiche
//ordina in una sola lista per tema tutti i giocatori con relativo punteggio in ordine decrescente utilizzando i bucket già creati
struct Player** get_final_rankings(struct Player** rankings,struct Player*** buckets,struct Player*** bucket_tails ) {
// Costruiamo le liste risultanti per ogni tema
  for (int i = 0; i < current_session.num_themes; i++) {
    rankings[i] = NULL;
    struct Player* ranking_tail = NULL;
   
    // Combina i bucket in ordine decrescente 
    for (int points = NUM_Q; points >= 0; points--) {
      if (buckets[i][points] != NULL) {
        if (rankings[i] == NULL) {//se non c'è ancora niente in classifica
          rankings[i] = buckets[i][points];
          ranking_tail = bucket_tails[i][points];
        } 
        else {//se c'è già qualcosa in classifica aggiungo in fondo alla coda
          ranking_tail->next = buckets[i][points];
          ranking_tail = bucket_tails[i][points];
        }
      }
    }
  }
  return rankings;
}

void print_rankings(struct Player** rankings) {
  
  for(int i = 0; i < current_session.num_themes ; i++) {
    struct Player* current_player = rankings[i];
    
    if(current_player != NULL) 
      printf("\nPunteggio tema %d\n",i+1);
    
    while(current_player != NULL) {
      printf("- %s %d\n",current_player->nickname, *current_player->themePoints);
      current_player = current_player->next;
    }
  }
  
}

void print_completed_quiz(struct Player** rankings) {
  bool first;
  for(int i = 0; i < current_session.num_themes ; i++) {
    struct Player* current_player = rankings[i];
    first = true;
        
    while(current_player != NULL && *current_player->themeCompleted == true) {
      if(first) {
        printf("\nQuiz Tema %d completato\n",i+1);
        first = false;
      }
      printf("- %s\n",current_player->nickname);
      current_player = current_player->next;
    }
  }
  
}

//funzione che gestisce l'eventualità che il client abbia richiesto endquiz o showscore
//torna true se deve continuare il flusso del quiz: quindi se il client ha risposto o ha fatto show scores correttamente
//false se deve uscire dal flusso a causa di endquiz
bool check_comand(int client_fd,char* msg) {
  //se ha ricevuto MSG_OK continua normalmente
  //se ha ricevuto MSG_RK rimanda alla funzione do_show_score(client_fd)
  //se ha ricevuto MSG_EX rimanda alla funzione do_endquiz(client_fd)
  if(strcmp(MSG_RK,msg) == 0) {
    do_show_score(client_fd);
    return true;
  }
  else if(strcmp(MSG_EX,msg) == 0) 
    do_endquiz(client_fd);
  return false;
}

void do_show_score(int client_fd) {
  //manda il numero di temi
  if(send(client_fd,&current_session.num_themes,sizeof(int),0)== -1) { 
      perror("Errore in send() del numero di classifiche");
      exit(EXIT_FAILURE);
  }

  struct Player** rankings = get_theme_rankings();
  struct Player* current_player;
  int num_ranked;
  for(int i = 0 ; i < current_session.num_themes ; i++) {
  current_player = rankings[i];
  //manda il numero di giocatori nella i-esima classifica
    num_ranked = count_ranked(rankings,i);
    if(send(client_fd,&num_ranked,sizeof(int),0) == -1) { 
      perror("Errore in send() del numero di giocatori nella classifica");
      exit(EXIT_FAILURE);
    }
    
    while(current_player != NULL) {
      //invio lunghezza del nickname
      int len = strlen(current_player->nickname);
      if(send(client_fd, &len, sizeof(int),0)== -1) { 
        perror("Errore in send() della lunghezza del nickname (ranking)");
        exit(EXIT_FAILURE);
      } 
      //invio nickname del k-esimo classificato dell'i-esimo tema
      if(send(client_fd,current_player->nickname,len,0)== -1) { 
        perror("Errore in send() del nickname (ranking)");
        exit(EXIT_FAILURE);
      }
      //invio del punteggio del giocatore
      if(send(client_fd,&current_player->themePoints[0],sizeof(int),0) == -1) { 
        perror("Errore in send() del punteggio (ranking)");
        exit(EXIT_FAILURE);
      } 
      current_player = current_player->next;
    }   
  }
}


void do_endquiz(int client_fd) {
  //riceve dal client il nickname per poter cancellare le relative informazioni
    int len;
    if(recv_all_bytes(client_fd,&len, sizeof(int)) <= 0) {
      perror("Errore nella ricezione della lunghezza del nickname (endquiz)");
      exit(EXIT_FAILURE);
    }
    
    char* nickname = malloc(len);
    if(recv_all_bytes(client_fd,nickname,len) <= 0) {
      perror("Errore nella ricezione del nickname (endquiz)");
      exit(EXIT_FAILURE);
    }
    printf("n endquiz nickname %s %d\n",nickname,len);
    
    printf("entra nella do_enquiz prima di delete_player\n");
    //deve cancellare il player da current_session
    delete_player(nickname);
    printf("entra nella do_enquiz dopo di delete_player\n");
    show_overview();//da cancellare
    //libera la memoria
    free(nickname);
    //chiude la comunicazione con il client
    close(client_fd);
    pthread_exit(NULL);
    
}

int count_ranked(struct Player** rankings,int theme_index) {
  if(rankings == NULL || theme_index < 0) 
    return -1;
    
  if(rankings[theme_index] == NULL)
    return 0;
    
  int num_players = 0;
  struct Player* current_player = rankings[theme_index];
  
  while(current_player != NULL) {
    num_players++;
    current_player = current_player->next;
  }
  return num_players;
}

void delete_player(char* nickname) {
  //eliminia il player corrispondente da current_session
  printf("entra in delete_player\n");
  pthread_mutex_lock(&lockPlayers);
  struct Player* current_player = current_session.players;
  struct Player* prec_player = NULL;

  while(current_player != NULL) {
    printf("paragona %s a %s\n",nickname, current_player->nickname);
    if(strcmp(nickname, current_player->nickname) == 0) {
      if(prec_player == NULL) {
        //se è il primo della lista
        current_session.players = current_player->next;
        printf("\neliminato il primo\n");
      }
      else {
        prec_player->next = current_player->next;
        printf("\neliminato\n");
      }
      current_session.num_players--;
      
      //libera la memoria allocata
      free(current_player->nickname);
      free(current_player->themePoints);
      free(current_player->themeCompleted);
      free(current_player);
      break;
    }
    prec_player = current_player;
    current_player = current_player->next;
  }
  pthread_mutex_unlock(&lockPlayers);
}
