#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EQUIPES 10

typedef struct {
    int nb_joueurs;
    int manches_reussies;
} Equipe;

void afficher_classement(Equipe equipes[], int n) {
    printf("\nClassement des équipes :\n");
    for (int i = 0; i < n; i++) {
        printf("%d : %d joueurs - %d manches réussies\n", i + 1, equipes[i].nb_joueurs, equipes[i].manches_reussies);
    }
}

void trier_classement(Equipe equipes[], int n) {
    // Tri simple par sélection
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (equipes[i].manches_reussies < equipes[j].manches_reussies) {
                Equipe temp = equipes[i];
                equipes[i] = equipes[j];
                equipes[j] = temp;
            }
        }
    }
}

void sauvegarder_classement(Equipe equipes[], int n) {
    FILE *fichier = fopen("classement.txt", "w");
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    fprintf(fichier, "Classement des équipes :\n");
    for (int i = 0; i < n; i++) {
        fprintf(fichier, "%d : %d joueurs - %d manches réussies\n", i + 1, equipes[i].nb_joueurs, equipes[i].manches_reussies);
    }

    fclose(fichier);
    printf("Classement sauvegardé dans 'classement.txt'.\n");
}

int main() {
    Equipe equipes[MAX_EQUIPES];
    int n;

    printf("Entrez le nombre d'équipes (maximum %d): ", MAX_EQUIPES);
    scanf("%d", &n);

    if (n < 1 || n > MAX_EQUIPES) {
        printf("Nombre d'équipes invalide. Utilisation de %d équipes.\n", MAX_EQUIPES);
        n = MAX_EQUIPES;
    }

    // Input for each team's number of players and successful matches
    for (int i = 0; i < n; i++) {
        printf("Entrez les informations pour l'équipe %d :\n", i + 1);
        printf("Nombre de joueurs: ");
        scanf("%d", &equipes[i].nb_joueurs);
        printf("Nombre de manches réussies: ");
        scanf("%d", &equipes[i].manches_reussies);
    }

    trier_classement(equipes, n);
    afficher_classement(equipes, n);
    sauvegarder_classement(equipes, n);

    return 0;
}
