#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAX_JOUEURS 2
#define TAILLE_BUFFER 1024
#define MAX_CARTES 100

void envoi_carte(int socket_client, int *paquet, int *nb_cartes) {
    if (*nb_cartes <= 0) {
        printf("Plus de cartes dans le paquet.\n");
        return;
    }

    // Choisir une carte aléatoire
    int index_carte = rand() % *nb_cartes;
    int carte = paquet[index_carte];

    // Envoyer la carte au client avec un préfixe "CARTE:"
    char message[TAILLE_BUFFER];
    sprintf(message, "CARTE:%d\n", carte);
    if (write(socket_client, message, strlen(message)) < 0) {
        perror("Erreur lors de l'envoi de la carte");
    }

    // Supprimer la carte de la liste des cartes
    for (int i = index_carte; i < *nb_cartes - 1; i++) {
        paquet[i] = paquet[i + 1];
    }
    (*nb_cartes)--;
}

void attendre_fin_round(int socket_client) {
    char buffer[TAILLE_BUFFER];
    int octets_recus;

    // Attendre le message "fin" du client
    while ((octets_recus = read(socket_client, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[octets_recus] = '\0'; // Ajouter le caractère de fin de chaîne
        if (strcmp(buffer, "fin\n") == 0) {
            printf("Le joueur a terminé le round.\n");
            break;
        }
    }

    if (octets_recus == 0) {
        printf("Le client s'est déconnecté.\n");
    } else if (octets_recus < 0) {
        perror("Erreur lors de la réception du message de fin");
    }
}

int main() {
    srand(time(NULL)); // Initialiser le générateur de nombres aléatoires

    int rounds = 1;

    // Initialiser le jeu de cartes
    int paquet[MAX_CARTES];
    int nb_cartes = MAX_CARTES;

    // Remplir la liste avec des valeurs de 1 à MAX_CARTES
    for (int i = 0; i < MAX_CARTES; i++) {
        paquet[i] = i + 1;
    }

    int socket_serveur, socket_clients[MAX_JOUEURS];
    struct sockaddr_in adresse_serveur, adresse_client;
    socklen_t taille_client = sizeof(adresse_client);

    // Créer le socket du serveur
    socket_serveur = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_serveur == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Configurer l'adresse du serveur
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_addr.s_addr = INADDR_ANY;  // Écoute sur toutes les interfaces réseau
    adresse_serveur.sin_port = htons(PORT);  // Port de communication

    // Associer l'adresse et le port au socket
    if (bind(socket_serveur, (struct sockaddr *)&adresse_serveur, sizeof(adresse_serveur)) < 0) {
        perror("Erreur lors du bind");
        close(socket_serveur);
        exit(EXIT_FAILURE);
    }

    // Écouter les connexions entrantes
    if (listen(socket_serveur, MAX_JOUEURS) < 0) {
        perror("Erreur lors de l'écoute");
        close(socket_serveur);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    // Accepter les connexions des clients
    int joueurs_connectes = 0;
    
    while (joueurs_connectes < MAX_JOUEURS) {
        int socket_client = accept(socket_serveur, (struct sockaddr *)&adresse_client, &taille_client);
        if (socket_client < 0) {
            perror("Erreur lors de l'acceptation de la connexion");
            continue;
        }
    
        printf("Nouveau client connecté.\n");
        socket_clients[joueurs_connectes] = socket_client;
        joueurs_connectes++;

        const char *message = "Bienvenue sur le serveur de jeu de cartes.\n";
        if (write(socket_client, message, strlen(message)) < 0) {
            perror("Erreur lors de l'envoi du message de bienvenue");
        }
    }
    
    printf("Nombre maximum de joueurs connectés. Lancement de la partie.\n");

    // Boucle de jeu
    while (1) {
        // Annoncer la manche
        printf("Manche %d\n", rounds);
        // Envoyer le numéro de manche aux joueurs
        for (int i = 0; i < MAX_JOUEURS; i++) {
            char message[TAILLE_BUFFER];
            sprintf(message, "Manche %d\n", rounds);
            if (write(socket_clients[i], message, strlen(message)) < 0) {
                perror("Erreur lors de l'envoi du message de manche");
            }
        }
    
        // Distribuer les cartes aux joueurs
        for (int j = 0; j < MAX_JOUEURS; j++) {
            envoi_carte(socket_clients[j], paquet, &nb_cartes);
        }
    
        // Envoyer un message de fin de manche aux joueurs
        for (int i = 0; i < MAX_JOUEURS; i++) {
            const char *fin_manche = "FIN_MANCHE\n";
            if (write(socket_clients[i], fin_manche, strlen(fin_manche)) < 0) {
                perror("Erreur lors de l'envoi du message de fin de manche");
            }
        }

        // Attendre que tous les joueurs envoient "fin" pour terminer le round
        for (int i = 0; i < MAX_JOUEURS; i++) {
            attendre_fin_round(socket_clients[i]);
        }
    
        rounds++;
    
        // Condition pour terminer la boucle de jeu
        if (nb_cartes <= 0) {
            printf("Toutes les cartes ont été distribuées. Fin de la partie.\n");
            break;
        }
    }

    // Fermer les sockets des clients
    for (int i = 0; i < MAX_JOUEURS; i++) {
        close(socket_clients[i]);
    }

    // Fermer le socket du serveur
    close(socket_serveur);
    return 0;
}