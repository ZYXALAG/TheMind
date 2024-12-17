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

void afficher_cartes_joueur(int main_joueur[], int taille_main, int joueur) {
    printf("Cartes dans la main du joueur %d:\n",joueur);
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
    jeu->tour_actuel = 1;

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
    char message[1024] = "103 Vos cartes : ";
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
                printf("Des données ont été reçues du joueur %d\n", i+1);
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
    initialiser_jeu(&jeu); //initialiser la struct jeu
    int mains_joueurs[MAX_JOUEURS][MAX_CARTES];
    distribuer_cartes(&jeu, mains_joueurs); //distribuer les cartes aux joueurs [23 0 0 0 ... 0]
    while (1){ //boucle de manche
        int playershandTEMP[MAX_JOUEURS][jeu.tour_actuel]; //creer une copie de la main des joueurs [23]
        for (int i = 0; i < MAX_JOUEURS; i++){
            for (int j = 0; j < jeu.tour_actuel; j++){
                    playershandTEMP[i][j] = mains_joueurs[i][j];   
                }
                
            }
            int cartes_jouees[MAX_CARTES] = {};
            int index_cartes_jouees = 0;
            int nb_carte_joueur[MAX_JOUEURS] ;
            for(int i = 0; i < MAX_JOUEURS; i++){
                nb_carte_joueur[i] = 0;
            }
            for (int i = 0; i < MAX_JOUEURS; i++)
            {     
                afficher_cartes_joueur(playershandTEMP[i], sizeof(playershandTEMP[i])/sizeof(int),i+1);
            }

            
            for (int i = 0; i < MAX_JOUEURS; i++){
                sleep(0.5);
                envoyer_cartes_joueur(sockets_joueurs[i], playershandTEMP[i], jeu.tour_actuel);
                int current_time = time(NULL);
                sleep(0.5);
                char messagetime[256];
                snprintf(messagetime, sizeof(messagetime), "200 %d\n", current_time+2);
                envoyer_message(sockets_joueurs[i], messagetime);
                sleep(1);
                char message[256];
                snprintf(message, sizeof(message), "101 Vous pouvez jouer. Jouez une carte.\n");
                envoyer_message(sockets_joueurs[i], message);
            }
            while (1){ //boucle de jeu

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
                for (int i = 0; i < MAX_JOUEURS; i++)
                {
                    char message1[256];
                    snprintf(message1, sizeof(message1), "105 Joueur %d a joué : %d\n", curretplayer + 1, carte_jouee);
                    envoyer_message(sockets_joueurs[i], message1);
                    sleep(1);
                    char message[256];
                    snprintf(message, sizeof(message), "105 partie terminée vous avez perdu\n");
                    envoyer_message(sockets_joueurs[i], message);
                }
                goto fin;
            } 
            nb_carte_joueur[curretplayer]++;
            cartes_jouees[index_cartes_jouees++] = carte_jouee;
            printf("Cartes jouées : %d \n", index_cartes_jouees);
            int newtime = time(NULL);   
            for (int i = 0; i < MAX_JOUEURS; i++)
            {
                if (i!=curretplayer)
                {
                    char messagetime[256];
                    snprintf(messagetime, sizeof(messagetime), "200 %d\n", newtime+2);
                    envoyer_message(sockets_joueurs[i], messagetime);
                    char message[256];
                    snprintf(message, sizeof(message), "105 Joueur %d a joué : %d\n", curretplayer + 1, carte_jouee);
                    envoyer_message(sockets_joueurs[i], message);
                }
                
            }
            

            if (index_cartes_jouees == jeu.tour_actuel * MAX_JOUEURS) {
                for (int i = 0; i < MAX_JOUEURS; i++)
                {
                    char message[256];
                    snprintf(message, sizeof(message), "108 Félicitation vous avez reussis la manche\n");
                    envoyer_message(sockets_joueurs[i], message);
                    printf("Félicitation vous avez reussis la manche\n");
                }
                break;
            }



            printf("carte jouée par le joueur %d : %d\n", curretplayer+1, nb_carte_joueur[curretplayer]);
            if (nb_carte_joueur[curretplayer] <jeu.tour_actuel)
            {
                char message[256];
                snprintf(message, sizeof(message), "101 Vous pouvez jouer. Jouez une carte.\n");
                envoyer_message(sockets_joueurs[curretplayer], message);
            
            } else{
                char message[256];
                snprintf(message, sizeof(message), "106 Vous avez joué toutes vos cartes. Attendez la fin de la manche.\n");
                envoyer_message(sockets_joueurs[curretplayer], message);
            }
            


        }
        jeu.tour_actuel++;
        distribuer_cartes(&jeu, mains_joueurs);

    }
    
fin:
    printf("Stat de partie\n");
    printf("Nombre de round gagné %d \n", jeu.tour_actuel);

    // Fermeture des sockets
    for (int i = 0; i < MAX_JOUEURS; i++) {
        close(sockets_joueurs[i]);
    }
    close(serveur_fd);
    return 0;
}