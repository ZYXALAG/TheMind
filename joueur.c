#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CARTES_MAINS 52

void recevoir_message(int sock, int *cartes, int *nb_cartes) {
    char buffer[BUFFER_SIZE];
    int octets_recus;

    // Recevoir le message du serveur
    while ((octets_recus = read(sock, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[octets_recus] = '\0'; // Ajouter le caractère de fin de chaîne

        // Vérifier si le message contient une carte
        if (strncmp(buffer, "CARTE:", 6) == 0) {
            int carte;
            sscanf(buffer + 6, "%d", &carte);
            cartes[*nb_cartes] = carte;
            (*nb_cartes)++;
            printf("Carte ajoutée à la main : %d\n", carte); // Message de débogage
        } else if (strncmp(buffer, "FIN_MANCHE", 10) == 0) {
            // Fin de la manche
            printf("Fin de la manche\n");
            break;
        } else {
            printf("Message du serveur : %s\n", buffer);
        }
    }

    if (octets_recus == 0) {
        printf("Le serveur s'est déconnecté.\n");
        exit(EXIT_SUCCESS);
    } else if (octets_recus < 0) {
        perror("Erreur lors de la réception");
    }
}

void envoyer_fin_round(int sock) {
    const char *message = "fin\n";
    if (write(sock, message, strlen(message)) < 0) {
        perror("Erreur lors de l'envoi du message de fin de round");
    }
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    int cartes[MAX_CARTES_MAINS];
    int nb_cartes = 0;

    // Créer le socket du client
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convertir l'adresse IP du serveur
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Adresse IP invalide");
        exit(EXIT_FAILURE);
    }

    // Se connecter au serveur
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de connexion au serveur");
        exit(EXIT_FAILURE);
    }

    // Recevoir les messages du serveur
    while (1) {
        recevoir_message(sock, cartes, &nb_cartes);
        
        // Afficher les cartes de la main
        printf("Main du joueur : ");
        for (int i = 0; i < nb_cartes; i++) {
            printf("%d ", cartes[i]);
        }
        printf("\n");

        // Envoyer le message de fin de round au serveur
        envoyer_fin_round(sock);

        // Réinitialiser le nombre de cartes pour le prochain round
        nb_cartes = 0;
    }

    // Fermer le socket
    close(sock);

    return 0;
}