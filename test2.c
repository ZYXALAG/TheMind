#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void shuffle(int* tableau, size_t taille) {
    // Initialisation du générateur de nombres aléatoires avec l'heure
    srand(time(NULL));

    // Algorithme de Fisher-Yates pour mélanger le tableau
    for (size_t i = taille - 1; i > 0; i--) {
        // Tirer un index aléatoire entre 0 et i
        size_t j = rand() % (i + 1);

        // Échanger les éléments tableau[i] et tableau[j]
        int temp = tableau[i];
        tableau[i] = tableau[j];
        tableau[j] = temp;
    }
}

void afficherTableau(int* tableau, size_t taille) {
    for (size_t i = 0; i < taille; i++) {
        printf("%d ", tableau[i]);
    }
    printf("\n");
}

int main() {
    int tableau[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    size_t taille = sizeof(tableau) / sizeof(tableau[0]);

    printf("Tableau avant le shuffle : ");
    afficherTableau(tableau, taille);

    // Mélanger le tableau
    shuffle(tableau, taille);

    printf("Tableau après le shuffle : ");
    afficherTableau(tableau, taille);

    return 0;
}
