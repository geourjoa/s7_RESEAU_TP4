#include <stdio.h>
#include <stdlib.h>
#include <curses.h> 		
#include <sys/signal.h>
#include <sys/wait.h>
#include "fon.h"
#include <string.h>

#include "commande.h"

#define TAILLE_MAX_COMMANDE 10

void client_appli(char *serveur, char *service, char *protocole);

/*****************************************************************************/
/*--------------- programme client -----------------------*/

main(int argc, char *argv[]) {

	char *serveur = SERVEUR_DEFAUT;
	char *service = SERVICE_DEFAUT; /* no de port) */
	char *protocole = PROTOCOLE_DEFAUT;

	/* Permet de passer un nombre de parametre variable a l'executable */
	switch (argc) {
	case 1:
		printf("serveur par defaut: %s\n", serveur);
		printf("service par defaut: %s\n", service);
		break;
	case 2:
		serveur = argv[1];
		printf("service par defaut: %s\n", service);
		break;
	case 3:
		serveur = argv[1];
		service = argv[2];
		break;

	default:
		printf("Usage: \n> ./client ip port \n");
		return EXIT_SUCCESS;
	}

	/* serveur est le nom (ou l'adresse IP) auquel le client va acceder */
	/* service le numero de port sur le serveur correspondant au  */
	/* service desire par le client */
	/* protocole le protocole qui sera utilise pour la communication */

	client_appli(serveur, service, protocole);

	return EXIT_SUCCESS;
}

/*****************************************************************************/
void client_appli(char *serveur, char *service, char *protocole) {
	char code_commande;

	char commande[TAILLE_MAX_COMMANDE];
	char tampon[TAILLE_TAMPON];

	char session_fini = 0;

	struct sockaddr_in p_adr_distant;

	//Cr�ation socket
	int socket_local = h_socket(AF_INET, SOCK_STREAM);

	//Initialisation du socket
	adr_socket(service, serveur, protocole, &p_adr_distant);

	//Connexion au serveur
	h_connect(socket_local, &p_adr_distant);

	while (!session_fini) {
		printf("Serveur $ ");
		scanf("%s", &commande);

		//Ajouté ici de nouvelles commandes coté client
		if (strcmp(commande, "ls") == 0) {
			code_commande = LS;
		} else {

			if (strcmp(commande, "get") == 0) {
				code_commande = GET;
			} else {

				if (strcmp(commande, "put") == 0) {
					code_commande = PUT;
				} else {

					if (strcmp(commande, "quit") == 0) {
						code_commande = QUIT;
					} else
						//défault
						code_commande = AIDE_CLIENT;
				}

			}
		}
		//TODO autre commande

		switch (code_commande) {
		case LS:

			h_writes(socket_local, &code_commande, 1);

			h_reads(socket_local, tampon, TAILLE_BUFFER_NOMBRE);

			tampon[TAILLE_BUFFER_NOMBRE] = '\0';

			int taille_flux = atoi(tampon);

			int nb_octets_imprime = 0;
			int nb_octest_lus = 0;

			//printf("flux %d\n", taille_flux);

			//Affichage du flux
			while (nb_octets_imprime != taille_flux) {

				//	printf("imprime %d\n", nb_octets_imprime);

				if (taille_flux - nb_octets_imprime > TAILLE_TAMPON)
					nb_octest_lus = h_reads(socket_local, tampon,
					TAILLE_TAMPON);
				else
					nb_octest_lus = h_reads(socket_local, tampon,
							taille_flux - nb_octets_imprime);

				int i;
				for (i = 0; i < nb_octest_lus; ++i) {
					printf("%c", tampon[i]);

				}

				nb_octets_imprime += nb_octest_lus;
			}

			break;

		case GET:

			h_writes(socket_local, &code_commande, 1);

			printf("nom fichier à télecharger ?  ");
			char fichierAObtenir[TAILLE_NOM_FICHIER_MAX];
			char fichierResultat[TAILLE_NOM_FICHIER_MAX];
			scanf("%s", &fichierAObtenir);

			h_writes(socket_local, fichierAObtenir, TAILLE_NOM_FICHIER_MAX);

			h_reads(socket_local, tampon, TAILLE_BUFFER_NOMBRE);

			strcpy(fichierResultat, "get_");
			strcpy(&fichierResultat[4], fichierAObtenir);

			socket_to_file(socket_local, atoi(tampon), fichierResultat);

			if (atoi(tampon) == 0) {
				printf("Fichier introuvable.\n");
			} else {
				printf("Télechargement terminé.\n");

			}

			break;

		case PUT:

			h_writes(socket_local, &code_commande, 1);

			printf("nom fichier à déposer ?  ");
			char fichierAEnvoyer[TAILLE_NOM_FICHIER_MAX];

			scanf("%s", &fichierAEnvoyer);

			h_writes(socket_local, fichierAEnvoyer, TAILLE_NOM_FICHIER_MAX);

			int taille = file_size(fichierAEnvoyer);
			char taille_texte[TAILLE_BUFFER_NOMBRE];

			char *stream = (char*) malloc(taille * sizeof(char));

			file_to_stream(fichierAEnvoyer, stream, taille);

			sprintf(taille_texte, "%d", taille);
			h_writes(socket_local, taille_texte, TAILLE_BUFFER_NOMBRE);

			h_writes(socket_local, stream, taille);

			printf("upload terminé\n");

			break;

		case QUIT:
			h_writes(socket_local, &code_commande, 1);
			session_fini = 1;
			break;

		case AIDE_CLIENT:

			printf("Commandes disponibles :\n");
			printf("ls : affichage du contenu du dossier.\n");
			printf(
					"get : telecharger un fichier du serveur, le préfixe 'get_' sera ajouté au nom du fichier.\n");
			printf(
					"put : envoyer un fichier du serveur, le préfixe 'put_' sera ajouté au nom du fichier.\n");

			break;
		default:
			printf("Erreur sur la saisie de la commande \n");
			break;
		}

		//Vider le buffer de stdin
		int c;
		do {
		  c = getchar();
		} while (c != EOF && c != '\n');

	}

	h_close(socket_local);

}

/*****************************************************************************/

