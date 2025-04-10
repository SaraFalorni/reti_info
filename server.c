#include "./utility/serverUtility.h"
#include "./utility/utility.h"


#include "serverUtility/QAfunctions.h"
#include "serverUtility/rankingsFunctions.h"
#include "serverUtility/PlayersFunctions.h"
#include "serverUtility/gameFunctions.h"
#include "serverUtility/generalFunctions.h"


int main(int argc, char *argv[]) {
    //inizializza la sessione di gioco;
    
    init_session();

    int server_fd;//, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int port = 8080;//??

    if(argc == 2) {
        port = atoi(argv[1]);
    }

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    //server_addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore nella conversione dell'indirizzo IP");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, BACKLOG) == -1) {
        perror("Errore nella listen");
        exit(EXIT_FAILURE);
    }


    while(1) {
        int *client_fd = malloc(sizeof(int));
        //accettare connessione in arrivo
        if((*client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len)) == -1) {
            perror("Errore nell'accettazione della connesione");
            continue;
        }

        pthread_t tid;
        if(pthread_create(&tid, NULL, client_handler, client_fd) != 0) {
          perror("Errore nella creazione del thread");
          close(*client_fd);
          free(client_fd);
        }
        pthread_detach(tid);

    }

          

    close(server_fd);
    return 0;
}


