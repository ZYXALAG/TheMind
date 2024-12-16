#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_CARTES 100

void erreur(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

// Fonction pour vérifier si une carte est dans la main du robot
int carte_dans_main(int carte, int *cartes, int nombre_cartes) {
    for (int i = 0; i < nombre_cartes; i++) {
        if (cartes[i] == carte) {
            return i; // Retourne l'indice de la carte
        }
    }
    return -1; // Carte non trouvée
}

int main() {
    int serveur_fd;
    struct sockaddr_in serveur_addr;
    char buffer[1024] = {0};
    int cartes[MAX_CARTES] = {0}; // Cartes dans la main du robot
    int nombre_cartes = 0;

    // Création de la socket
    if ((serveur_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        erreur("Erreur de création de la socket");
    }

    // Configuration de l'adresse du serveur
    serveur_addr.sin_family = AF_INET;
    serveur_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serveur_addr.sin_addr) <= 0) {
        erreur("Adresse invalide ou non supportée");
    }

    // Connexion au serveur
    if (connect(serveur_fd, (struct sockaddr *)&serveur_addr, sizeof(serveur_addr)) < 0) {
        erreur("Erreur de connexion au serveur");
    }

    printf("Connecté au serveur ! En attente des instructions...\n");

    while (1) {
        // Réception des messages du serveur
        int bytes_recus = recv(serveur_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_recus <= 0) {
            printf("Connexion terminée par le serveur.\n");
            break;
        }
        buffer[bytes_recus] = '\0';
        printf("Message du serveur : %s\n", buffer);

        // Vérifier si le message contient la liste des cartes
        if (strncmp(buffer, "Vos cartes :", 12) == 0) {
            // Extraire les cartes envoyées par le serveur
            char *cartes_str = buffer + 12; // Ignore "Vos cartes : "
            char *token = strtok(cartes_str, " ");
            nombre_cartes = 0;
            while (token != NULL) {
                cartes[nombre_cartes++] = atoi(token);
                token = strtok(NULL, " ");
            }
            continue;
        }

        // Si c'est le tour du robot
        if (strncmp(buffer, "Votre tour", 10) == 0) {
            // Jouer une carte valide
            for (int i = 0; i < nombre_cartes; i++) {
                if (cartes[i] != -1) {
                    snprintf(buffer, sizeof(buffer), "%d", cartes[i]);
                    send(serveur_fd, buffer, strlen(buffer), 0);
                    printf("Robot joue la carte : %s\n", buffer);
                    cartes[i] = -1; // Retirer la carte jouée
                    break;
                }
            }
        }
    }

    close(serveur_fd);
    return 0;
}
