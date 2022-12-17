#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 1000000 // Taille de la mémoire partagée
#define MAX_WARRIORS 100    // Nombre maximal de warriors
#define MAX_PROCESSES 1000  // Nombre maximal de processus

// Structures de données pour les warriors et les processus
typedef struct
{
    char name[256]; // Nom du warrior
    int size;       // Taille du warrior en octets
    char *code;     // Code du warrior
} Warrior;

typedef struct
{
    int warrior;      // Numéro du warrior auquel appartient le processus
    int pointer;      // Pointeur d'instruction du processus
    int registers[4]; // Registres du processus
} Process;

// Mémoire partagée
char memory[MEMORY_SIZE];

// Tableaux de warriors et de processus
Warrior warriors[MAX_WARRIORS];
Process processes[MAX_PROCESSES];

// Nombre de warriors et de processus en cours
int numWarriors;
int numProcesses;

// Fonction d'exécution d'une instruction
void execute(int process, int instruction)
{
    // Récupère le warrior et le processus
    Warrior *w = &warriors[processes[process].warrior];
    Process *p = &processes[process];

    // Exécute l'instruction en utilisant les registres et le pointeur d'instruction du processus
    switch (instruction) {
    case 0: // MOV
        p->registers[w->code[p->pointer + 1]] = p->registers[w->code[p->pointer + 2]];
        p->pointer += 3;
        break;
    case 1: // ADD
        p->registers[w->code[p->pointer + 1]] += p->registers[w->code[p->pointer + 2]];
        p->pointer += 3;
        break;
    case 2: // SUB
        p->registers[w->code[p->pointer + 1]] -= p->registers[w->code[p->pointer + 2]];
        p->pointer += 3;
        break;
    case 3: // JMP
        p->pointer = p->registers[w->code[p->pointer + 1]];
        break;
    case 4: // JZ
        if (p->registers[w->code[p->pointer + 1]] == 0) {
            p->pointer = p->registers[w->code[p->pointer + 2]];
        } else {
            p->pointer += 3;
        }
        break;
    // ... autres instructions
    case 5: // LOAD
        p->registers[w->code[p->pointer + 1]] = memory[p->registers[w->code[p->pointer + 2]]];
        p->pointer += 3;
        break;
    case 6: // STORE
        memory[p->registers[w->code[p->pointer + 1]]] = p->registers[w->code[p->pointer + 2]];
        p->pointer += 3;
        break;
    case 7: // COPY
    {
        int src = p->registers[w->code[p->pointer + 1]];
        int dst = p->registers[w->code[p->pointer + 2]];
        int len = p->registers[w->code[p->pointer + 3]];
        memcpy(&memory[dst], &memory[src], len);
        p->pointer += 4;
    }
    break;
    case 8: // SPL
    {
        // Crée un nouveau processus en copiant les données du processus actuel
        Process newProcess = *p;
        // Modifie le pointeur d'instruction du nouveau processus
        newProcess.pointer = p->registers[w->code[p->pointer + 1]];
        // Ajoute le nouveau processus à la liste
        processes[numProcesses++] = newProcess;
        p->pointer += 2;
    }
    break;
        // ... autres instructions
    }
}

int main(int argc, char **argv)
{
    // Chargement des warriors à partir de fichiers
    for (int i = 1; i < argc; i++) {
        // Ouvre le fichier du warrior
        FILE *f = fopen(argv[i], "r");
        // Lit le nom et la taille du warrior
        fscanf(f, " (%[^,],%d)", warriors[numWarriors].name, &warriors[numWarriors].size);
        // Alloue de l'espace pour le code du warrior
        warriors[numWarriors].code = malloc(warriors[numWarriors].size);
        // Lit le code du warrior
        for (int j = 0; j < warriors[numWarriors].size; j++) {
            fscanf(f, " %hhd", &warriors[numWarriors].code[j]);
        }
        // Crée un processus pour le warrior
        processes[numProcesses++] = (Process){numWarriors, 0, {0, 0, 0, 0}};
        // Passe au warrior suivant
        numWarriors++;
    }
    printf("Finish loading all champions\n");

    // Boucle principale de combat
    while (numProcesses > 0) {
        // Exécute chaque processus
        // Exécute chaque processus
        for (int i = 0; i < numProcesses; i++) {
            Warrior *w = &warriors[processes[i].warrior];
            Process *p = &processes[i];
            if (p->pointer < w->size) {
                execute(i, w->code[p->pointer]);
            } else {
                // Pointeur d'instruction hors de la zone de mémoire allouée, le processus est terminé
                processes[i] = processes[--numProcesses];
            }
        }
        printf("%i\n", numProcesses);
    }

    return 0;
}
