#include "./utility/clientUtility.h"

int main(int argc, char *argv[]) {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[MSG_SIZE];
    char *server_ip = "127.0.0.1";
    int port = 8080;//??

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

    showMainMenu(client_fd);
    showQuizThemes(client_fd);
}
