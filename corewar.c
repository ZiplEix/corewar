#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INSTRUCTION_LENGTH 100

typedef struct
{
  char opcode[MAX_INSTRUCTION_LENGTH];
  int op1;
  int op2;
  int op3;
} Instruction;

typedef void (*InstructionHandler)(Instruction *);

InstructionHandler instructionHandlers[256];

void parseProgram(const char *filename, Instruction *program, int *programSize)
{
  FILE *fp = fopen(filename, "r");
  if (fp == NULL)
  {
    perror("Erreur lors de l'ouverture du fichier");
    exit(1);
  }

  char line[MAX_INSTRUCTION_LENGTH];
  int lineNumber = 0;
  while (fgets(line, MAX_INSTRUCTION_LENGTH, fp) != NULL)
  {
    char *opcode = strtok(line, " ");
    int op1 = atoi(strtok(NULL, " "));
    int op2 = atoi(strtok(NULL, " "));
    int op3 = atoi(strtok(NULL, " "));

    Instruction instr;
    strcpy(instr.opcode, opcode);
    instr.op1 = op1;
    instr.op2 = op2;
    instr.op3 = op3;
    program[lineNumber++] = instr;
  }
  *programSize = lineNumber;

  fclose(fp);
}

void executeProgram(Instruction *program, int programSize)
{
  for (int i = 0; i < programSize; i++)
  {
    Instruction instr = program[i];
    InstructionHandler handler = instructionHandlers[instr.opcode];
    if (handler == NULL)
    {
      fprintf(stderr, "Instruction non reconnue : %s\n", instr.opcode);
      exit(1);
    }
    handler(&instr);
  }
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Usage: %s <fichier d'assembleur>\n", argv[0]);
    return 1;
  }

  // Initialisation des fonctions de traitement des instructions
  instructionHandlers['MOV'] = handleMovInstruction;
  instructionHandlers['ADD'] = handleAddInstruction;
  // ...

  Instruction program[MAX_PROGRAM_SIZE];
  int programSize;
  parseProgram(argv[1], program, &programSize);
  executeProgram(program, programSize);

  return 0;
}

#define MAX_PROGRAM_SIZE 1000

typedef struct
{
  char name[MAX_PROGRAM_NAME_LENGTH];
  int startAddress;
  int instructionPointer;
  int remainingCycles;
  int alive;
} Program;

Program programs[MAX_NUM_PROGRAMS];
int numPrograms;

void loadProgram(const char *filename, int startAddress)
{
  Instruction program[MAX_PROGRAM_SIZE];
  int programSize;
  parseProgram(filename, program, &programSize);

  Program p;
  strcpy(p.name, filename);
  p.startAddress = startAddress;
  p.instructionPointer = 0;
  p.remainingCycles = 0;
  p.alive = 1;
  programs[numPrograms++] = p;

  // Copie du code du programme dans l'espace mémoire
  for (int i = 0; i < programSize; i++)
  {
    memory[startAddress + i] = program[i];
  }
}

void executeProgram(Program *p)
{
  // Exécution de l'instruction suivante du programme
  Instruction instr = memory[p->instructionPointer];
  InstructionHandler handler = instructionHandlers[instr.opcode];
  if (handler == NULL)
  {
    fprintf(stderr, "Instruction non reconnue : %s\n", instr.opcode);
    p->alive = 0;
    return;
  }
  handler(&instr);
  p->instructionPointer++;

  // Mise à jour du nombre de cycles restants avant la prochaine instruction
  p->remainingCycles--;
  if (p->remainingCycles <= 0)
  {
    // Chargement de la prochaine instruction
    instr = memory[p->instructionPointer];
    p->remainingCycles = instructionCycles[instr.opcode];
  }
}

void runSimulation()
{
  int cycle = 0;
  int programsAlive = numPrograms;
  while (programsAlive > 1)
  {
    // Exécution des programmes
    for (int i = 0; i < numPrograms; i++)
    {
      Program *p = &programs[i];
      if (p->alive)
      {
        executeProgram(p);
        if (!p->alive)
        {
          programsAlive--;
        }
      }
    }

    cycle++;
  }

  printf("Simulation terminée en %d cycles\n", cycle);
}

#include <SDL2/SDL.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define CELL_SIZE 10

SDL_Window *window;
SDL_Renderer *renderer;

void initGraphics()
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    fprintf(stderr, "Erreur lors de l'initialisation de SDL : %s\n", SDL_GetError());
    exit(1);
  }

  window = SDL_CreateWindow("Corewar", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (window == NULL)
  {
    fprintf(stderr, "Erreur lors de la création de la fenêtre : %s\n", SDL_GetError());
    exit(1);
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL)
  {
    fprintf(stderr, "Erreur lors de la création du renderer : %s\n", SDL_GetError());
    exit(1);
  }
}

void drawMemory()
{
  int x = 0, y = 0;
  for (int i = 0; i < MEMORY_SIZE; i++)
  {
    SDL_Rect cellRect = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
    SDL_SetRenderDrawColor(renderer, memory[i] * 255, memory[i] * 255, memory[i] * 255, 255);
    SDL_RenderFillRect(renderer, &cellRect);

    x++;
    if (x == WINDOW_WIDTH / CELL_SIZE)
    {
      x = 0;
      y++;
    }
  }
}

void drawPrograms()
{
  for (int i = 0; i < numPrograms; i++)
  {
    Program *p = &programs[i];
    if (p->alive)
    {
      int x = p->instructionPointer % (WINDOW_WIDTH / CELL_SIZE);
      int y = p->instructionPointer / (WINDOW_WIDTH / CELL_SIZE);
      SDL_Rect cellRect = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
      SDL_SetRenderDrawColor(renderer, p->id * 255 / numPrograms, 0, 0, 255);
      SDL_RenderFillRect(renderer, &cellRect);
    }
  }
}

void render()
{
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  drawMemory();
  drawPrograms();
  SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
  // Initialisation de la fenêtre de visualisation
  initGraphics();

  // Chargement des programmes dans l'espace mémoire
  loadProgram("program1.asm", 0);
  loadProgram("program2.asm", 100);
  // ...

  // Exécution de la simulation
  runSimulation();

  // Boucle d'affichage de la visualisation
  while (1)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        goto cleanup;
      }
    }

    render();
    SDL_Delay(1000 / 60);
  }

cleanup:
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
