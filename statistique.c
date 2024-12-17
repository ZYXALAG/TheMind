#include <stdio.h>
#include <stdlib.h>

#define MAX_JOUEURS 10
#define MAX_NOM 50
#define MAX_MANCHES 100

typedef struct {
    char nom[MAX_NOM];
    int manches_reussies;
    int manches_perdues;
    int valeurs_perdues[MAX_MANCHES];
    int temps_reaction[MAX_MANCHES]; // en secondes
} Joueur;

void afficher_statistiques(Joueur joueurs[], int n) {
    printf("\nStatistiques de la partie :\n");
    for (int i = 0; i < n; i++) {
        printf("Joueur : %s\n", joueurs[i].nom);
        printf("  Manches réussies : %d\n", joueurs[i].manches_reussies);
        printf("  Manches perdues : %d\n", joueurs[i].manches_perdues);
        
        if (joueurs[i].manches_perdues > 0) {
            printf("  Valeurs perdues : ");
            for (int j = 0; j < joueurs[i].manches_perdues; j++) {
                printf("%d ", joueurs[i].valeurs_perdues[j]);
            }
            printf("\n");
        }

        printf("  Temps de réaction moyen : ");
        if (joueurs[i].manches_reussies + joueurs[i].manches_perdues > 0) {
            float somme_temps = 0;
            for (int j = 0; j < joueurs[i].manches_reussies + joueurs[i].manches_perdues; j++) {
                somme_temps += joueurs[i].temps_reaction[j];
            }
            printf("%.2f secondes\n", somme_temps / (joueurs[i].manches_reussies + joueurs[i].manches_perdues));
        } else {
            printf("N/A\n");
        }
    }
}

void sauvegarder_statistiques(Joueur joueurs[], int n) {
    FILE *fichier = fopen("statistiques.txt", "w");
    if (fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return;
    }

    fprintf(fichier, "Statistiques de la partie :\n");
    for (int i = 0; i < n; i++) {
        fprintf(fichier, "Joueur : %s\n", joueurs[i].nom);
        fprintf(fichier, "  Manches réussies : %d\n", joueurs[i].manches_reussies);
        fprintf(fichier, "  Manches perdues : %d\n", joueurs[i].manches_perdues);
        
        if (joueurs[i].manches_perdues > 0) {
            fprintf(fichier, "  Valeurs perdues : ");
            for (int j = 0; j < joueurs[i].manches_perdues; j++) {
                fprintf(fichier, "%d ", joueurs[i].valeurs_perdues[j]);
            }
            fprintf(fichier, "\n");
        }

        fprintf(fichier, "  Temps de réaction moyen : ");
        if (joueurs[i].manches_reussies + joueurs[i].manches_perdues > 0) {
            float somme_temps = 0;
            for (int j = 0; j < joueurs[i].manches_reussies + joueurs[i].manches_perdues; j++) {
                somme_temps += joueurs[i].temps_reaction[j];
            }
            fprintf(fichier, "%.2f secondes\n", somme_temps / (joueurs[i].manches_reussies + joueurs[i].manches_perdues));
        } else {
            fprintf(fichier, "N/A\n");
        }
    }

    fclose(fichier);
    printf("Statistiques sauvegardées dans 'statistiques.txt'.\n");
}

int main() {
    Joueur joueurs[MAX_JOUEURS];
    int n;

    printf("Entrez le nombre de joueurs (max %d) : ", MAX_JOUEURS);
    scanf("%d", &n);

    if (n > MAX_JOUEURS) {
        printf("Le nombre de joueurs ne peut pas dépasser %d.\n", MAX_JOUEURS);
        return 1;
    }

    // Saisie des données des joueurs
    for (int i = 0; i < n; i++) {
        printf("Entrez le nom du joueur %d : ", i + 1);
        scanf("%s", joueurs[i].nom);
        printf("Entrez le nombre de manches réussies pour %s : ", joueurs[i].nom);
        scanf("%d", &joueurs[i].manches_reussies);
        printf("Entrez le nombre de manches perdues pour %s : ", joueurs[i].nom);
        scanf("%d", &joueurs[i].manches_perdues);

        // Saisie des valeurs perdues
        for (int j = 0; j < joueurs[i].manches_perdues; j++) {
            printf("Entrez la valeur perdue pour la manche %d de %s : ", j + 1, joueurs[i].nom);
            scanf("%d", &joueurs[i].valeurs_perdues[j]);
        }

        // Saisie des temps de réaction
        for (int j = 0; j < joueurs[i].manches_reussies + joueurs[i].manches_perdues; j++) {
            printf("Entrez le temps de réaction pour la manche %d de %s (en secondes) : ", j + 1, joueurs[i].nom);
            scanf("%d", &joueurs[i].temps_reaction[j]);
        }
    }

    // Afficher les statistiques
    afficher_statistiques(joueurs, n);

    // Sauvegarder les statistiques
    sauvegarder_statistiques(joueurs, n);

    return 0;
}