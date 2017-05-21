/*
* The Game of Life
*
* a cell is born, if it has exactly three neighbours
* a cell dies of loneliness, if it has less than two neighbours
* a cell dies of overcrowding, if it has more than three neighbours
* a cell survives to the next generation, if it does not die of loneliness
* or overcrowding
*
* In this version, a 2D array of ints is used.  A 1 cell is on, a 0 cell is off.
* The game plays a number of steps (given by the input), printing to the screen each time.  'x' printed
* means on, space means off.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>

typedef unsigned char cell_t;

//Variaveis globais
cell_t** prev;
cell_t** next;
int maxth, steps, cont = 0, size;
pthread_mutex_t m;
pthread_barrier_t barreira;


cell_t ** allocate_board (int size) {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
  int i;
  for (i=0; i<size; i++)
  board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  return board;
}

void free_board (cell_t ** board, int size) {
  int     i;
  for (i=0; i<size; i++)
  free(board[i]);
  free(board);
}


/* return the number of on cells adjacent to the i,j cell */
int adjacent_to (cell_t ** board, int size, int i, int j) {
  int k, l, count=0;

  int sk = (i>0) ? i-1 : i;
  int ek = (i+1 < size) ? i+1 : i;
  int sl = (j>0) ? j-1 : j;
  int el = (j+1 < size) ? j+1 : j;

  for (int k=sk; k<=ek; k++) {
    for (l=sl; l<=el; l++) {
      count+=board[k][l];
    }
  }
  count-=board[i][j];
  return count;
}
/* print the life board */
void print (cell_t ** board, int size) {
  int i, j;
  /* for each row */
  for (j=0; j<size; j++) {
    /* print each column position... */
    for (i=0; i<size; i++)
    printf ("%c", board[i][j] ? 'x' : ' ');
    /* followed by a carriage return */
    printf ("\n");
  }
}

void *play () {
  int i, j, a, q;
  cell_t** tmp;
  while (steps > 0){  
    while (cont < size) {
      pthread_mutex_lock(&m); 
      cont++;                      
      i = cont-1;
      pthread_mutex_unlock(&m);
      if (i >= size)
        break;      
      for (j=0; j<size; j++) {
          a = adjacent_to (prev, size, i, j);
          if (a == 2) next[i][j] = prev[i][j];
          if (a == 3) next[i][j] = 1;
          if (a < 2) next[i][j] = 0;
          if (a > 3) next[i][j] = 0; 
      }
    }
    q = pthread_barrier_wait(&barreira);
    if (q == PTHREAD_BARRIER_SERIAL_THREAD) {
      steps--;
      tmp = next;
      next = prev;
      prev = tmp;
      cont = 0;
    }
    pthread_barrier_wait(&barreira);
  }
  pthread_exit(NULL);
}


/* read a file into the life board */
void read_file (FILE * f, cell_t ** board, int size) {
  int i, j;
  char  *s = (char *) malloc(size+10);

  /* read the first new line (it will be ignored) */
  fgets (s, size+10,f);

  /* read the life board */
  for (j=0; j<size; j++) {
    /* get a string */
    fgets (s, size+10,f);
    /* copy the string to the life board */
    for (i=0; i<size; i++)
    board[i][j] = s[i] == 'x';

  }
}

int main (int argc, char *argv[]) {
  pthread_mutex_init(&m,NULL); 
  int j, a;
  // ---------- Leitura do argumento ---------- //
  maxth = argc > 1? atoi(argv[1]) : 1;
  printf("threads = %d\n",maxth);
  // ---------- Leitura do file ---------- //
  
  FILE *f;
  f = stdin;
  fscanf(f,"%d %d", &size, &steps);
  prev = allocate_board (size);
  read_file (f, prev,size);
  fclose(f);
  // ---------- Alocação ---------- //
  next = allocate_board (size);
  cell_t ** tmp;
  
  #ifdef DEBUG
  print(prev,size);
  #endif
  // ---------- Criação de ''maxth'' threads ---------- //
  pthread_t threads[maxth];
  pthread_barrier_init (&barreira, NULL, maxth);
  
  // ---------- Inicio da chamada das threads e da execução do GoL ---------- //
  for (a=0; a<maxth; a++) 
    pthread_create(&threads[a], NULL, play, NULL);

  for (j=0; j<maxth; j++)
    pthread_join(threads[j], NULL);  
  printf("FINAL----------\n");
  print(next,size);

  pthread_barrier_destroy(&barreira);
  pthread_mutex_destroy(&m);
  free_board(prev,size);
  free_board(next,size);
}
