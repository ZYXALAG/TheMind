#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 12345
#define MAX_JOUEURS 2
#define MAX_CARTES 10
#define MAX_SIZE_MAIN = 1

// Structure pour le jeu
typedef struct {
    int cartes[MAX_CARTES]; // Cartes restantes dans le jeu
    int nombre_cartes;
    int tour_actuel;
} Jeu;

void erreur(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

// Initialiser le jeu
void initialiser_jeu(Jeu *jeu) {
    jeu->nombre_cartes = MAX_CARTES;
    jeu->tour_actuel = 1;

    // Générer les cartes de 1 à MAX_CARTES
    for (int i = 0; i < MAX_CARTES; i++) {
        jeu->cartes[i] = i + 1;
    }

    // Mélanger les cartes
    srand(time(NULL));
    for (int i = 0; i < MAX_CARTES; i++) {
        int j = rand() % MAX_CARTES;
        int temp = jeu->cartes[i];
        jeu->cartes[i] = jeu->cartes[j];
        jeu->cartes[j] = temp;
    }
}

// Distribuer les cartes aux joueurs
void distribuer_cartes(Jeu *jeu, int cartes_par_joueur, int mains_joueurs[MAX_JOUEURS][MAX_CARTES]) {
    for (int i = 0; i < MAX_JOUEURS; i++) {
        for (int j = 0; j < cartes_par_joueur; j++) {
            if (jeu->nombre_cartes > 0) {
                mains_joueurs[i][j] = jeu->cartes[--jeu->nombre_cartes];  // Distribution de cartes
            } else {
                mains_joueurs[i][j] = -1; // Indiquer qu'il n'y a plus de cartes
            }
        }
    }
}

// Envoyer un message à un client
void envoyer_message(int socket, const char *message) {
    if (send(socket, message, strlen(message), 0) == -1) {
        erreur("Erreur d'envoi du message");
    }
}

// Recevoir un message d'un client
void recevoir_message(int socket, char *buffer, size_t taille) {
    memset(buffer, 0, taille);
    if (recv(socket, buffer, taille - 1, 0) <= 0) {
        erreur("Erreur de réception du message");
    }
}

// Envoyer les cartes d'un joueur
void envoyer_cartes_joueur(int socket, int *cartes, int nombre_cartes) {
    char message[1024] = "Vos cartes : ";
    char buffer[16];

    // Boucle pour ajouter chaque carte au message
    for (int i = 0; i < nombre_cartes; i++) {
        if (cartes[i] != -1) {  // Vérifie si la carte est valide
            snprintf(buffer, sizeof(buffer), "%d ", cartes[i]);
            strcat(message, buffer);
        }
    }

    // Envoie le message au joueur
    strcat(message, "\n");
    envoyer_message(socket, message);
}

int main() {
    int serveur_fd, nouvelle_socket;
    struct sockaddr_in adresse;
    int addrlen = sizeof(adresse);
    int sockets_joueurs[MAX_JOUEURS];

    // Création de la socket
    if ((serveur_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        erreur("Erreur de création de la socket");
    }

    // Configuration de l'adresse
    adresse.sin_family = AF_INET;
    adresse.sin_addr.s_addr = INADDR_ANY;
    adresse.sin_port = htons(PORT);

    // Liaison de la socket à l'adresse
    if (bind(serveur_fd, (struct sockaddr *)&adresse, sizeof(adresse)) < 0) {
        erreur("Erreur de liaison (bind)");
    }

    // Écoute des connexions entrantes
    if (listen(serveur_fd, MAX_JOUEURS) < 0) {
        erreur("Erreur d'écoute (listen)");
    }

    printf("Gestionnaire en attente de joueurs sur le port %d...\n", PORT);

    int joueurs_connectes = 0;
    while (joueurs_connectes < MAX_JOUEURS) {
        // Accepter une connexion
        if ((nouvelle_socket = accept(serveur_fd, (struct sockaddr *)&adresse, (socklen_t *)&addrlen)) < 0) {
            erreur("Erreur d'acceptation de connexion");
        }

        sockets_joueurs[joueurs_connectes] = nouvelle_socket;
        printf("Joueur %d connecté !\n", joueurs_connectes + 1);
        joueurs_connectes++;
    }

    printf("Tous les joueurs sont connectés. Le jeu peut commencer.\n");

    // Initialisation du jeu
    Jeu jeu;
    initialiser_jeu(&jeu);

    // Distribution des cartes pour le premier tour
    int mains_joueurs[MAX_JOUEURS][MAX_CARTES] = {};
    distribuer_cartes(&jeu, jeu.tour_actuel, mains_joueurs);

    // Boucle de jeu
    int cartes_jouees[MAX_CARTES] = {0};
    int index_cartes_jouees = 0;

    while (1) {
        for (int i = 0; i < MAX_JOUEURS; i++) {
            // Envoyer les cartes disponibles au joueur
            envoyer_cartes_joueur(sockets_joueurs[i], mains_joueurs[i], jeu.tour_actuel);

            char message[256];
            char reponse[256];

            // Informer le joueur de son tour
            snprintf(message, sizeof(message), "Votre tour. Jouez une carte.\n");
            envoyer_message(sockets_joueurs[i], message);

            // Recevoir la carte jouée
            recevoir_message(sockets_joueurs[i], reponse, sizeof(reponse));
            int carte_jouee = atoi(reponse);

            // Vérification si le joueur possède la carte
            int carte_valide = 0;
            for (int j = 0; j < jeu.tour_actuel; j++) {
                if (mains_joueurs[i][j] == carte_jouee) {
                    mains_joueurs[i][j] = -1; // Retirer la carte de la main
                    carte_valide = 1;
                    break;
                }
            }

            if (!carte_valide) {
                printf("Joueur %d a tenté de jouer une carte invalide : %d\n", i + 1, carte_jouee);
                snprintf(message, sizeof(message), "Carte invalide. Veuillez jouer une carte que vous possédez.\n");
                envoyer_message(sockets_joueurs[i], message);
                i--; // Rejouer le même joueur
                continue;
            }

            printf("Joueur %d a joué : %d\n", i + 1, carte_jouee);

            // Vérification de l'ordre des cartes
            if (index_cartes_jouees > 0 && carte_jouee < cartes_jouees[index_cartes_jouees - 1]) {
                printf("Ordre incorrect ! La partie est terminée.\n");
                goto fin;
            }

            cartes_jouees[index_cartes_jouees++] = carte_jouee;
        }

        // Préparer le tour suivant
        jeu.tour_actuel++;
        distribuer_cartes(&jeu, jeu.tour_actuel, mains_joueurs);
    }

fin:
    // Fermeture des sockets
    for (int i = 0; i < MAX_JOUEURS; i++) {
        close(sockets_joueurs[i]);
    }
    close(serveur_fd);
    return 0;
}
