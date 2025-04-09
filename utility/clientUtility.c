#include "clientUtility.h"

void showMainMenu(int client_fd) {
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

    switch(choice) {
        case 1:
            chooseNickname(client_fd);
            break;
        case 2:
            exitGame(client_fd);
            break;
    }    
}

void chooseNickname(int client_fd) {
    char nickname[MAXCHAR_NICKNAME];
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
    
    //nickname inserito
    
   //prima di inviare il nickname al server tolgo '\n'
    if(nickname[strlen(nickname)-1] == '\n')
      nickname[strlen(nickname)-1] = '\0';
    //manda il nickname scelto al server che risponde con MSG_OK se è utilizzabile, MSG_NO altrimenti  
    if(send(client_fd, nickname, MAXCHAR_NICKNAME, 0) == -1) {
      perror("Errore in send() del nickname");
      exit(EXIT_FAILURE);
    }
    
    //ricezione risposta del server
    if(recv(client_fd, msg, MSG_LEN, 0) == -1) {
      perror("Errore in recv() di conferma nickname");
      exit(EXIT_FAILURE);
    }
    
    //nickname accettabile
    if(strcmp(msg,MSG_OK) == 0) {
      break;
    }
    else //nickname già in uso 
      printf("Nickname già utilizzato\n");
  }
}


void showQuizThemes(int client_fd) {
//riceve il numero di temi disponibili dal server
  int num_themes;
  
  ssize_t bytes_received = recv(client_fd, &num_themes, sizeof(num_themes), 0);
  
  if(bytes_received == -1) {
    perror("Errore in recv() per il numero di temi disponibili");
    exit(EXIT_FAILURE);
  }
   
  //messaggio di ok al server per sincronizzazione
  if(send(client_fd, MSG_OK, MSG_LEN, 0) == -1) {
    perror("Errore in send() dell'ok alla ricezione del numero di temi\n");
    exit(EXIT_FAILURE);
  }   
  
  //server inizia a mandare i nomi dei temi disponibili 
  char *themes[num_themes];
  
  for(int i = 0; i < num_themes ; i++) {
    int len;
    if(recv_all_bytes(client_fd,&len, sizeof(int)) <= 0) {
      perror("Errore nella ricezione della lunghezza della stringa");
      exit(EXIT_FAILURE);
    }
    
    if(len == -1) {
      themes[i] = "0";
    }
    else {
      char* buf = malloc(len);
      if(recv_all_bytes(client_fd,buf,len) <= 0) {
        perror("Errore nella ricezione della stringa");
        exit(EXIT_FAILURE);
      }
    
      themes[i] = malloc(len);
      strncpy(themes[i], buf, len);
      free(buf);
    }
    
  }
  
  int choice = 0;
    printf("Quiz disponibili\n");

    for(int i = 0 ; i < NUM_SEPARATOR; i++)
        printf("+");
    int n = 0;//intero per "tradurre" l'indice dato al client con quello del server
    for(int i = 0; i < num_themes ; i++) {
      if(strcmp(themes[i],"0") == 0 ) 
        n++;
      else 
        printf("\n%d - %s", i-n+1, themes[i]);
    }
    
    //caso in cui ha già giocato a tutti i quiz disponibili
    //FARE FUNZIONE DI FINE GIOCO!!!!!!!!!!!!!!!!!!!!
    
    printf("\n");  
    for(int i = 0 ; i < 20; i++)
        printf("+");
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
    if(send(client_fd, &choice, sizeof(int), 0) == -1) {
      perror("Errore in send() del tema scelto");
      exit(EXIT_FAILURE);
    }
    
    //stampa titolo del quiz
    printf("\nQuiz - %s\n", themes[choice]);
    for(int i = 0 ; i < NUM_SEPARATOR; i++)
        printf("+");
    printf("\n");
}

void playGame(int client_fd) {
  for(int i = 0; i < NUM_Q ; i++) {
    int len;
    if(recv_all_bytes(client_fd,&len, sizeof(int)) <= 0) {
      perror("Errore nella ricezione della lunghezza della domanda");
      exit(EXIT_FAILURE);
    }
    
    char* buf = malloc(len);
    if(recv_all_bytes(client_fd,buf,len) <= 0) {
      perror("Errore nella ricezione della domanda");
      exit(EXIT_FAILURE);
    }
    
    printf("\n%s\n",buf);
    
    char risp[MAXCHAR_LINE];
    strcpy(risp,"0");
    
    //ripulisce stdin evitando la doppia stampa
    if(i == 0) {
      int c;
      while((c = getchar()) != '\n' && c != EOF);
    }
    
    //risposta
    do {
      printf("\nRisposta: ");
    }while(fgets(risp, MAXCHAR_LINE, stdin) == NULL || risp[0] == '\n');
    printf("\n"); 
    
    //sostituisco '\n' con '\0'
    if(strlen(risp) > 0 && risp[strlen(risp) - 1] == '\n')
      risp[strlen(risp)-1] = '\0';
    
    free(buf);
    
    remove_spaces(risp); //elimina eventuali spazi iniziali o finali
    
    checkComand(client_fd,risp); 

  }//chiude for
}

void send_answer(int client_fd,char* risp) {
  //manda la risposta al server
    int lenR = strlen(risp)+1;
    if(send(client_fd, &lenR, sizeof(int),0)== -1) { //invio lunghezza della risposta
        perror("Errore in send() della lunghezza della risposta");
        exit(EXIT_FAILURE);
      } 
      if(send(client_fd,risp,lenR,0)== -1) { 
        perror("Errore in send() della domanda");
        exit(EXIT_FAILURE);
      }
     
    char msg[MSG_LEN];
    //server dice se la risposta è corretta o meno
    if(recv_all_bytes(client_fd,msg,MSG_LEN) <= 0) {
      perror("Errore in recv() del feedback sulla risposta");
      exit(EXIT_FAILURE);
    }
     
    if(strcmp(msg,MSG_OK) == 0) {
      printf("\nRisposta Corretta\n");
    }
    else
      printf("\nRisposta Errata\n");
}

//ogni volta che il client sta partecipando ad un quiz può richiedere i comandi showscore o endquiz. questa funzione gestisce questa possibilità
void checkComand(int client_fd,char* risp) {
  //manda MSG_OK al server se il client ha risposto alla domanda
  //manda MSG_RK al server se il client ha richiesto show score
  //manda MSG_EX al server se il client ha richiesto endquiz
  if(strcmp(SHOWSCORE,risp) == 0) {
    if(send(client_fd,MSG_RK,MSG_LEN,0)== -1) { 
      perror("Errore in send() del show score");
      exit(EXIT_FAILURE);
    }
    showScore(client_fd);
  }
  else if(strcmp(ENDQUIZ,risp) == 0) {
    if(send(client_fd,MSG_EX,MSG_LEN,0)== -1) { 
      perror("Errore in send() del endquiz");
      exit(EXIT_FAILURE);
    }
    exitGame(client_fd);
  }
  else {
    if(send(client_fd,MSG_OK,MSG_LEN,0)== -1) { 
      perror("Errore in send() del normale continuo di gioco");
      exit(EXIT_FAILURE);
    }
    send_answer(client_fd,risp);
  }
  return;
}

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
  if(num_ranked > 0)
    printf("\nPunteggio tema %d\n",i+1);
  for(int k = 0; k < num_ranked ; k++) {
  //riceve la lunghezza del nickname
    int len;
    if(recv_all_bytes(client_fd,&len, sizeof(int)) <= 0) {
      perror("Errore nella ricezione della lunghezza del nickname (ranking)");
      exit(EXIT_FAILURE);
    }
    //riceve il nickname
    char* nickname = malloc(len);
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
  }
 }
}




void exitGame(int client_fd) {
    printf("exit game\n");
}

