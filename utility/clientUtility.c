#include "clientUtility.h"

//funzione che stampa il menù principale
int showMainMenu(int client_fd,char* nickname) {
    int choice = 0;
    printf("Trivia Quiz\n");

    for(int i = 0 ; i < NUM_SEPARATOR; i++)
        printf("+");
    printf("\nMenù:\n1 - Comincia una sessione di Trivia\n2 - Esci\n");
    for(int i = 0 ; i < NUM_SEPARATOR; i++)
        printf("+");
    do {
        printf("\nLa tua scelta:  ");


        scanf("%d", &choice);

        if(choice != 1 && choice != 2) {
            printf("scelta non valida, devi inserire un numero tra 1 e 2\n");

            while(getchar() != '\n');
        }

    } while(choice != 1 && choice != 2);

    return choice;    
}

//-------------------------------------------------------------------------------------------------------------

//funzione che chiede l'input e comunica con il server per registrare il nuovo utente
//in caso di nickanem già utilizzato richiede il nickname
void chooseNickname(int client_fd,char* nickname) {
    printf("Trivia Quiz\n");

    for(int i = 0 ; i < NUM_SEPARATOR; i++)
        printf("+");
        
    char msg[MSG_LEN] = "";
    while(1) {
      
        printf("\nScegli un nickname (deve essere univoco) :\n ");
        
        //se preme invio senza inserire niente 
        while(fgets(nickname, MAXCHAR_NICKNAME, stdin) == NULL || nickname[0] == '\n') {
            continue; 
        }  
        
        //nickname acquisito
        
       //prima di inviare il nickname al server toglie '\n'
        if(nickname[strlen(nickname)-1] == '\n')
            nickname[strlen(nickname)-1] = '\0';
          
        //manda il nickname scelto al server che risponde con MSG_OK se è utilizzabile, MSG_NO altrimenti  
        if(sendString(client_fd, nickname) <= 0) {
            perror("Errore in send() del nickname");
            exit(EXIT_FAILURE);
        }
        
        //ricezione risposta del server
        if(recvAllBytes(client_fd, msg, MSG_LEN) <= 0) { 
            perror("Errore in recv() di conferma nickname");
            exit(EXIT_FAILURE);
        }
        
        //nickname utilizzabile, giocatore registrato correttamento
        if(strcmp(msg,MSG_OK) == 0) {
            break;
        }
        else //nickname già in uso, continua il ciclo 
           printf("\nNickname già utilizzato\n");
    }
}

//-------------------------------------------------------------------------------------------------------------

//mostra a video i temi disponibili nella sessione (quelli a cui non ha ancora partecipato)
//e manda la decisione al server sotto forma di indice (corrispondente al numero di riga nel file ./txt/indiceTemi.txt
void showQuizThemes(int client_fd) {
    //riceve il numero di temi disponibili dal server
    int num_themes;
    printf("prima di ricevere il num themes\n"); //cancellare
    if(recvInt(client_fd, &num_themes) <= 0) {
        perror("Errore in recv() per il numero di temi disponibili");
        exit(EXIT_FAILURE);
    }
    printf("dopo di ricevere il num themes %d\n",num_themes); //cancellare
    //server inizia a mandare i nomi dei temi disponibili 
    char *themes[num_themes];
  
    //riceve i nomi dei temi e li salva in un array di stringhe (*themes)
    for(int i = 0; i < num_themes ; i++) {
        //riceve la stringa con il nome del i-esimo tema
        uint32_t len = recvStringLen(client_fd); //riceve la lunghezza della stringa
        themes[i] = safeMalloc(len);
        printf("prima di ricevere il  theme\n"); //cancellare
        recvAllBytes(client_fd,themes[i],len);//riceve la stringa       
        printf("dopo di ricevere il num theme %s\n", themes[i]); //cancellare
        //se il server ha mandato una stringa vuota " "
        //vuol dire che quel tema non è disponibile (perchè ci ha già giocato)       
    }
  
    
    int choice = 0;//intero che conterrà l'indice del tema scelto
    printf("\nQuiz disponibili\n");

    for(int i = 0 ; i < NUM_SEPARATOR; i++)
          printf("+");
          
    int n = 0;//intero per "tradurre" l'indice dato al client con quello del server
    
    for(int i = 0; i < num_themes ; i++) {
        if(strcmp(themes[i]," ") == 0 ) 
           n++;//incrementato per ogni tema non disponibile
        else 
            printf("\n%d - %s", i-n+1, themes[i]);//l'indice da mostrare a video è la differenza fra l'indice vero (a cui fa riferimento il server) e il numero di temi non disponibili sommato 1
    }

    //caso in cui ha già giocato a tutti i quiz disponibili
    if(n == num_themes) {
        printf("\nHai già partecipato a tutti i quiz disponibili, arrivederci!\n");
        exitGame(client_fd);//esce dal gioco
    }
        
    printf("\n");  
    for(int i = 0 ; i < NUM_SEPARATOR; i++)
         printf("+");
    //richiede l'indice finche non ne ottiene uno valido 
    do {
         printf("\nLa tua scelta:  ");


          scanf("%d", &choice);

          if(choice > (num_themes-n) || choice <= 0) {
              printf("scelta non valida, devi inserire un numero tra quelli associati ai temi disponibili\n");

              while(getchar() != '\n');
        }

    } while(choice > (num_themes-n) || choice <= 0);
    
    //"traduzione" dell'indice da mandare al server
   
    for(int i = 0; i < num_themes ; i++) {
        if(strcmp(themes[i]," ") == 0  && i < choice) 
            choice++;
    }
    choice = choice - 1;
    
    //manda la scelta fatta al server
    if(sendInt(client_fd, choice) <= 0) {
        perror("Errore in send() del tema scelto");
        exit(EXIT_FAILURE);
    }
    
    //stampa titolo del quiz scelto
    printf("\nQuiz - %s\n", themes[choice]);
    for(int i = 0 ; i < NUM_SEPARATOR; i++)
         printf("+");
    printf("\n");
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce il quiz
//mostra a video le domande, prende le risposte comunicando con il server per ottenere le prime e verificare le seconde
void playGame(int client_fd,char* nickname) {
    int com = 0;
    //per ogni domanda
    for(int i = 0; i < NUM_Q ; i++) {
        //riceve la stringa della domanda
        uint32_t len = recvStringLen(client_fd);//riceve la lunghezza della stringa
        char* buf = safeMalloc(len);
        
        recvAllBytes(client_fd,buf,len);//riceve la stringa
                
        printf("\n%s\n",buf);//stampa la domanda
        
        char risp[MAXCHAR_LINE];
        strcpy(risp,"0");
        
        //ripulisce stdin evitando la doppia stampa
        if(i == 0 ) {
            int c;
            while((c = getchar()) != '\n' && c != EOF);
        }
        
        //aspetta la risposta
        do {
            printf("\nRisposta: ");
        }while(fgets(risp, MAXCHAR_LINE, stdin) == NULL || risp[0] == '\n');
        
        printf("\n"); 
        
        //sostituisce '\n' con '\0'
        if(strlen(risp) > 0 && risp[strlen(risp) - 1] == '\n')
           risp[strlen(risp)-1] = '\0';
        
        free(buf);//libera la memoria
        
        removeSpaces(risp); //elimina eventuali spazi iniziali o finali
        
        //se check_comand torna 1 vuol dire che è stata fatta una show score invece di rispondere, quindi va ripetuta la domanda precedente
        com = checkComand(client_fd,risp,nickname);
        if(com == 1) {
            i--;
            continue;
        }//se torna 2 vuol dire che è stata fatta endquiz, deve uscire dal flusso
        else if(com == 2)
          return;

    }//chiude for
}

//-------------------------------------------------------------------------------------------------------------

//comunica con il server per verificare la correttezza della risposta
//nel parametro risp c'è la risposta data
void sendAnswer(int client_fd,char* risp) {
    //manda la stringa
    if(sendString(client_fd,risp) <= 0) { 
        perror("Errore in send() della domanda");
        exit(EXIT_FAILURE);
    }
   
    //se la stringa è corretta riceve MSG_OK dal server altrimenti la risposta è sbagliata
    //stampa a video di conseguenza
    char msg[MSG_LEN];
    
    if(recvAllBytes(client_fd,msg,MSG_LEN) <= 0) {
        perror("Errore in recv() del feedback sulla risposta");
        exit(EXIT_FAILURE);
    }
     
    if(strcmp(msg,MSG_OK) == 0) {
        printf("Risposta Corretta\n");
    }
    else
        printf("Risposta Errata\n");
}

//-------------------------------------------------------------------------------------------------------------

//ogni volta che il client sta partecipando ad un quiz può richiedere i comandi showscore o endquiz
//questa funzione gestisce questa possibilità
int checkComand(int client_fd,char* risp,char* nickname) {
    //manda MSG_RK al server se il client ha richiesto show score
    //esegue la funzione showScore
    if(strcmp(SHOWSCORE,risp) == 0) {
        if(sendAllBytes(client_fd,MSG_RK,MSG_LEN) <= 0) { 
            perror("Errore in send() del show score");
            exit(EXIT_FAILURE);
        }
        showScore(client_fd);
        
        return 1;
    }
    //manda MSG_EX al server se il client ha richiesto endquiz
    //esegue la funzione endGame
    else if(strcmp(ENDQUIZ,risp) == 0) {
        if(sendAllBytes(client_fd,MSG_EX,MSG_LEN) <= 0) { 
            perror("Errore in send() del endquiz");
            exit(EXIT_FAILURE);
        }
        endGame(client_fd,nickname);
        return 2;
    }
    //manda MSG_OK al server se il client ha risposto alla domanda
    //esegue la funzione sendAnswer
    else {
        if(sendAllBytes(client_fd,MSG_OK,MSG_LEN) <= 0) { 
            perror("Errore in send() del normale continuo di gioco");
            exit(EXIT_FAILURE);
        }
        sendAnswer(client_fd,risp);
        return 0;
    }
}

//-------------------------------------------------------------------------------------------------------------

//esegue la funzione showscore come descritto nelle specifiche
void showScore(int client_fd) { 
   //riceve il numero di temi, quindi il numero di classifiche massimo da stampare
   int num_themes;
   if(recvAllBytes(client_fd,&num_themes, sizeof(int)) <= 0) {
        perror("Errore nella ricezione del numero di temi (ranking)");
        exit(EXIT_FAILURE);
   }  

   for(int i = 0; i < num_themes; i++) {
     //riceve il numero di giocatori nella i-esima classifica
      int num_ranked;
      if(recvAllBytes(client_fd,&num_ranked, sizeof(int)) <= 0) {
            perror("Errore nella ricezione del numero di giocatori in classifica");
            exit(EXIT_FAILURE);
      } 
      //stampa la classifica solo se c'è almeno un giocatore
      if(num_ranked > 0)
          printf("\nPunteggio tema %d\n",i+1);
      //per ogni giocatore in classifica stampa nickname e punti
      for(int k = 0; k < num_ranked ; k++) {
          //riceve il nickname
          uint32_t len = recvStringLen(client_fd);
          char* nickname = safeMalloc(len);
          
          recvAllBytes(client_fd,nickname,len);
          
          //riceve il punteggio del k-esimo classificato dell'i-esimo tema
          int points;
          if(recvInt(client_fd,&points) <= 0) {
              perror("Errore nella ricezione del punteggio (ranking)");
              exit(EXIT_FAILURE);
          }
          
          printf("- %s %d\n",nickname,points);
          
          free(nickname); //libera la memoria
      }
   }
}

//-------------------------------------------------------------------------------------------------------------

//esegue la funzione endquiz come descritto nelle specifiche
void endGame(int client_fd,char* nickname) {
    strcpy(nickname, "");
    
    //torna al main menu
   close(client_fd);
}

//-------------------------------------------------------------------------------------------------------------

//funzione di uscita del client
void exitGame(int client_fd) {
    close(client_fd);
    exit(EXIT_SUCCESS);
}

//-------------------------------------------------------------------------------------------------------------

//funzione che garantisce la lettura del numero corretto di bytes dal socket
//e gestisce l'uscita nel caso di disconnessione del server
uint32_t recvAllBytes(int client_fd, void *buf,uint32_t len) {
  uint32_t totRec = 0;
  uint32_t bytesRec = 0;
  
  while(totRec < len) {
    bytesRec = recv(client_fd,buf+totRec,len-totRec,0);
    if(bytesRec < 0) {
        manageErrRecv(client_fd);
    }
    else if(bytesRec == 0)
        printf("Il server è al momento disconnesso\n");
    totRec += bytesRec;
  }
  
  return totRec;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che garantisce di mandare il numero corretto di bytes
//e gestisce l'uscita nel caso di disconnessione del server

  uint32_t sendAllBytes(int client_fd, void *buf, uint32_t len) {
    uint32_t totSent = 0;
    uint32_t bytesSent = 0;
    
    while(totSent < len) {
      bytesSent = send(client_fd,buf+totSent,len-totSent,0);
      if(bytesSent < 0) {
          //gestione errore
          manageErrSend(client_fd);// in generalFunctions
      }
      else if(bytesSent == 0)
        printf("Il server è al momento disconnesso\n");
      totSent += bytesSent;
    }
    
    return totSent;
  }

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce la ricezione della lunghezza della  stringa
//ritorna la lunghezza della stringa
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

//-------------------------------------------------------------------------------------------------------------

void removeSpaces(char* str) {
  //rimozione spazi iniziali
  while(isspace((unsigned char)* str)) 
    str++;
    
  char* fine = str + strlen(str) -1;
  
  //rimozione spazi finali
  while(fine > str && isspace((unsigned char)* str)) 
    fine--;
    
  *(fine + 1) = '\0';
}

//-------------------------------------------------------------------------------------------------------------
//gestisce possibile errori in malloc
void* safeMalloc(int len) {
  void *p = malloc(len);
  
  if(p == NULL) {
      perror("Errore nel malloc");
      exit(EXIT_FAILURE);
  }
  
  return p;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestsce errori dovuti a send o improvvise disconnessione del server
void manageErrSend(int client_fd) {
    //gestione errore
   if(errno == ECONNRESET)
        printf("Connessione interrotta dal server.\n");
    else
        perror("errore nella send");
    
    close(client_fd);
    exit(EXIT_FAILURE);
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestsce errori dovuti a send o improvvise disconnessione del server
void manageErrRecv(int client_fd) {
    //gestione errore
    if(errno == ECONNRESET)
        printf("Connessione interrotta dal server.\n");
    else
        perror("errore nella recv");
    
    close(client_fd);
    exit(EXIT_FAILURE);
}

//--------------------------------------------------------------------------------------------------------------------------
int sendInt(int client_fd, int val) {
    uint32_t netVal = htonl(val);
    return sendAllBytes(client_fd,&netVal,sizeof(netVal));
}

//--------------------------------------------------------------------------------------------------------------------------
int recvInt(int client_fd, int* val) {
    uint32_t netVal;
    int res = recvAllBytes(client_fd,&netVal,sizeof(netVal));

    if(res <= 0)
        return res;

    *val = ntohl(netVal);
    return 1;
}
