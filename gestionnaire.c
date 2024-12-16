#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/select.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_JOUEURS 2
#define TAILLE_BUFFER 1024

#define MAX_CARTES 100

typedef struct {
    int cartes[MAX_CARTES]; // Cartes restantes dans le jeu
    int nombre_cartes;
    int tour_actuel;
} Jeu;

void afficher_cartes_joueur(int main_joueur[], int taille_main) {
    printf("Cartes dans la main du joueur:\n");
    for (int i = 0; i < taille_main; i++) {
        printf("Carte %d: %d\n", i + 1, main_joueur[i]);
    }
}

void erreur(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void shuffle(Jeu *jeu){
    srand(time(NULL));
    for (int i = 0; i < jeu->nombre_cartes; i++) {
        int j = rand() % jeu->nombre_cartes;
        int temp = jeu->cartes[i];
        jeu->cartes[i] = jeu->cartes[j];
        jeu->cartes[j] = temp;
    }
}

void initialiser_jeu(Jeu *jeu) {
    jeu->nombre_cartes = MAX_CARTES;
    jeu->tour_actuel = 2;

    // Générer les cartes de 1 à MAX_CARTES
    for (int i = 0; i < MAX_CARTES; i++) {
        jeu->cartes[i] = i + 1;
    }
    shuffle(jeu);
}

// Distribuer les cartes aux joueurs
void distribuer_cartes(Jeu *jeu, int mains_joueurs[MAX_JOUEURS][MAX_CARTES]) {
    int cartes_par_joueur = jeu->tour_actuel;
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
void envoyer_message(int socket, const char *message) {
    if (send(socket, message, strlen(message), 0) == -1) {
        erreur("Erreur d'envoi du message");
    }
}


void envoyer_cartes_joueur(int socket, int *cartes, int nombre_cartes) {
    char message[1024] = "Vos cartes : ";
    char buffer[16];

    // Boucle pour ajouter chaque carte au message
    for (int i = 0; i < nombre_cartes; i++) {
        if (cartes[i] != -1) {  // Vérifie si la carte est valide
            snprintf(buffer, sizeof(buffer), "%d ", cartes[i]);
            
            printf("carte ajouté au buffer %s\n", buffer);
            strcat(message, buffer);
            printf("carte ajouté au message %s\n", message);
        }
    }

    // Envoie le message au joueur
    strcat(message, "\n");
    envoyer_message(socket, message);
}

int verifier_donnees_joueurs(int sockets[], int nb_joueurs) {
    fd_set readfds;
    int max_sd = 0;
    struct timeval timeout;

    // Initialiser le set de descripteurs de fichiers
    FD_ZERO(&readfds);

    // Ajouter les sockets des joueurs au set
    for (int i = 0; i < nb_joueurs; i++) {
        FD_SET(sockets[i], &readfds);
        if (sockets[i] > max_sd) {
            max_sd = sockets[i];
        }
    }

    // Définir le délai d'attente (timeout)
    timeout.tv_sec = 5000;  // 5 secondes
    timeout.tv_usec = 0;

    // Utiliser select pour surveiller les sockets
    int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

    if (activity < 0) {
        perror("Erreur lors de l'appel à select");
        return -1;
    } else if (activity == 0) {
        printf("Aucune donnée reçue dans le délai imparti\n");
        return 0;
    } else {
        // Vérifier quel socket a des données disponibles
        for (int i = 0; i < nb_joueurs; i++) {
            if (FD_ISSET(sockets[i], &readfds)) {
                printf("Des données ont été reçues du joueur %d\n", i);
                return sockets[i];
            }
        }
    }

    return -1;
}

void recevoir_message(int socket[], char *buffer, size_t taille) {
    memset(buffer, 0, taille);
    int activesocket = verifier_donnees_joueurs(socket, MAX_JOUEURS);
    if (recv(activesocket, buffer, taille - 1, 0) <= 0) {
        erreur("Erreur de réception du message");
    }
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
    for (int i = 0; i < MAX_JOUEURS; i++)
    {
        char messagesocket[256];
        snprintf(messagesocket, sizeof(messagesocket), "100 %d.\n", i);
        envoyer_message(sockets_joueurs[i], messagesocket);
    }

    Jeu jeu;
    initialiser_jeu(&jeu);
    int mains_joueurs[MAX_JOUEURS][MAX_CARTES];
    distribuer_cartes(&jeu, mains_joueurs);

    while (1){
        int condition = 1;
        int playershandTEMP[MAX_JOUEURS][jeu.tour_actuel];
        for (int i = 0; i < MAX_JOUEURS; i++){
            for (int j = 0; j < jeu.tour_actuel; j++){
                    playershandTEMP[i][j] = mains_joueurs[i][j];   
                }
                
            }
            int cartes_jouees[MAX_CARTES] = {};
            int index_cartes_jouees = 0;

            afficher_cartes_joueur(playershandTEMP[0], sizeof(playershandTEMP[0])/sizeof(int));
            afficher_cartes_joueur(playershandTEMP[1], sizeof(playershandTEMP[1])/sizeof(int));
            
            for (int i = 0; i < MAX_JOUEURS; i++){
                envoyer_cartes_joueur(sockets_joueurs[i], playershandTEMP[i], jeu.tour_actuel);
                sleep(0.5);
                char message[256];
                snprintf(message, sizeof(message), "101 Vous pouvez jouer. Jouez une carte.\n");
                envoyer_message(sockets_joueurs[i], message);
            }
            while (condition){

                char reponse[256];

                recevoir_message(sockets_joueurs, reponse, sizeof(reponse));
                char *token = strtok(reponse, " ");
                int curretplayer = atoi(token);
                token = strtok(NULL, " ");
                int carte_jouee = atoi(token);
                
                // Vérification si le joueur possède la carte
            int carte_valide = 0;
            for (int j = 0; j < jeu.tour_actuel; j++) {
                if (playershandTEMP[curretplayer][j] == carte_jouee) {
                    playershandTEMP[curretplayer][j] = -1; // Retirer la carte de la main
                    carte_valide = 1;
                    break;
                }
            }

            if (!carte_valide) {
                char message[256];
                printf("Joueur %d a tenté de jouer une carte invalide : %d\n", curretplayer + 1, carte_jouee);
                snprintf(message, sizeof(message), "Carte invalide. Veuillez jouer une carte que vous possédez.\n");
                envoyer_message(sockets_joueurs[curretplayer], message);
                continue;
            }

            printf("Joueur %d a joué : %d\n", curretplayer + 1, carte_jouee);

            // Vérification de l'ordre des cartes
            if (index_cartes_jouees > 0 && carte_jouee < cartes_jouees[index_cartes_jouees - 1]) {
                printf("Ordre incorrect ! La partie est terminée.\n");
                goto fin;
            } else
            {
                char message[256];
                snprintf(message, sizeof(message), "101 Vous pouvez jouer. Jouez une carte.\n");
                envoyer_message(sockets_joueurs[curretplayer], message);
            }
            
            cartes_jouees[index_cartes_jouees++] = carte_jouee;
            printf("Cartes jouées : %d \n", index_cartes_jouees);


            if (index_cartes_jouees == jeu.tour_actuel * MAX_JOUEURS) {
                printf("Félicitation vous avez reussis la manche\n");
                condition = 0;
            }
            
            

        }
        jeu.tour_actuel++;
        distribuer_cartes(&jeu, mains_joueurs);

    }
    
fin:
    // Fermeture des sockets
    for (int i = 0; i < MAX_JOUEURS; i++) {
        close(sockets_joueurs[i]);
    }
    close(serveur_fd);
    return 0;
}