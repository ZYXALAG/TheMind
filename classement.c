#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_JOUEURS 2
#define MAX_NOM 50

typedef struct {
    char nom[MAX_NOM];
    int manches_reussies;
} Joueur;

void afficher_classement(Joueur joueurs[], int n) {
    printf("\nClassement des joueurs :\n");
    for (int i = 0; i < n; i++) {
        printf("%d. %s - %d manches réussies\n", i + 1, joueurs[i].nom, joueurs[i].manches_reussies);
    }
}

void trier_classement(Joueur joueurs[], int n) {
    // Tri simple par sélection
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (joueurs[i].manches_reussies < joueurs[j].manches_reussies) {
                Joueur temp = joueurs[i];
                joueurs[i] = joueurs[j];
                joueurs[j] = temp;
            }
        }
    }
}

void sauvegarder_classement(Joueur joueurs[], int n) {
    FILE *fichier = fopen("classement.txt", "w");
    if (fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return;
    }

    fprintf(fichier, "Classement des joueurs :\n");
    for (int i = 0; i < n; i++) {
        fprintf(fichier, "%d. %s - %d manches réussies\n", i + 1, joueurs[i].nom, joueurs[i].manches_reussies);
    }

    fclose(fichier);
    printf("Classement sauvegardé dans 'classement.txt'.\n");
}