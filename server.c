#include "./utility/serverUtility.h"

int main(int argc, char *argv[]) {
    //inizializza la sessione di gioco;
    struct Session current_session;
    init_session(&current_session);

    int server_fd, client_fd;
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
        //accettare connessione in arrivo
        if((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len)) == -1) {
            perror("Errore nell'accettazione della connesione");
            continue;
        }

        printf("client connesso..\n");
        
        //il primo msg che riceve è il nickname
        char nickname[MAXCHAR_NICKNAME];
        get_nickname(client_fd,&current_session,nickname);
        //una volta registrato il nuovo giocatore invia i temi disponibili
        send_themes(client_fd, &current_session, nickname);
    }

          

    close(server_fd);
}


