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
            printf("current time2 %d\n",timestart);
            returnarr[0] = 1;
            returnarr[1] = timestart;
            return;
        }

    }

    returnarr[0] = -1;
    return;
}






/*void chat_loop(int server_socket, int *cartes, int nombre_cartes, int idjoueur) {
    int carte_choisie;
    fd_set readfds;
    char buffer[BUFFER_SIZE];

    while (1) {
        // Initialiser le set de descripteurs
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);         // Ajouter stdin (entrée utilisateur)
        FD_SET(server_socket, &readfds);       // Ajouter le socket serveur
        int max_fd = (server_socket > STDIN_FILENO) ? server_socket : STDIN_FILENO;

        // Attendre une activité sur stdin ou le socket
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Erreur avec select");
            break;
        }

        // Si l'utilisateur a saisi quelque chose
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, BUFFER_SIZE, stdin); // Lire l'entrée utilisateur
            
            // Vérifier si la carte est valide (présente dans la main du joueur)
                carte_choisie = atoi(buffer);
                int indice = carte_dans_main(carte_choisie, cartes, nombre_cartes);
                if (indice != -1) {
                    // Retirer la carte de la main et informer le serveur
                    cartes[indice] = -1;
                    snprintf(buffer, sizeof(buffer), "%d %d", idjoueur, carte_choisie);
                    send(server_socket, buffer, strlen(buffer), 0);
                    printf("Vous avez joué la carte %d.\n", carte_choisie);
                    break;  // Quitter la boucle de sélection de carte
                } else {
                    printf("Carte invalide. Veuillez choisir une carte dans votre main.\n");
                }
        }

        // Si le serveur a envoyé un message
        if (FD_ISSET(server_socket, &readfds)) {
            int bytes_received = recv(server_socket, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received <= 0) {
                printf("Le serveur a fermé la connexion.\n");
                break;
            }
            buffer[bytes_received] = '\0'; // Null-terminate le message
            printf("Message du serveur : %s\n", buffer +4);
        }
    }
}*/

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
            printf("current time1 %d\n",timestart);
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
            
            char *cartes_str = buffer + 12; // Ignore "Vos cartes : "
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
            int carte_choisie;
            //printf("current time %d\n",(int)time(NULL));
            // Demander à l'utilisateur de choisir une carte valide
            int x = 1 ;
            while (x>0) {
                /*printf("%ld Choisissez une carte %ld à jouer : ",timestart, timestart-time(NULL));*/
                if (x==2){
                    printf("fhdsjqflkqhslqskhdf\n");
                    x=1;
                }

                int condition = 1;
                if(timestart-time(NULL)<0){
                    printf("test3");
                    //printf("index %d %d %d\n",(int) time(NULL), (int)timestart, (int) (time(NULL)-timestart));
                    for (int i = 0; i < MAX_CARTES; i++){
                        if (cartes[i] != -1 && cartes[i] != 0){
                            
                            printf("carte %d\n",cartes[i]);
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
                            } else if (returnarr[0]==1){
                                printf("tessssstttttttt\n");
                                timestart = returnarr[1];
                                x = 2;
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
