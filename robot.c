#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CARTES 100

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

void chat_loop(int server_socket, int carte_choisie, int* returnarr) {
    fd_set readfds;
    int max_sd = server_socket;
    struct timeval timeout;
    char buffer[BUFFER_SIZE];

    // Initialiser le set de descripteurs de fichiers
    FD_ZERO(&readfds);
    FD_SET(server_socket, &readfds);


    // Définir le délai d'attente (timeout)
    timeout.tv_sec = carte_choisie*0.2;  // 5 secondes
    timeout.tv_usec = 0;

    // Utiliser select pour surveiller les sockets
    int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

    if (activity < 0) {
        perror("Erreur lors de l'appel à select");
        returnarr[0] = -1;
        return;
    } else if (activity == 0) {
        returnarr[0] = 0;
        return;
    } else {
        // Vérifier quel socket a des données disponibles
        int bytes_recus = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_recus <= 0) {
            printf("Connexion terminée par le serveur.\n");
            returnarr[0] = -1;
            return;
        }
        if (strncmp(buffer, "200", 3) == 0) {
            // Extraire les cartes envoyées par le serveur
            char *time = buffer + 4; // Ignore "Vos cartes : "
            int timestart = atoi(time);
            returnarr[0] = 1;
            returnarr[1] = timestart;
            return;
        }

    }

    returnarr[0] = -1;
    return;
}

// Fonction de comparaison pour qsort
int comparer(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);  // Comparer deux entiers
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
    int idjoueur;
    int timestart;
    while (1) {
        // Réception des messages du serveur
        int bytes_recus = recv(serveur_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_recus <= 0) {
            printf("Connexion terminée par le serveur.\n");
            break;
        }
        buffer[bytes_recus] = '\0';
        if (strncmp(buffer, "100", 3) == 0) {
            // Extraire les cartes envoyées par le serveur
            char *id_str = buffer + 4; // Ignore "Vos cartes : "
            idjoueur= atoi(id_str);
            continue;
        }

        buffer[bytes_recus] = '\0';
        if (strncmp(buffer, "200", 3) == 0) {
            // Extraire les cartes envoyées par le serveur
            char *time = buffer + 4; // Ignore "Vos cartes : "
            timestart = atoi(time);
            continue;
        }

        
        printf("Message du serveur : %s\n", buffer+4);

        // Vérifier si le message contient la liste des cartes
        if (strncmp(buffer, "103", 3) == 0) {
            // Extraire les cartes envoyées par le serveur
            for (int i = 0; i < MAX_CARTES; i++)
            {
                cartes[i] = 0;
            }
            
            char *cartes_str = buffer + 17; // Ignore "Vos cartes : "
            char *token = strtok(cartes_str, " ");
            nombre_cartes = 0;
            while (token != NULL) {
                cartes[nombre_cartes++] = atoi(token);
                token = strtok(NULL, " ");
            }
            qsort(cartes, MAX_CARTES, sizeof(int), comparer);
            continue;
        }

        // Si c'est le tour du joueur
        if (strncmp(buffer, "101", 3) == 0) {
            timestart = time(NULL)+2;
            int carte_choisie;
            // Demander à l'utilisateur de choisir une carte valide
            int x = 1 ;
            while (x>0) {

                int condition = 1;
                if(timestart-time(NULL)<0){
                    for (int i = 0; i < MAX_CARTES; i++){
                        if (cartes[i] != -1 && cartes[i] != 0){
                            carte_choisie = cartes[i];
                            int returnarr[2]= {0,0};
                            chat_loop(serveur_fd, carte_choisie, returnarr);   
                            if(returnarr[0]==0){
                                carte_choisie = cartes[i];
                                cartes[i] = -1;
                                snprintf(buffer, sizeof(buffer), "%d %d", idjoueur, carte_choisie);
                                send(serveur_fd, buffer, strlen(buffer), 0);
                                printf("Vous avez joué la carte %d.\n", carte_choisie);
                                condition = 0;
                                break;  // Quitter la boucle de sélection de carte
                            
                            } else if (returnarr[0]==-1){
                            printf("Connexion terminée par le serveur.\n");
                            break;
                            condition = 0;
                            } else if (returnarr[0]==1){
                                timestart = returnarr[1];
                                break;
                            }
                        }
                    }   
                    if (condition==0)
                    {
                        break;
                    }     
                }

            }
                
        }
        
    }

    close(serveur_fd);
    return 0;
}
