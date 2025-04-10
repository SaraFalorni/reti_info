#include "./utility/clientUtility.h"

void sigpipe_handler() {
//ignora SIG_PIPE, gestito dalle funzioni recv_all_bytes e send_all_bytes
  signal(SIGPIPE, SIG_IGN);
}

int main(int argc, char *argv[]) {
    sigpipe_handler();

    int client_fd;
    struct sockaddr_in server_addr;
    char *server_ip = "127.0.0.1";

    if(argc != 2) {
        //qualche printf che spieghi
        exit(EXIT_FAILURE);
    }

    if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));

    if(inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Errore nella conversione dell'indirizzo IP");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    if(connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore nella connessione al server");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    
    char nickname[MAXCHAR_NICKNAME] = "";
    
    
    while(1) {
        //menu mostrato all'inizio del gioco
        int choice = showMainMenu(client_fd,nickname);//1 se vuole giocare, 2 se vuole uscire
        
        if(choice == 1) {
          chooseNickname(client_fd,nickname);
          while(strlen(nickname) > 0)
          {
            showQuizThemes(client_fd);
            playGame(client_fd, nickname);
          }
          //nel caso in cui sia stato fatto endQuiz deve essere ristampato il menu iniziale
          //il nickname deve essere svuotato
          strcpy(nickname,"");
        }
        else if(choice == 2)
            exitGame(client_fd);
    }
    close(client_fd);
    return 0;
}
