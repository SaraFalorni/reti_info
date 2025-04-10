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
    //char nickname[MAXCHAR_NICKNAME]; da cancellare
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
        if(send_all_bytes(client_fd, nickname, MAXCHAR_NICKNAME) <= 0) {
            perror("Errore in send() del nickname");
            exit(EXIT_FAILURE);
        }
        
        //ricezione risposta del server
        if(recv(client_fd, msg, MSG_LEN, 0) == -1) { 
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
        
    if(recv_all_bytes(client_fd, &num_themes, sizeof(num_themes)) == -1) {
        perror("Errore in recv() per il numero di temi disponibili");
        exit(EXIT_FAILURE);
    }
     
    //messaggio di ok al server per sincronizzazione
    if(send_all_bytes(client_fd, MSG_OK, MSG_LEN) <= 0) {
        perror("Errore in send() dell'ok alla ricezione del numero di temi\n");
        exit(EXIT_FAILURE);
    }   
    
    //server inizia a mandare i nomi dei temi disponibili 
    char *themes[num_themes];
  
    //riceve i nomi dei temi e li salva in un array di stringhe (*themes)
    for(int i = 0; i < num_themes ; i++) {
        //riceve la lunghezza della stringa
        size_t len;
        if(recv_all_bytes(client_fd,&len, sizeof(size_t)) <= 0) {
            perror("Errore nella ricezione della lunghezza della stringa");
            exit(EXIT_FAILURE);
        }
        //se il server ha mandato lunghezza pari a -1 vuol dire che quel tema non è disponibile (perchè ci ha già giocato)
        //viene contraddistinto dalla stringa "0"
        if(len == -1) {
            themes[i] = "0";
        }
        else {
            char* buf = safe_malloc(len);//stringa (nome tema)
            if(recv_all_bytes(client_fd,buf,len) <= 0) {
                perror("Errore nella ricezione della stringa");
                exit(EXIT_FAILURE);
            }
          
            themes[i] = safe_malloc(len);
            strncpy(themes[i], buf, len);//copia il nome del tema nell'array
            free(buf);//libera la memoria
        }
        
    }
  
    
    int choice = 0;//intero che conterrà l'indice del tema scelto
    printf("\nQuiz disponibili\n");

    for(int i = 0 ; i < NUM_SEPARATOR; i++)
          printf("+");
          
    int n = 0;//intero per "tradurre" l'indice dato al client con quello del server
    
    for(int i = 0; i < num_themes ; i++) {
        if(strcmp(themes[i],"0") == 0 ) 
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
    for(int i = 0 ; i < 20; i++)
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
        if(strcmp(themes[i],"0") == 0  && i < choice) 
            choice++;
    }
    choice = choice - 1;
    
    //manda la scelta fatta al server
    if(send_all_bytes(client_fd, &choice, sizeof(int)) <= 0) {
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
    //per ogni domanda
    for(int i = 0; i < NUM_Q ; i++) {
        //riceve la lunghezza della domanda
        size_t len;
        if(recv_all_bytes(client_fd,&len, sizeof(size_t)) <= 0) {
            perror("Errore nella ricezione della lunghezza della domanda");
            exit(EXIT_FAILURE);
        }
        //riceve la stringa della domanda
        char* buf = safe_malloc(len);
        if(recv_all_bytes(client_fd,buf,len) <= 0) {
            perror("Errore nella ricezione della domanda");
            exit(EXIT_FAILURE);
        }
        
        printf("\n%s\n",buf);//stampa la domanda
        
        char risp[MAXCHAR_LINE];
        strcpy(risp,"0");
        
        //ripulisce stdin evitando la doppia stampa
        if(i == 0) {
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
        
        remove_spaces(risp); //elimina eventuali spazi iniziali o finali
        
        //se check_comand torna true vuol dire che è stata fatta una show score invece di rispondere, quindi va ripetuta la domanda precedente
        if(checkComand(client_fd,risp,nickname)) {
            i--;
            continue;
        }

    }//chiude for
    printf("fine quiz in nickname %s\n",nickname);//cancellare ?
}

//-------------------------------------------------------------------------------------------------------------

//comunica con il server per verificare la correttezza della risposta
//nel parametro risp c'è la risposta data
void send_answer(int client_fd,char* risp) {
  //manda la lunghezza della risposta al server
    int lenR = strlen(risp)+1;
    if(send_all_bytes(client_fd, &lenR, sizeof(lenR)) <= 0) { //invio lunghezza della risposta
        perror("Errore in send() della lunghezza della risposta");
        exit(EXIT_FAILURE);
    } 
    //manda la stinga
    if(send_all_bytes(client_fd,risp,lenR) <= 0) { 
        perror("Errore in send() della domanda");
        exit(EXIT_FAILURE);
    }
   
    //se la stringa è corretta riceve MSG_OK dal server altrimenti la risposta è sbagliata
    //stampa a video di conseguenza
    char msg[MSG_LEN];
    
    if(recv_all_bytes(client_fd,msg,MSG_LEN) <= 0) {
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
bool checkComand(int client_fd,char* risp,char* nickname) {
    //manda MSG_RK al server se il client ha richiesto show score
    //esegue la funzione showScore
    if(strcmp(SHOWSCORE,risp) == 0) {
        if(send_all_bytes(client_fd,MSG_RK,MSG_LEN) <= 0) { 
            perror("Errore in send() del show score");
            exit(EXIT_FAILURE);
        }
        showScore(client_fd);
        return true;
    }
    //manda MSG_EX al server se il client ha richiesto endquiz
    //esegue la funzione endGame
    else if(strcmp(ENDQUIZ,risp) == 0) {
        if(send_all_bytes(client_fd,MSG_EX,MSG_LEN) <= 0) { 
            perror("Errore in send() del endquiz");
            exit(EXIT_FAILURE);
        }
        endGame(client_fd,nickname);
        return false;
    }
    //manda MSG_OK al server se il client ha risposto alla domanda
    //esegue la funzione send_answer
    else {
        if(send_all_bytes(client_fd,MSG_OK,MSG_LEN) <= 0) { 
            perror("Errore in send() del normale continuo di gioco");
            exit(EXIT_FAILURE);
        }
        send_answer(client_fd,risp);
        return false;
    }
}

//-------------------------------------------------------------------------------------------------------------

//esegue la funzione showscore come descritto nelle specifiche
void showScore(int client_fd) { 
   //riceve il numero di temi, quindi il numero di classifiche massimo da stampare
   int num_themes;
   if(recv_all_bytes(client_fd,&num_themes, sizeof(int)) <= 0) {
        perror("Errore nella ricezione del numero di temi (ranking)");
        exit(EXIT_FAILURE);
   }  

   for(int i = 0; i < num_themes; i++) {
     //riceve il numero di giocatori nella i-esima classifica
      int num_ranked;
      if(recv_all_bytes(client_fd,&num_ranked, sizeof(int)) <= 0) {
            perror("Errore nella ricezione del numero di giocatori in classifica");
            exit(EXIT_FAILURE);
      } 
      //stampa la classifica solo se c'è almeno un giocatore
      if(num_ranked > 0)
          printf("\nPunteggio tema %d\n",i+1);
      //per ogni giocatore in classifica stampa nickname e punti
      for(int k = 0; k < num_ranked ; k++) {
          //riceve la lunghezza del nickname
          size_t len;
          if(recv_all_bytes(client_fd,&len, sizeof(size_t)) <= 0) {
              perror("Errore nella ricezione della lunghezza del nickname (ranking)");
              exit(EXIT_FAILURE);
          }
          //riceve il nickname
          char* nickname = safe_malloc(len);
          if(recv_all_bytes(client_fd,nickname,len) <= 0) {
              perror("Errore nella ricezione del nickname (ranking)");
              exit(EXIT_FAILURE);
          }
          //riceve il punteggio del k-esimo classificato dell'i-esimo tema
          int points;
          if(recv_all_bytes(client_fd,&points, sizeof(int)) <= 0) {
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
    int len = strlen(nickname)+1;
    //manda al server il nickname per permettere al server di cancellare il corrispondente Player 
    
    if(send_all_bytes(client_fd, &len, sizeof(int)) <= 0) { //invio lunghezza del nickname
        perror("Errore in send() della lunghezza del nickname (endGame)");
        exit(EXIT_FAILURE);
    } 
    
    if(send_all_bytes(client_fd, nickname, len) <= 0) {
        perror("Errore in send() del nickname");
        exit(EXIT_FAILURE);
    }
    strcpy(nickname, "");
    
    //torna al main menu
    showMainMenu(client_fd,nickname);
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
int recv_all_bytes(int client_fd, void *buf, int len) {
  int tot_rec = 0;
  int bytes_rec = 0;
  
  while(tot_rec < len) {
    bytes_rec = recv(client_fd,buf+tot_rec,len - tot_rec,0);
    if(bytes_rec <= 0) {
        //gestione errore
        printf("Connessione interrotta dal server.\n");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    tot_rec += bytes_rec;
  }
  
  return tot_rec;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che garantisce di mandare il numero corretto di bytes dal socket
//e gestisce l'uscita nel caso di disconnessione del server
int send_all_bytes(int client_fd, void *buf, int len) {
  int tot_sent = 0;
  int bytes_sent = 0;
  
  while(tot_sent < len) {
    bytes_sent = send(client_fd,buf+tot_sent,len - tot_sent,0);
    if(bytes_sent <= 0) {
        //gestione errore
        if(errno == 0) 
            printf("Connessione interrotta dal server.\n");
        else
            perror("errore nella send");
      
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    tot_sent += bytes_sent;
  }
  
  return tot_sent;
}

//-------------------------------------------------------------------------------------------------------------

void remove_spaces(char* str) {
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
void* safe_malloc(size_t len) {
  void *p = malloc(len);
  
  if(p == NULL) {
      perror("Errore nel malloc");
      exit(EXIT_FAILURE);
  }
  
  return p;
}
