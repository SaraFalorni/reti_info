#include "./utility/serverUtility.h"
#include "./utility/utility.h"


#include "serverUtility/QAfunctions.h"
#include "serverUtility/rankingsFunctions.h"
#include "serverUtility/PlayersFunctions.h"
#include "serverUtility/gameFunctions.h"
#include "serverUtility/generalFunctions.h"


int main(int argc, char *argv[]) {
    //inizializza la sessione di gioco;
    
    initSession();
    
    int numSets = 0;//numero di ClientSet presenti 
    struct ClientSet sets[MAX_CLIENTSETS];

    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int port = 8080;

    if(argc == 2) {
        port = atoi(argv[1]);
    }

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
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
        if(client_fd == NULL) {
            //errore nel malloc
            perror("Errore nel malloc");
            exit(EXIT_FAILURE);
        } 

        //accettare connessione in arrivo
        if((*client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len)) == -1) {
            perror("Errore nell'accettazione della connessione");
            continue;
        }
        //socket non bloccanti
        int f = fcntl(*client_fd, F_GETFL, 0);
        if( f == -1) {
            perror("errore nell'ottenimento dei flag");
            close(*client_fd);
            exit(EXIT_FAILURE);
        }
        if(fcntl(*client_fd, F_GETFL, f | O_NONBLOCK) == -1) {
            perror("errore nell'ottenimento dei flag");
            close(*client_fd);
            exit(EXIT_FAILURE);
        }

        //assegna il client all'ultimo set creato non ancora pieno
        //ritorna true se c'è un posto libero, false altrimenti
        if(!assignClientToSet(*client_fd,sets,&numSets)) {
            printf("non c'è più posto per partecipare, ritenta più tardi");
            break;
        }
    }

    close(server_fd);
    return 0;
}


