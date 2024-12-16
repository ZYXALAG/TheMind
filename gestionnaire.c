#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_JOUEURS 2
#define TAILLE_BUFFER 1024

#define MAX_CARTES 100
#define MAX_CARTES_MAINS 5

void envoi_carte(int socket_client, int *cartes, int *nb_cartes) {
    // Choisir une carte aléatoire
    int index_carte = rand() % *nb_cartes;
    int carte = cartes[index_carte];

    // Envoyer la carte au client avec un préfixe "CARTE:"
    char message[TAILLE_BUFFER];
    sprintf(message, "CARTE:%d\n", carte);
    write(socket_client, message, strlen(message));

    // Supprimer la carte de la liste des cartes
    for (int i = index_carte; i < *nb_cartes - 1; i++) {
        cartes[i] = cartes[i + 1];
    }
    (*nb_cartes)--;
}

int main() {

    int rounds = 1;

    // Initialiser le jeu de cartes
    int cartes[MAX_CARTES];

    // Remplir la liste avec des valeurs de 1 à MAX_CARTES
    for (int i = 0; i < MAX_CARTES; i++) {
        cartes[i] = i + 1;
    }

    int socket_serveur, socket_client;
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
        socket_client = accept(socket_serveur, (struct sockaddr *)&adresse_client, &taille_client);
        if (socket_client < 0) {
            perror("Erreur lors de l'acceptation de la connexion");
            continue;
        }
    
        printf("Nouveau client connecté.\n");
        joueurs_connectes++;

        const char *message = "Bienvenue sur le serveur de jeu de cartes.\n";
        write(socket_client, message, strlen(message));
    }
    
    printf("Nombre maximum de joueurs connectés. Lancement de la partie.\n");

    // Envoyer des cartes aux joueurs
    int nb_cartes = MAX_CARTES_MAINS;
    for (int i = 0; i < MAX_JOUEURS; i++) {
        for (int j = 0; j < MAX_CARTES_MAINS; j++) {
            envoi_carte(socket_client, cartes, &nb_cartes);
        }
    }



    // Fermer le socket du serveur
    // close(socket_serveur);
    return 0;
}