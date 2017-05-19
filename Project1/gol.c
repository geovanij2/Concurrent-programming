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
#include <pthread.h>  // include na biblioteca de threads para linux
#include <sys/types.h>

typedef unsigned char cell_t;

pthread_barrier_t barrier;

// variaveis globais
int max_threads;
int size;
int steps;
cell_t** prev;
cell_t** next;


cell_t ** allocate_board (int size) {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
  for (int i = 0; i < size; i++){
    board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  }
  return board;
}

void free_board (cell_t ** board, int size) {
  for (int i = 0; i < size; i++){
    free(board[i]);
  }
  free(board);
}


/* return the number of on cells adjacent to the i,j cell */
int adjacent_to (cell_t ** board, int size, int i, int j) {
  int	k, l, count=0;

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
  /* for each row */
  for (int j = 0; j < size; j++) {
    /* print each column position... */
    for (int i = 0; i < size; i++){
      printf ("%c", board[i][j] ? 'x' : ' ');
    }
    /* followed by a carriage return */
    printf ("\n");
  }
}

/*------------------------------------------------------------
  nova função play receberá 1 argumento e vai controlar steps
  play precisa de board antiga, board nova, size, steps, numero da threads
--------------------------------------------------------------*/
void *play (void* arg) {
  int a, b;
  // calculo de quantas linhas a thread deverá calcular:
  int thread_number = *((int *) arg);

  int valor_min = thread_number * (size / max_threads);
  int valor_max = valor_min + (size / max_threads);
  if (size % max_threads != 0 && thread_number == max_threads -1) {
    valor_max += thread_number % max_threads;
  }  
  if (valor_max > size)
    valor_max = size;
  /* for each cell, apply the rules of Life */
  while (steps > 0){
    for (int i = valor_min; i < valor_max; i++){
      for (int j = 0; j < size; j++) {
        a = adjacent_to (prev, size, i, j);
        if (a == 2)
          next[i][j] = prev[i][j];
        if (a == 3)
          next[i][j] = 1;
        if (a < 2)
          next[i][j] = 0;
        if (a > 3)
          next[i][j] = 0;
      }
    }
    b = pthread_barrier_wait(&barrier);
    if (b == PTHREAD_BARRIER_SERIAL_THREAD) {
      cell_t** tmp = next;
      next = prev;
      prev = tmp;
      steps--;

      #ifdef DEBUG
      printf("%d ----------\n", steps);
      print(next, size);
      #endif 

    }
    pthread_barrier_wait(&barrier);
  }
  free(arg);
  pthread_exit(NULL);
}


/* read a file into the life board */
void read_file (FILE * f, cell_t ** board, int size) {
  char	*s = (char *) malloc(size+10);
  /* read the first new line (it will be ignored) */
  fgets (s, size+10,f);
  /* read the life board */
  for (int j = 0; j < size; j++) {
    /* get a string */
    fgets (s, size+10,f);
    /* copy the string to the life board */
    for (int i = 0; i < size; i++) {
      board[i][j] = s[i] == 'x';
    }
  }
}

int main (int argc, char const *argv[]) {
  // recebendo numero maximo de threads como parametro argv
  // e setando max_threads como default = 2
  if (argc > 1) {
    max_threads = atoi(argv[1]);
  } else {
    max_threads = 2;
  }


  FILE    *f;
  f = stdin;
  fscanf(f,"%d %d", &size, &steps);


  prev = allocate_board (size);
  read_file (f, prev,size);
  fclose(f);


  next = allocate_board (size);


  #ifdef DEBUG
  printf("Initial:\n");
  print(prev,size);
  #endif


  pthread_t threads[max_threads];

  pthread_barrier_init(&barrier, NULL, max_threads);

  for (int i = 0; i < max_threads; i++) {
    int* x = malloc(sizeof(int));
    *x = i;
    pthread_create(&threads[i], NULL, play, (void*) x);
  }

  for (int i = 0; i < max_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  pthread_barrier_destroy(&barrier);

#ifdef RESULT
  printf("Final:\n");
  print (prev,size);
#endif

  free_board(prev,size);
  free_board(next,size);
}
