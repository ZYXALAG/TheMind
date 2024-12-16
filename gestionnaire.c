#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024

// Fonction pour gérer la communication avec un client
void gestion_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Recevoir le message du client
    while ((bytes_received = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_received] = '\0'; // Ajouter le caractère de fin de chaîne
        printf("Message reçu : %s\n", buffer);

        // Répondre au client
        const char *message = "Message reçu avec succès";
        write(client_socket, message, strlen(message));
    }

    if (bytes_received == 0) {
        printf("Le client s'est déconnecté.\n");
    } else {
        perror("Erreur lors de la réception");
    }

    // Fermer la connexion avec le client
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Créer le socket du serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Configurer l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Écoute sur toutes les interfaces réseau
    server_addr.sin_port = htons(PORT);  // Port de communication

    // Associer l'adresse et le port au socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors du bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Écouter les connexions entrantes
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Erreur lors de l'écoute");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    // Accepter les connexions des clients
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Erreur lors de l'acceptation de la connexion");
            continue;
        }

        printf("Nouveau client connecté.\n");

        // Gérer la communication avec le client
        gestion_client(client_socket);
    }

    // Fermer le socket du serveur
    close(server_socket);
    return 0;
}
