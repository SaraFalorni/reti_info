//FUNZIONI CHE IMPLEMENTANO IL GIOCO

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "gameFunctions.h"
//-------------------------------------------------------------------------------------------------------------

//funzione che interagisce con il client per ottenere il nuovo nickname
//ha come parametri il socket e una stringa vuota
void get_nickname(int client_fd, char* name) {
    char* nickname;
    
    while(1) {
        //riceve il nickname dal client
        uint32_t len = recvStringLen(client_fd);
        nickname = malloc(len);
        //gestion errore malloc
        if(nickname == NULL) {
            printf("Errore nel malloc del nickname\n");
            close(client_fd);
            pthread_exit(NULL); 
        }
        
        if(recvAllBytes(client_fd,nickname,len) <= 0) {
            printf("Disconnessione del client o errore.\n");
            close(client_fd);
            pthread_exit(NULL);
        }
        //se l'inserimento va a buon fine manda un messaggio di conferma al client, altrimenti manda un messaggio di errore e chiede nuovamente un nickname
        if(insert_player(nickname,client_fd)) {   
            //messaggio di ok a client
            if(sendAllBytes(client_fd, MSG_OK, MSG_LEN) == 0) {
                perror("Errore in send() dell'ok al nickname");
                close(client_fd);
                pthread_exit(NULL);
            }    
            break;//esce dal while solo dopo un inserimento avvenuto con successo
        }
        else {
          //messaggio non ok al client
          if(sendAllBytes(client_fd, MSG_NO, MSG_LEN) == 0) {
                perror("Errore in send() del no al nickname");
                close(client_fd);
                pthread_exit(NULL);
            } 
        }
   }
   strcpy(name,nickname);//copia in name il nickname dato dal client
}

//-------------------------------------------------------------------------------------------------------------

//funzione che manda al client i nomi dei temi disponibili
//parametri: socket del client e nickname
//il nickname serve per un'eventuale endquiz durante il gioco
void send_themes(int client_fd, char* nickname) {
    //manda un messaggio al client con il numero di temi e aspetta un feedback sulla ricezione di quest'ultimo

    //manda il numero dei temi al client
    if(sendAllBytes(client_fd, &current_session.num_themes, sizeof(current_session.num_themes)) == 0) {
        perror("Errore in send() del numero di temi");
        close(client_fd);
        delete_player(nickname);
        pthread_exit(NULL);
    }

    //il numero dei temi è stato ricevuto correttamente prosegue mandando il nome di ogni tema, uno per volta         
      for(int i = 0; i < current_session.num_themes ; i++) {
          //controllo se il giocatore ha già giocato l'i-esimo tema
          
          //lock mutex
          pthread_mutex_lock(&lockPlayers);
          
          struct Player* ptr = get_player(current_session.players,nickname);
          
          //unlock mutex
          pthread_mutex_unlock(&lockPlayers);
          
          //se ptr->themePoints[i] != -1 (valore di inizializzazione) vuol dire che il client ha già giocato a quel tema
          //quindi viene inviato -1 invece che la lunghezza della stringa del nome del tema per notificare il client
          if(ptr->themePoints[i] != -1) {
              char* emptyStr = " ";//indica il tema non disponibile
          
              if(sendString(client_fd, emptyStr) <= 0) { //invio della stringa vuota per indicare che non è un tema disponibile
                  perror("Errore in send() del nome del tema");
                  close(client_fd);
                  delete_player(nickname);
                  pthread_exit(NULL);
              } 
          }
          //client non ha ancora giocato al quiz di quel tema
          //viene inviata la lunghezza della stringa e poi la stringa contenente il nome del tema
          else {
              if(sendString(client_fd,current_session.availableThemes[i]) <= 0) { //invio della stringa (nome dell'i-esimo tema)
                  perror("Errore in send() del nome del tema");
                  close(client_fd);
                  delete_player(nickname);
                  pthread_exit(NULL);
              }
          }
     } 
  
  //riceve dal client l'indice del tema a cui vuole giocare
  int themeChosen;
  if(recvAllBytes(client_fd,&themeChosen,sizeof(int)) <= 0) {
      printf("Disconnessione del client o errore.\n");
      delete_player(nickname);
      close(client_fd);
      pthread_exit(NULL);
  }
  
  //chiamata alla funzione che implementa lo scambio domande risposte
  playquiz(themeChosen, nickname,client_fd);
}

//-------------------------------------------------------------------------------------------------------------

//funzione che implementa il gioco vero e proprio
//ha come parametri l'indice del tema scelto dal client, il nickname e il socket
void playquiz(int themeChosen, char* nickname, int client_fd) {
    //stringhe in cui sono memorizzate le domande (bufQ) e il nome completo del file da cui prenderle (bufFile)
    char bufQ[MAXCHAR_LINE], bufFile[MAXCHAR_LINE];
    
    //funzione che scrive in bufFile il nome del file
    get_filename_from_index(bufFile,themeChosen, &current_session);
    
    //ciclo che invia ogni domanda al client 
    for(int i = 0; i < NUM_Q ; i++) {
        //recupero la domanda dal file
        read_q(bufFile,bufQ,i);
        
        //invia al client la lunghezza della stringa e poi la stringa contenente la i-esima domanda
        if(sendString(client_fd,bufQ) <= 0) { //invio della stringa (domanda)
            perror("Errore in send() della domanda");
            close(client_fd);
            delete_player(nickname);
            pthread_exit(NULL);
        }
        
        //il messaggio che riceve è per indicare se il client ha normalmente risposto o richiesto schowscore o endquiz
        //per valutarlo sfrutta la funzione check_comand
        char msg[MSG_LEN];
        if(recvAllBytes(client_fd,msg,MSG_LEN) <= 0) {
            printf("Disconnessione del client o errore.\n");
            delete_player(nickname);
            close(client_fd);
            pthread_exit(NULL);
        }
        //se check_comand torna true vuol dire che è stata fatta una show score invece di rispondere
        //quindi va ripetuta la domanda precedente, per farlo decremento i
        if(check_comand(client_fd,msg)) {
            i--;
            continue;
        }
          
        //RICEZIONE DELLA RISPOSTA E VALUTAZIONE DI ESSA  
        
        //riceve la risposta (sempre ricevendo prima il numero di byte)    
        uint32_t len = recvStringLen(client_fd);
        char* bufR = malloc(len);
        //gestione errore nel malloc
        if(bufR == NULL) {
            printf("Errore nel malloc della risposta\n");
            delete_player(nickname);
            close(client_fd);
            pthread_exit(NULL); 
        }        
        
        if(recvAllBytes(client_fd,bufR,len) <= 0) {
            printf("Disconnessione del client o errore.\n");
            delete_player(nickname);
            close(client_fd);
            pthread_exit(NULL);
        }
        
        //calcolo punteggio
        int p = updatePoints(i,bufR,themeChosen,nickname);
        
        //manda feedback sulla risposta data al client
        //p = 1 se la risposta è giusta
        if(p == 1) {   
            //messaggio corretta al client
            if(sendAllBytes(client_fd, MSG_OK, MSG_LEN) <= 0) {
                perror("Errore in send() del risposta corretta");
                close(client_fd);
                delete_player(nickname);
                pthread_exit(NULL);
            }    
        }
        else {
            //messaggio non corretta al client
            if(sendAllBytes(client_fd, MSG_NO, MSG_LEN) <= 0) {
                perror("Errore in send() del risposta errata");
                close(client_fd);
                delete_player(nickname);
                pthread_exit(NULL);
            } 
        }  
        
        //libera la memoria
        free(bufR);
        
    }//chiude for
    
    //se arriva a questo punto il quiz è stato completato 
    //aggiorna la struttura dati corispondente in current_session
    pthread_mutex_lock(&lockPlayers);
    
    get_player(current_session.players,nickname)->themeCompleted[themeChosen] = true;
    
    pthread_mutex_unlock(&lockPlayers);
}

//-------------------------------------------------------------------------------------------------------------

//funzione che data una risposta torna 1 se è giusta o 0 altrimenti 
//aggiornando il punteggio nella relativa struttura dati di current_session
//parametri: numero della domanda (numq), risposta data (bufR), indice del tema scelto, nickname del giocatore
int updatePoints(int numq,char* bufR,int themeChosen,char* nickname) {
    
    //recupera il nome del file da aprire per verificare la correttezza
    char bufFile[MAXCHAR_LINE];
    get_filename_from_index(bufFile,themeChosen, &current_session);
    
    //lock sul mutex
    pthread_mutex_lock(&lockPlayers);
    
    struct Player* ptr = get_player(current_session.players, nickname);//puntatore al giocatore dato il nickname
    
    if(ptr->themePoints[themeChosen] == -1) //se è la prima domanda 
        ptr->themePoints[themeChosen] = 0;
        
    //controllo se la risposta è corretta utilizzando check_answer che torna true (corretta) o false (errata)
    if(check_answer(bufFile, bufR,numq)) {//risposta corretta
        ptr->themePoints[themeChosen]++;
        
        //unlock mutex
        pthread_mutex_unlock(&lockPlayers);
        
        return 1;
    }
    else  {//risposta errata
        //unlock mutex
        pthread_mutex_unlock(&lockPlayers);
        return 0;
    }   
}

//-------------------------------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce il comando show score chiamato dal client
//manda al client prima il numero di temi e poi per ognuno di essi la classifica ottenuta con get_theme_rankings
//manda sempre la lunghezza della stringa prima della stringa
void do_show_score(int client_fd) {
    //manda il numero di temi
    if(sendAllBytes(client_fd,&current_session.num_themes,sizeof(int)) <= 0) { 
        perror("Errore in send() del numero di classifiche");
        close(client_fd);
        pthread_exit(NULL);
    }

    struct Player** rankings = get_theme_rankings(client_fd);//calcola le classifiche
    struct Player* current_player;
    
    int num_ranked;//intero che contiene il numero di giocatori in una determinata classifica
    for(int i = 0 ; i < current_session.num_themes ; i++) {
        current_player = rankings[i]; //classifica dell'i-esimo tema
        
        //manda il numero di giocatori nella i-esima classifica
        num_ranked = count_ranked(rankings,i);
        if(sendAllBytes(client_fd,&num_ranked,sizeof(int)) <= 0) { 
            perror("Errore in send() del numero di giocatori nella classifica");
            close(client_fd);
            pthread_exit(NULL);
        }
        //per ogni giocatore nella classifica invia nickanme e punti
        while(current_player != NULL) {
            //invio nickname del k-esimo classificato dell'i-esimo tema
            if(sendString(client_fd,current_player->nickname) <= 0) { 
                perror("Errore in send() del nickname (ranking)");
                close(client_fd);
                pthread_exit(NULL);
            }
            //invio del punteggio del giocatore
            if(sendAllBytes(client_fd,&current_player->themePoints[0],sizeof(int)) <= 0) { 
                perror("Errore in send() del punteggio (ranking)");
                close(client_fd);
                pthread_exit(NULL);
            } 
            current_player = current_player->next;
        }   
    }
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce il comando endquiz chiamato dal client
void do_endquiz(int client_fd) {
    //riceve dal client il nickname per poter cancellare le relative informazioni  
    uint32_t len = recvStringLen(client_fd);
    char* nickname = malloc(len);
    //gestione errore nel malloc
    if(nickname == NULL) {
        printf("Errore nel malloc del nickname nell'endquiz\n");
        close(client_fd);
        pthread_exit(NULL); 
    }
    
    if(recvAllBytes(client_fd,nickname,len) <= 0) {
        printf("Disconnessione del client o errore.\n");
        close(client_fd);
        pthread_exit(NULL);
    }
    //deve eliminare il player da current_session
    delete_player(nickname);

    show_overview();//da cancellare
    
    //libera la memoria
    free(nickname);
      
}

//-------------------------------------------------------------------------------------------------------------
//funzione che garantisce la lettura del numero corretto di bytes dal socket
uint32_t recvAllBytes(int client_fd, void *buf, uint32_t len) {
  uint32_t totRec = 0;
  uint32_t bytesRec = 0;
  
  while(totRec < len) {
    bytesRec = recv(client_fd,buf+totRec,len-totRec,0);
    if(bytesRec <= 0)
      return 0;
    totRec += bytesRec;
  }
  
  return totRec;
}

//-------------------------------------------------------------------------------------------------------------
//funzione che garantisce di mandare il numero corretto di bytes
uint32_t sendAllBytes(int client_fd, void *buf, uint32_t len) {
  uint32_t totSent = 0;
  uint32_t bytesSent = 0;
  
  while(totSent < len) {
    bytesSent = send(client_fd,buf+totSent,len-totSent,0);
    if(bytesSent <= 0)
      return 0;
    totSent += bytesSent;
  }
  
  return totSent;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce la ricezione di una stringa
//ricevendo prima la lunghezza e successivamente la stringa stessa
//ritorna il puntatore alla stringa
uint32_t recvStringLen(int client_fd) {
  //riceve prima la lunghezza della stringa
  uint32_t netLen;
  
  recvAllBytes(client_fd,&netLen,sizeof(netLen));
  
  uint32_t len = ntohl(netLen); //da network a host
  
  return len;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce l'invio di una stringa, mandando prima la lunghezza e successivamente la stringa stessa
int sendString(int client_fd, void *buf) {
  //lunghezza della stringa
  uint32_t len = (uint32_t)strlen(buf)+1;
  uint32_t netLen = htonl(len); //host to network
  
  sendAllBytes(client_fd,&netLen,sizeof(netLen));//manda la lunghezza della stringa
  
  //manda la stringa
  int bytesSent = sendAllBytes(client_fd,buf,len);
  
  return bytesSent;
}

