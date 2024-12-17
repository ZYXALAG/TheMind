CC=gcc
CFLAGS=-Wall

all: gestionnaire joueur robot classement

gestionnaire: gestionnaire.c
	$(CC) $(CFLAGS) gestionnaire.c -o gestionnaire

joueur_humain: joueur.c
	$(CC) $(CFLAGS) joueur_humain.c -o joueur

joueur_robot: robot.c
	$(CC) $(CFLAGS) joueur_robot.c -o robot

clean:
	rm -f gestionnaire joueur robot classement
