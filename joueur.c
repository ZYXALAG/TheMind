#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_CARTES 1

// Fonction pour afficher les cartes du joueur
void afficher_cartes(int *cartes, int nombre_cartes) {
    printf("Vos cartes : ");
    for (int i = 0; i < nombre_cartes; i++) {
        if (cartes[i] != -1) {  // Si la carte est valide
            printf("%d ", cartes[i]);
        }
    }
    printf("\n");
}

// Fonction pour vérifier si une carte est dans la main du joueur
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
    char buffer[1024] = {};
    int cartes[MAX_CARTES] = {}; // Cartes dans la main du joueur
    int nombre_cartes = 0;

    // Création de la socket
    if ((serveur_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Erreur de création de la socket");
        exit(1);
    }

    // Configuration de l'adresse du serveur
    serveur_addr.sin_family = AF_INET;
    serveur_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serveur_addr.sin_addr) <= 0) {
        perror("Adresse invalide ou non supportée");
        exit(1);
    }

    // Connexion au serveur
    if (connect(serveur_fd, (struct sockaddr *)&serveur_addr, sizeof(serveur_addr)) < 0) {
        perror("Erreur de connexion au serveur");
        exit(1);
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
            afficher_cartes(cartes, nombre_cartes);
            continue;
        }

        // Si c'est le tour du joueur
        if (strncmp(buffer, "Votre tour", 10) == 0) {
            int carte_choisie;

            // Demander à l'utilisateur de choisir une carte valide
            while (1) {
                printf("Choisissez une carte à jouer : ");
                scanf("%d", &carte_choisie);

                // Vérifier si la carte est valide (présente dans la main du joueur)
                int indice = carte_dans_main(carte_choisie, cartes, nombre_cartes);
                if (indice != -1) {
                    // Retirer la carte de la main et informer le serveur
                    cartes[indice] = -1;
                    snprintf(buffer, sizeof(buffer), "%d", carte_choisie);
                    send(serveur_fd, buffer, strlen(buffer), 0);
                    printf("Vous avez joué la carte %d.\n", carte_choisie);
                    break;  // Quitter la boucle de sélection de carte
                } else {
                    printf("Carte invalide. Veuillez choisir une carte dans votre main.\n");
                }
            }
        }
    }

    close(serveur_fd);
    return 0;
}
