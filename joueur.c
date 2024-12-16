#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char message[BUFFER_SIZE];

    // Créer le socket du client
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Configurer l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Se connecter au serveur
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Erreur lors de la connexion au serveur");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connecté au serveur %s:%d\n", SERVER_IP, PORT);

    // Demander un message à l'utilisateur et l'envoyer au serveur
    printf("Entrez un message à envoyer au serveur : ");
    fgets(message, sizeof(message), stdin);
    send(sock, message, strlen(message), 0);

    // Recevoir la réponse du serveur
    int bytes_received = recv(sock, message, sizeof(message) - 1, 0);
    if (bytes_received > 0) {
        message[bytes_received] = '\0';  // Ajouter le caractère de fin de chaîne
        printf("Réponse du serveur : %s\n", message);
    }

    // Fermer le socket
    close(sock);
    return 0;
}
