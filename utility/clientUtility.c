#include "clientUtility.h"

void showMainMenu(int client_fd) {
    int choice = 0;
    printf("Trivia Quiz\n");

    for(int i = 0 ; i < 20; i++)
        printf("+");
    printf("\nMenù:\n1 - Comincia una sessione di Trivia\n2 - Esci\n");
    for(int i = 0 ; i < 20; i++)
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
            exitGame();
            break;
    }    
}

void chooseNickname(int client_fd) {
    char nickname[MAXCHAR_NICKNAME];
    printf("Trivia Quiz\n");

    for(int i = 0 ; i < 20; i++)
        printf("+");
        
  char msg[MSG_LEN];
  while(strcmp(MSG_NO, msg) || msg[0] == '\n') {
    while(fgets(nickname, MAXCHAR_NICKNAME, stdin) == NULL || nickname[0] == '\n') {
        printf("\nScegli un nickname (deve essere univoco) :\n ");
    }    
    
    if(send(client_fd, nickname, MAXCHAR_NICKNAME, 0) == -1) {
      perror("Errore in send() del nickname");
      exit(EXIT_FAILURE);
    }

    ssize_t bytes_received = recv(client_fd, msg, MSG_LEN, 0);
    
    if(bytes_received == -1) {
      perror("Errore in recv() di conferma nickname");
      exit(EXIT_FAILURE);
    }
    
    if(strcmp(msg,MSG_OK)) {
      printf("nickname ok\n");
      break;
    }
  }    
}

void exitGame() {
    printf("exit game\n");
}

void showQuizThemes(int client_fd) {
//riceve il numero di temi disponibili dal server
  int num_themes;
  
  ssize_t bytes_received = recv(client_fd, &num_themes, sizeof(num_themes), 0);
  
  if(bytes_received == -1) {
    perror("Errore in recv() per il numero di temi disponibili");
    exit(EXIT_FAILURE);
  }
  printf("numero temi ricevuti client %d\n",num_themes);
   
  //messaggio di ok al server per sincronizzazione
  if(send(client_fd, MSG_OK, MSG_LEN, 0) == -1) {
    perror("Errore in send() dell'ok alla ricezione del numero di temi\n");
    exit(EXIT_FAILURE);
  }   
  printf("client manda segnale ricezione\n");
  /*
  else {
    //messaggio non ok al server
    if(send(client_fd, MSG_NO, MSG_LEN, 0) == -1) {
      perror("Errore in send() del no alla ricezione del numero di temi");
      exit(EXIT_FAILURE);
    } */
  
  printf("numero di temi ok\n"); //da cancellare
  //server inizia a mandare i nomi dei temi disponibili 
  char *themes[num_themes];
  
  for(int i = 0; i < num_themes ; i++) {
    int len;
    if(recv_all_bytes(client_fd,&len, sizeof(int)) <= 0) {
      perror("Errore nella ricezione della lunghezza della stringa");
      exit(EXIT_FAILURE);
    }
    
    printf("lunghezza della stringa del ciclo %d: %d",i,len);
    
    char* buf = malloc(len);
    if(recv_all_bytes(client_fd,buf,len) <= 0) {
      perror("Errore nella ricezione della stringa");
      exit(EXIT_FAILURE);
    }
    
    printf("stringa del ciclo %d: %s",i,buf);
    themes[i] = malloc(len);
    strncpy(themes[i], buf, len);
    free(buf);
  }
  
  int choice = 0;
    printf("Quiz disponibili\n");

    for(int i = 0 ; i < 20; i++)
        printf("+");
    
    for(int i = 0; i < num_themes ; i++) {
      printf("\n%d - %s", i+1, themes[i]);
    }
    printf("\n");  
    for(int i = 0 ; i < 20; i++)
        printf("+");
    do {
        printf("\nLa tua scelta:  ");


        scanf("%d", &choice);

        if(choice > num_themes || choice <= 0) {
            printf("scelta non valida, devi inserire un numero tra quelli associati ai temi disponibili\n");

            while(getchar() != '\n');
        }

    } while(choice > num_themes || choice <= 0);
    choice -= 1;
    printf("scelta fatta (ancora da inviare al server) : %d cioè %s\n", choice, themes[choice]); 
    
    //manda la scelta fatta al server
    if(send(client_fd, &choice, sizeof(int), 0) == -1) {
      perror("Errore in send() del tema scelto");
      exit(EXIT_FAILURE);
    }
}

void playGame(int client_fd) {
  
}
