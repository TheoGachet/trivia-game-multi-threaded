/* Code par Théo Gachet et Thimoté Dupuch - 2023 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "functions.h"
#include "server.h"
#include "client.h"

/* Les fonctions de ce fichiers ont été crées pour debug le programme, elles n'ont plus d'utilité apparente */

void handle_error(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void readRandomQuestionAndreponse()
{
    char question[MAX_LENGTH];
    char reponse[MAX_LENGTH];
    char reponse_client[MAX_LENGTH];
    int currentLine;
    int numLines;
    int randomLine;
    char ch;

    FILE *questionsFile = fopen("questions.txt", "r");
    FILE *reponsesFile = fopen("reponses.txt", "r");

    if (questionsFile == NULL || reponsesFile == NULL) {
        printf("Erreur lors de l'ouverture des fichiers.\n");
        return;
    }

    /* Comptage du nombre de lignes dans le fichier des questions */
    numLines = 0;
    while ((ch = fgetc(questionsFile)) != EOF) {
        if (ch == '\n') {
            numLines++;
        }
    }

    /* Réinitialisation du curseur de lecture au début du fichier */
    rewind(questionsFile);
    rewind(reponsesFile);

    /* Génération d'un nombre aléatoire entre 0 et numLines-1 */
    srand(time(NULL));
    randomLine = rand()% numLines;

    /* Lecture de la question et de la réponse correspondante */
    currentLine = 0;

    while (fgets(question, sizeof(question), questionsFile) != NULL &&
           fgets(reponse, sizeof(reponse), reponsesFile) != NULL) {
        if (currentLine == randomLine) {
            /* Suppression du saut de ligne à la fin de la question et de la réponse */
            question[strcspn(question, "\n")] = '\0';
            reponse[strcspn(reponse, "\n")] = '\0';

            printf("Question : %s\n", question);

            printf("Entrez la réponse : ");
            fgets(reponse_client, sizeof(reponse_client), stdin);

            reponse_client[strcspn(reponse_client, "\n")] = '\0';
            reponse[strcspn(reponse, "\n")] = '\0';
            reponse[strlen(reponse) - 1] = '\0';

            if(strcmp(reponse, reponse_client) == 0) {
                printf("Bonne réponse !");
            }
            else {
                printf("Faux, la réponse était : %s\n", reponse);
            }
            break;
        }
        currentLine++;
    }
    /* Fermeture des fichiers */
    fclose(questionsFile);
    fclose(reponsesFile);
}

char *reponse_aleatoire(int seconds) {
    FILE *fp;
    int numLines;
    char ch;
    unsigned long int randomLine;
    unsigned long int currentLine;
    char *reponse;
    size_t bufferSize = 2000;

    numLines = 0;

    reponse = (char*)malloc(bufferSize * sizeof(char));

    fp = fopen("reponses.txt", "r");
    if (!fp) {
        perror("Erreur ouverture du fichier réponse");
        return NULL;
    }

    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            numLines++;
        }
    }
    rewind(fp);
    randomLine = abs((rand() + (seconds)) % numLines);
    /*printf("random line : %ld\n", randomLine);*/

    currentLine = 0;

    while (fgets(reponse, bufferSize, fp) != NULL) {
        if (currentLine == randomLine) {
            reponse[strcspn(reponse, "\n")] = '\0';
            break;
        }
        currentLine++;
    }

    fclose(fp);
    return reponse;
}