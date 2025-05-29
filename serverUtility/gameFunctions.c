//FUNZIONI CHE IMPLEMENTANO IL GIOCO

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "gameFunctions.h"
//-------------------------------------------------------------------------------------------------------------

//funzione che interagisce con il client per ottenere il nuovo nickname
//ha come parametri il socket e una stringa vuota
int getNickname(struct ClientInfo* client) {
    //char* nickname;
    
    //while(1) {
        //riceve il nickname dal client recvString cancellare
        /*int len = recvStringLen(client_fd);
        if(len < 0) {
            perror("Errore nella ricezione della lunghezza del nickname\n");
            return -1;//gestito in manageClientGame
        }
        else if(len == 0)
            return 0;//gestito in manageClientGame

        nickname = malloc(len);
        //gestion errore malloc
        if(nickname == NULL) {
            perror("Errore nel malloc del nickname\n");
            return -1;//gestito in manageClientGame
        }*/
    
    if(client->isResponding == true) {
        //caso in cui il client non ha ancora inserito un nickname valido
        printf("entra in getNickname isResponding %d\n",client->isResponding);//cancellare
        //ricezione lunghezza stringa
        uint32_t netLen = 0;
        printf("prima della ricezione lunghezza\n"); //cancellare
        int lenRecv = recvAllBytes(client,&netLen,sizeof(uint32_t));//riceve la lunghezza della stringa
        if(lenRecv <= 0) 
            return lenRecv; //socket non ancora pronto o errore
        
        uint32_t len = ntohl(netLen); //da network a host
        printf("lunghezza nickname: %d\n",len);//cancellare
        char* nickname = malloc(len);
        if(nickname == NULL)
            return -1; //gestione errore propagata
        int bytesRec = recvAllBytes(client,nickname,len);
        //int bytesRec = recvString(client,&nickname); cancellare
        if( bytesRec <= 0) 
            return bytesRec; //-1 se c'è stato errore, 0 se socket non pronto, gestito da manageClientGame
            printf("nickname: %s\n",nickname);//cancellare
        printf("prima di insert\n");//cancellare
        //se l'inserimento va a buon fine manda un messaggio di conferma al client, altrimenti manda un messaggio di errore e chiede nuovamente un nickname
        int insert = insertPlayer(nickname,client->client_fd);
        printf("insert risulted: %d\n",insert);//cancellare
        if( insert == -1)
            return -1;
        else if( insert == 1) {  
            //inserisce il nickname nel ClientInfo
            client->nickname = malloc(strlen(nickname)+1);
            if(client->nickname == NULL) {
                perror("errore nel malloc");
                deletePlayer(client->nickname);
                return -1;//gestito in ClientHandler
            }
            strcpy(client->nickname,nickname);//copia in name il nickname dato dal client
            free(nickname);

            client->isResponding = false; //il nickname è stato ricevuto dal client, passa alla fase di conferma/rifiuto

            //prepara l'invio del messaggio di ok a client
            //scrive nel buffer send del client
            client->sendBuf.totLen = MSG_LEN;
            client->sendBuf.buffer = malloc(MSG_LEN+1);
            if(client->sendBuf.buffer == NULL)
                //gestione errore propagata
                return -1; 
            strcpy(client->sendBuf.buffer,MSG_OK);
            client->sendBuf.progress = 0;
                                /*int bytesSent = sendAllBytes(client->client_fd, MSG_OK, MSG_LEN); cancellare
                                if( bytesSent < 0) {
                                    perror("Errore in send() dell'ok al nickname");
                                    return -1;//gestito in manageClientGame
                                } 
                                else if(bytesSent == 0)
                                    return 0; //socket non pronto, gestito in manageClientGame  */ 
                            // break;//esce dal while solo dopo un inserimento avvenuto con successo cancellare
        }
        else if( insert == 0){
            client->isResponding = false; //il nickname è stato ricevuto dal client, passa alla fase di conferma/rifiuto
            //prepara l'invio del messaggio di non ok a client
            //scrive nel buffer send del client
            client->sendBuf.totLen = MSG_LEN;
            client->sendBuf.buffer = malloc(MSG_LEN+1);
            if(client->sendBuf.buffer == NULL)
                //gestione errore propagata
                return -1; 
            strcpy(client->sendBuf.buffer,MSG_NO);
            client->sendBuf.progress = 0;
            /*//messaggio non ok al client cancellare
            int bytesSent = sendAllBytes(client->client_fd, MSG_NO, MSG_LEN);
            if( bytesSent < 0) {
                    perror("Errore in send() del no al nickname");
                    return -1;//gestito in manageClientGame
                } 
            else if(bytesSent == 0)
                return 0;//gestito in manageClientGame*/
        }
        //strcpy(name,nickname);//copia in name il nickname dato dal client cancellare
        //free(nickname);
    }

    //fase di invio del feedback sul nickname già ricevuto
    int bytesSent = sendAllBytes(client,client->sendBuf.buffer,MSG_LEN);
    if(bytesSent <= 0)
        return bytesSent;

    //}       
    return 1;//conclusa correttamente
}
//-------------------------------------------------------------------------------------------------------------

//funzione che manda al client i nomi dei temi disponibili
//parametri: socket del client e nickname
//il nickname serve per un'eventuale endquiz durante il gioco
int sendThemes(struct ClientInfo* client, char* nickname) {
    //manda un messaggio al client con il numero di temi e aspetta un feedback sulla ricezione di quest'ultimo
    printf("in sendThemes prima del send %d\n", current_session.numThemes);// cancellare
    //manda il numero dei temi al client
    int bytesSent = sendInt(client, current_session.numThemes);
    if( bytesSent < 0) {
        perror("Errore in send() del numero di temi");
        deletePlayer(nickname);
        return -1;//gestito in manageClientGame
    }
    else if(bytesSent == 0)
        return 0;
    printf("in sendThemes dopo prima send\n");// cancellare
    //il numero dei temi è stato ricevuto correttamente prosegue mandando il nome di ogni tema, uno per volta         
      for(int i = 0; i < NUM_THEMES ; i++) {
        //controllo se il giocatore ha già giocato l'i-esimo tema
          
        //lock mutex
        pthread_mutex_lock(&lockPlayers);
        
        struct Player* ptr = getPlayer(current_session.players,nickname);
        printf("getPlayer controllo temi\n");// cancellare
        //unlock mutex
        pthread_mutex_unlock(&lockPlayers);
        char* buf;//buffer ausiliaro che contiene la stringa da inviare al client
        
        //se ptr->themePoints[i] != -1 (valore di inizializzazione) vuol dire che il client ha già giocato a quel tema
        //quindi viene inviato -1 invece che la lunghezza della stringa del nome del tema per notificare il client
        if(ptr->themePoints[i] != -1) {
            buf = malloc(strlen(" ")+1);
            if( buf == NULL) {
                perror("Errore nel malloc");
                deletePlayer(nickname);
                return -1;//gestito in manageClientGame
            }
            strcpy(buf," ");//indica il tema non disponibile
                //char* emptyStr = " ";//indica il tema non disponibile cancellare

                /*int bytesSent = sendString(client, emptyStr); cancellare
                if( bytesSent < 0) { //invio della stringa vuota per indicare che non è un tema disponibile
                    perror("Errore in send() del nome del tema");
                    deletePlayer(nickname);
                    return -1;//gestito in manageClientGame
                } 
                else if (bytesSent == 0)
                    return 0;*/
        }
        //client non ha ancora giocato al quiz di quel tema
        //viene inviata la lunghezza della stringa e poi la stringa contenente il nome del tema
        else {
            buf = malloc(strlen(current_session.availableThemes[i].name)+1);
            if( buf == NULL) {
                perror("Errore nel malloc");
                deletePlayer(nickname);
                return -1;//gestito in manageClientGame
            }
            strcpy(buf,current_session.availableThemes[i].name);


            /*int bytesSent = sendString(client,current_session.availableThemes[i].name);
            if( bytesSent < 0) { //invio della stringa (nome dell'i-esimo tema)
                perror("Errore in send() del nome del tema");
                deletePlayer(nickname); cancellare
                return -1;//gestito in manageClientGame
            }
            else if (bytesSent == 0)
                return 0;*/ 
        }
        printf("in sendThemes dopo prima send %s\n",buf);// cancellare
        //invio della stringa buf
        int bytesSent = sendString(client,buf);
        if( bytesSent < 0) { //invio della stringa (nome dell'i-esimo tema)
            perror("Errore in send() del nome del tema");
            deletePlayer(nickname); 
            return -1;//gestito in manageClientGame
        }
        else if (bytesSent == 0)
            return 0;
        printf("in sendThemes dopo send %s\n", buf);// cancellare
     } 
     printf("in sendThemes fine\n");// cancellare
     return 1;//conclusa correttamente
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce la recezione del tema scelto
//parametri: socket del client e nickname
//il nickname serve per un'eventuale endquiz durante il gioco
//ritorna l'indice del tema scelto
int recvThemes(struct ClientInfo* client, char* nickname) {
    //riceve dal client l'indice del tema a cui vuole giocare
    int themeChosen;

    int bytesRec = recvInt(client,&themeChosen);
    if( bytesRec < 0) {
        perror("Disconnessione del client o errore.\n");
        deletePlayer(nickname);
        return -1;//gestito in manageClientGame
    }
    else if(bytesRec == 0)
        return 0; //client non pronto, gestito in ManageClient
    return themeChosen+1;
}

//-------------------------------------------------------------------------------------------------------------

int sendQuestion(struct ClientInfo* client) {
    //invia al client la lunghezza della stringa e poi la stringa contenente la i-esima domanda
    int bytesSent = sendString(client,current_session.availableThemes[client->currentTheme].quiz[client->currentQ].question);
    if( bytesSent < 0) { 
        perror("Errore in send() della domanda");
        deletePlayer(client->nickname);
        return -1;//gestito in manageClientGame
    }
    else if(bytesSent == 0)
        return 0;

    return 1;
}

//-------------------------------------------------------------------------------------------------------------
//funzione che riceve il comando
int recvCommand(struct ClientInfo* client) {
    //il messaggio che riceve è per indicare se il client ha normalmente risposto o richiesto schowscore o endquiz
    //per valutarlo sfrutta la funzione checkCommand
    char msg[MSG_LEN];
    int bytesRec = recvAllBytes(client,msg,MSG_LEN);
    if( bytesRec < 0) {
        printf("Disconnessione del client o errore.\n");
        deletePlayer(client->nickname);
        return -1;//gestito in manageClientGame
    }
    else if( bytesRec == 0)
        return 0; //socket non pronto, gestito in manageClientGame

    //se checkCommand torna true vuol dire che è stata fatta una show score invece di rispondere
    //quindi va ripetuta la domanda precedente, per farlo non incremento client->currentQ
    int command = checkCommand(client,msg); 
    switch(command) {
        case 1:
            return 1;
        break;
        case 2:
            return -1;//endquiz, voglio che il client sia eliminato dal set
        break;
        case 0:
            //il client ha effettivamente dato la risposta
            if(recvResponse(client) == -1)
                return -1;
            else
                return 1;
        break;
    }
    return -1;//errore 
    /*if( command == 1 || command == 2) {//show score o endquiz
        return 1;
    }
    else if
    else if(command == 0) {
        //il client ha effettivamente dato la risposta
        if(recvResponse(client) == -1)
            return -1;
        else
            return 1;
    }
    else    
        return -1;*/
}

//-------------------------------------------------------------------------------------------------------------

int recvResponse(struct ClientInfo* client) {
    
    //RICEZIONE DELLA RISPOSTA E VALUTAZIONE DI ESSA  
    
    //riceve la risposta (sempre ricevendo prima il numero di byte)    recvString
    /*int len = recvStringLen(client->client_fd);
    if(len < 0) {
        perror("Errore nella ricezione della lunghezza della risposta\n");
        return -1;//gestito in manageClientGame
    }
    else if(len == 0)
        return 0;//gestito in manageClientGame

    char* bufR = malloc(len);
    //gestione errore nel malloc
    if(bufR == NULL) {
        perror("Errore nel malloc della risposta\n");
        deletePlayer(client->nickname);
        return -1;//gestito in manageClientGame
    }        */

    //ricezione lunghezza stringa
    uint32_t netLen = 0;
    int lenRecv = recvAllBytes(client,&netLen,sizeof(uint32_t));//riceve la lunghezza della stringa
    if(lenRecv <= 0) 
        return lenRecv; //socket non ancora pronto o errore
    
    uint32_t len = ntohl(netLen); //da network a host
    char* bufR = malloc(len);
    if(bufR == NULL) {
        perror("Disconnessione del client o errore.\n");
        deletePlayer(client->nickname);
        return -1;//gestito in manageClientGame
    }
    int bytesRec = recvAllBytes(client, bufR,len);
    //int bytesRec = recvString(client->client_fd,bufR); cancellare
    if( bytesRec < 0) {
        perror("Disconnessione del client o errore.\n");
        deletePlayer(client->nickname);
        return -1;//gestito in manageClientGame
    }
    else if( bytesRec ==  0)   
        return 0; //socket non pronto, gestito in manageClientGame
    
    //calcolo punteggio
    int p = updatePoints(client->currentQ,bufR,client->currentTheme,client->nickname);
    
    //manda feedback sulla risposta data al client
    //p = 1 se la risposta è giusta
    if(p == 1) {   
        //messaggio corretta al client
        int bytesSent = sendAllBytes(client, MSG_OK, MSG_LEN);
        if( bytesSent < 0) {
            perror("Errore in send() del risposta corretta");
            deletePlayer(client->nickname);
            return -1;//gestito in manageClientGame
        }  
        else if(bytesSent == 0)
            return 0;  
    }
    else {
        //messaggio non corretta al client
        int bytesSent = sendAllBytes(client, MSG_NO, MSG_LEN);
        if(bytesSent < 0) {
            perror("Errore in send() del risposta errata");
            deletePlayer(client->nickname);
            return -1;//gestito in manageClientGame
        } 
        else if(bytesSent == 0)
            return 0;
    }  
    
    //libera la memoria
    free(bufR);

    client->currentQ++;

    return 1;

}

//-------------------------------------------------------------------------------------------------------------

//funzione che data una risposta torna 1 se è giusta o 0 altrimenti 
//aggiornando il punteggio nella relativa struttura dati di current_session
//parametri: numero della domanda (numq), risposta data (bufR), indice del tema scelto, nickname del giocatore
int updatePoints(int numq,char* bufR,int themeChosen,char* nickname) {
    
    //lock sul mutex
    pthread_mutex_lock(&lockPlayers);
    
    struct Player* ptr = getPlayer(current_session.players, nickname);//puntatore al giocatore dato il nickname
    
    if(ptr->themePoints[themeChosen] == -1) //se è la prima domanda 
        ptr->themePoints[themeChosen] = 0;
        
    //controllo se la risposta è corretta utilizzando checkAnswer che torna true (corretta) o false (errata)
    if(checkAnswer(themeChosen, bufR,numq)) {//risposta corretta
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
int checkCommand(struct ClientInfo* client,char* msg) {
    //se ha ricevuto MSG_OK continua normalmente (torna 0)
    //se ha ricevuto MSG_RK rimanda alla funzione doShowScore(client_fd) (torna 1)
    //se ha ricevuto MSG_EX rimanda alla funzione doEndquiz(client_fd) (torna -1)
    if(strcmp(MSG_RK,msg) == 0) {
        doShowScore(client);
        return 1;
    }
    else if(strcmp(MSG_EX,msg) == 0) {
        doEndquiz(client);
        return 2;
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce il comando show score chiamato dal client
//manda al client prima il numero di temi e poi per ognuno di essi la classifica ottenuta con getThemeRankings
//manda sempre la lunghezza della stringa prima della stringa
int doShowScore(struct ClientInfo* client) {
    //manda il numero di temi
    int bytesSent1 = sendAllBytes(client,&current_session.numThemes,sizeof(int));
    if( bytesSent1 < 0) { 
        perror("Errore in send() del numero di classifiche");
        deletePlayer(client->nickname);
        return -1;//gestito in manageClientGame
    }
    else if(bytesSent1 == 0)
        return 0;

    struct Player** rankings = getThemeRankings(client->client_fd);//calcola le classifiche
    if(rankings == NULL) {
        //gestione errore
        perror("Errore nel calcolo della classifica");
        deletePlayer(client->nickname);
        return -1;//gestito in manageClientGame
    }

    struct Player* current_player;
    
    int num_ranked;//intero che contiene il numero di giocatori in una determinata classifica
    for(int i = 0 ; i < NUM_THEMES ; i++) {
        current_player = rankings[i]; //classifica dell'i-esimo tema
        
        //manda il numero di giocatori nella i-esima classifica
        num_ranked = countRanked(rankings,i);
        int bytesSent = sendAllBytes(client,&num_ranked,sizeof(int));
        if( bytesSent < 0) { 
            perror("Errore in send() del numero di giocatori nella classifica");
            deletePlayer(client->nickname);
            return -1;//gestito in manageClientGame
        }
        else if(bytesSent == 0)
            return 0;
        //per ogni giocatore nella classifica invia nickanme e punti
        while(current_player != NULL) {
            //invio nickname del k-esimo classificato dell'i-esimo tema
            int bytesSent2 = sendString(client,current_player->nickname);
            if( bytesSent2 < 0) { 
                perror("Errore in send() del nickname (ranking)");
                deletePlayer(client->nickname);
                return -1;//gestito in manageClientGame
            }
            else if(bytesSent2 == 0)
                return 0;
            //invio del punteggio del giocatore
            int bytesSent3 = sendInt(client,current_player->themePoints[0]);
            if( bytesSent3 < 0) { 
                perror("Errore in send() del punteggio (ranking)");
                deletePlayer(client->nickname);
                return -1;//gestito in manageClientGame
            } 
            else if(bytesSent3 == 0)
                return 0;
            current_player = current_player->next;
        }   
    }

    return 1;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce il comando endquiz chiamato dal client
int doEndquiz(struct ClientInfo* client) {
    //deve eliminare il player da current_session
    deletePlayer(client->nickname);
    
    //resetta le info di ClientInfo
    free(client->nickname);
    client->nickname = NULL;
    client->currentTheme = -1;
    client->currentQ = -1;
    client->isResponding = true;
    client->state = WaitingForNickname;
    return -1;
}

//-------------------------------------------------------------------------------------------------------------
//funzione che garantisce la lettura del numero corretto di bytes dal socket
int recvAllBytes(struct ClientInfo* client, void *buf, uint32_t len) {
    //caso in cui la ricezione non è ancora iniziato -> inizializzo i campi di recvBuf
    if(client->recvBuf.totLen == 0) {
        client->recvBuf.buffer = malloc(len);
        if(client->recvBuf.buffer == NULL)
            return -1; //propagazione della gestione dell'errore
        client->recvBuf.totLen = len;
        client->recvBuf.progress = 0;
    }

    while(client->recvBuf.progress < client->recvBuf.totLen) {
        //caso in cui la ricezione non sia ancora terminato
        int bytesRec = recv(client->client_fd,client->recvBuf.buffer + client->recvBuf.progress,client->recvBuf.totLen-client->recvBuf.progress,0);
        if(bytesRec < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                return 0;//socket non pronto
            return -1;
        } 
        client->recvBuf.progress += bytesRec;
    }

    //se è uscito dal while vuol dire che la ricezione è completa
    memcpy(buf, client->recvBuf.buffer,client->recvBuf.totLen);//copio in buf quanto ricevuto
    //resetta i campi di recvBuf
    free(client->recvBuf.buffer);
    client->recvBuf.buffer = NULL;
    client->recvBuf.totLen = 0;
    client->recvBuf.progress = 0;
    return 1;



  /*uint32_t totRec = 0; cancellare
  uint32_t bytesRec = 0;
  
  while(totRec < len) {
    bytesRec = recv(client_fd,buf+totRec,len-totRec,0);
    if(bytesRec < 0){
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return totRec;//ritorno parziale
        else
            return -1;
    }      
    totRec += bytesRec;
  }
  
  return totRec;*/
}

//-------------------------------------------------------------------------------------------------------------
//funzione che garantisce di mandare il numero corretto di bytes
int sendAllBytes(struct ClientInfo* client, void *buf, uint32_t len) {
    //caso in cui l'invio'non è ancora iniziato -> inizializzo i campi di sendBuf
    if(client->sendBuf.totLen == 0) {
        client->sendBuf.buffer = malloc(len);
        if(client->sendBuf.buffer == NULL)
            return -1; //propagazione della gestione dell'errore
        memcpy(client->sendBuf.buffer, buf,len);
        client->sendBuf.totLen = len;
        client->sendBuf.progress = 0;
    }

    while(client->sendBuf.progress < client->sendBuf.totLen) {
        //caso in cui l'invio' non sia ancora terminato
        int bytesSent = send(client->client_fd,client->sendBuf.buffer + client->sendBuf.progress,client->sendBuf.totLen-client->sendBuf.progress,0);
        if(bytesSent < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                return 0;//socket non pronto
            return -1;
        } 
        client->sendBuf.progress += bytesSent;
    }
    printf("%s\n", client->sendBuf.buffer); //cancellare
    //se è uscito dal while vuol dire che l'invio è completo
    //resetta i campi di recvBuf
    free(client->sendBuf.buffer);
    client->sendBuf.buffer = NULL;
    client->sendBuf.totLen = 0;
    client->sendBuf.progress = 0;
    return 1;
  /*uint32_t totSent = 0; cancellare
  uint32_t bytesSent = 0;
  
  while(totSent < len) {
    bytesSent = send(client_fd,buf+totSent,len-totSent,0);
    if(bytesSent < 0) {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return totSent;//ritorno parziale
        else
            return -1;
    }

    totSent += bytesSent;
  }
  
  return totSent;*/
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce la ricezione di una stringa
//ricevendo prima la lunghezza e successivamente la stringa stessa
//ritorna il puntatore alla stringa cancellare
/*int recvStringLen(int client_fd) {
  //riceve prima la lunghezza della stringa
  uint32_t netLen;
  
  int bytesRec = recvAllBytes(client_fd,&netLen,sizeof(netLen));
  if(bytesRec < 0) 
    return -1;
  
  uint32_t len = ntohl(netLen); //da network a host
  
  return len;
}*/

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce l'invio di una stringa, mandando prima la lunghezza e successivamente la stringa stessa
int sendString(struct ClientInfo* client, void *buf) {
    uint32_t len = (uint32_t)strlen(buf)+1;
    uint32_t netLen = htonl(len); //host to network

    int lenSent = sendAllBytes(client,&netLen,sizeof(netLen));//manda la lunghezza della stringa
    if(lenSent <= 0)
        return lenSent; //errore o invio non finito

    //se arriva a questo punto la lunghezza è stata inviata completamente e correttamente
    //invia stringa
    lenSent = sendAllBytes(client,buf,len);//manda la lunghezza della stringa
    if(lenSent <= 0)
        return lenSent; //errore o invio non finito
    
    return 1;

    //caso in cui non sia ancora stato mandato niente cancellare
   /* if(client->sendBuf.totLen == 0) {
        uint32_t len = (uint32_t)strlen(buf)+1;
        uint32_t netLen = htonl(len); //host to network

        client->sendBuf.totLen = sizeof(uint32_t) + netLen;
        client->sendBuf.buffer = malloc(client->sendBuf.totLen+1);
        if(client->sendBuf.buffer == NULL)
            return -1; //gestito in manageClientGame
        //nel buffer ci scrive sia la lunghezza che la stringa
        memcpy(client->sendBuf.buffer, &netLen, sizeof(uint32_t));//lunghezza della stringa
        memcpy(client->sendBuf.buffer + sizeof(uint32_t), buf, netLen);//stringa stessa
        client->sendBuf.progress = 0;
    }

    //invia la parte del buffer rimanente
    int lenSent = sendAllBytes(client->client_fd,client->sendBuf.buffer+client->sendBuf.progress,client->sendBuf.totLen-client->sendBuf.progress);//manda la lunghezza della stringa

    if(lenSent < 0) {
        //errore 
        free(client->sendBuf.buffer);
        client->sendBuf.totLen = 0;
        client->sendBuf.buffer = NULL;
        client->sendBuf.progress = 0;
        return -1;
    }

    //aggiorno progress nella struttura dati
    client->sendBuf.progress += lenSent;

    //caso in cui l'invio non sia completo
    if(client->sendBuf.progress < client->sendBuf.totLen)
        return 0;

    //caso in cui l'invio sia completato
    free(client->sendBuf.buffer);
    client->sendBuf.totLen = 0;
    client->sendBuf.buffer = NULL;
    client->sendBuf.progress = 0;
    return 1;



  //lunghezza della stringa cancellare
  uint32_t len = (uint32_t)strlen(buf)+1;
  uint32_t netLen = htonl(len); //host to network
  
  int lenSent = sendAllBytes(client->client_fd,&netLen,sizeof(netLen));//manda la lunghezza della stringa
  if(lenSent < 0)
    return -1;   
  
  //manda la stringa
  int bytesSent = sendAllBytes(client->client_fd,buf,len);
  if(bytesSent < 0)
    return -1;
  
  return bytesSent;*/
}

//-------------------------------------------------------------------------------------------------------------
// cancellare
//funzione che gestisce la ricezione di una stringa, ricevendo prima la lunghezza e successivamente la stringa stessa
/*int recvString(struct ClientInfo* client, void **buf) {
    //caso in cui non sia ancora stato ricevuto niente
    if(client->recvBuf.totLen == 0) {
        uint32_t netLen = 0;
        int lenRecv = recvAllBytes(client,&netLen,sizeof(uint32_t));//riceve la lunghezza della stringa
        if(lenRecv <= 0) 
            return lenRecv; //socket non ancora pronto o errore
            
        uint32_t len = ntohl(netLen); //da network a host

        *buf = malloc(len);//buffer dove ricevere la stringa
        if(buf == NULL) {
            //errore 
            return -1;
        }
    }

    //ricezione della stringa


    //riceve la parte del buffer rimanente cancellare
    int lenRecv = recvAllBytes(client->client_fd,client->recvBuf.buffer+client->recvBuf.progress,client->recvBuf.totLen-client->recvBuf.progress);//riceve la stringa

    if(lenRecv < 0) {
        //errore 
        free(client->recvBuf.buffer);
        client->recvBuf.totLen = 0;
        client->recvBuf.buffer = NULL;
        client->recvBuf.progress = 0;
        return -1;
    }
    if(lenRecv == 0)
        return 0;//socket non pronto

    //aggiorno progress nella struttura dati
    client->recvBuf.progress += lenRecv;

    //caso in cui l'invio non sia completo
    if(client->recvBuf.progress < client->recvBuf.totLen)
        return 0;//socket non pronto

    //caso in cui l'invio sia completato

    //scrive in buf la stringa
    *buf = malloc(client->recvBuf.totLen);
    if(buf == NULL) {
        //errore 
        free(client->recvBuf.buffer);
        client->recvBuf.totLen = 0;
        client->recvBuf.buffer = NULL;
        client->recvBuf.progress = 0;
        return -1;
    }
    strcpy(*buf,client->recvBuf.buffer);
    //resetta
    free(client->recvBuf.buffer);
    client->recvBuf.totLen = 0;
    client->recvBuf.buffer = NULL;
    client->recvBuf.progress = 0;
    return 1;
}*/ 

//--------------------------------------------------------------------------------------------------------------------------
int sendInt(struct ClientInfo* client, int val) {
    uint32_t netVal = htonl(val);
    return sendAllBytes(client,&netVal,sizeof(netVal));
}

//--------------------------------------------------------------------------------------------------------------------------
int recvInt(struct ClientInfo* client, int* val) {
    uint32_t netVal;
    int res = recvAllBytes(client,&netVal,sizeof(netVal));

    if(res <= 0)
        return res;

    *val = ntohl(netVal);
    return 1;
}


