#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

#define MAX_CARTES_MAINS 5

void recevoir_message(int sock, int *cartes, int *nb_cartes) {
    char buffer[BUFFER_SIZE];
    int octets_recus;

    // Recevoir le message du serveur
    while ((octets_recus = read(sock, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[octets_recus] = '\0'; // Ajouter le caractère de fin de chaîne
        printf("Message reçu : %s\n", buffer);

        // Vérifier si le message contient une carte
        if (strncmp(buffer, "CARTE:", 6) == 0) {
            int carte;
            sscanf(buffer + 6, "%d", &carte);
            if (*nb_cartes < MAX_CARTES_MAINS) {
                cartes[*nb_cartes] = carte;
                (*nb_cartes)++;
                printf("Carte ajoutée à la main : %d\n", carte);
            } else {
                printf("Main pleine, impossible d'ajouter la carte : %d\n", carte);
            }
        } else {
            printf("Message du serveur : %s\n", buffer);
        }
    }

    if (octets_recus == 0) {
        printf("Le serveur s'est déconnecté.\n");
    } else {
        perror("Erreur lors de la réception");
    }
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    int cartes[MAX_CARTES_MAINS];
    int nb_cartes = 0;

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

    // Recevoir les messages du serveur et ajouter les cartes à la main
    recevoir_message(sock, cartes, &nb_cartes);

    // Fermer le socket du client
    close(sock);
    return 0;
}