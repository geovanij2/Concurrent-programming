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
  for (int i = 0; i < size; i++)
  	board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
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
  int count = 0;

  int sk = (i>0) ? i-1 : i;
  int ek = (i+1 < size) ? i+1 : i;
  int sl = (j>0) ? j-1 : j;
  int el = (j+1 < size) ? j+1 : j;

  for (int k = sk; k <= ek; k++) {
    for (int l = sl; l <= el; l++) {
      count+=board[k][l];
    }
  }
  count -= board[i][j];
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

void *play () {
	// --------- Inicialização das variáveis usadas ----------//
  int i, j, a, q;
  cell_t** tmp;
  // --------- laço mais externo (controla quantas gerações seram executadas) ----------//
  while (steps > 0) {
  	/* Esse laço faz com que quando uma thread chegue nele ela pegue a primeira linha 
  	 Disponível no momento. O mutex serve para impedir uma condição de corrida com 
  	 a variável cont, que é lida e escrita por todas as threads. */
    while (cont < size) {
      pthread_mutex_lock(&m); 
      i = cont;
      cont++;                      
      pthread_mutex_unlock(&m);
      // Esse if impede que as threads acessem locais invalidos da memória
      if (i >= size)
        break;
      // Calculo da próxima geração de células (inalterado)
      for (j = 0; j < size; j++) {
          a = adjacent_to (prev, size, i, j);
          if (a == 2) next[i][j] = prev[i][j];
          if (a == 3) next[i][j] = 1;
          if (a < 2) next[i][j] = 0;
          if (a > 3) next[i][j] = 0; 
      }
    }
    // primeira barreira. Esta faz com que as threads esperem todas as outras acabarem,
    // quando isso ocorre, uma retorna um inteiro diferente de zero armazenado em q para
    // uma das threads e todas as threads são liberadas
    q = pthread_barrier_wait(&barreira);
    // Caso seja a thread que recebeu o inteiro diferente de zero, dimuia steps reset cont
    // e realize a triangularização para o começo de uma nova geração
    if (q == PTHREAD_BARRIER_SERIAL_THREAD) {
      steps--;
      tmp = next;
      next = prev;
      prev = tmp;
      cont = 0;
      // ------------ DEBUG ------------//
      #ifdef DEBUG
      printf("-----------> [%d] passo\n", steps);
  	  print(prev,size);
      #endif
    }
    // Outra barreira para esperar a thread terminar a o procedimento acima descrito
    pthread_barrier_wait(&barreira);
  }
  // termina a função
  pthread_exit(NULL);
}


/* read a file into the life board */
void read_file (FILE * f, cell_t ** board, int size) {
  char  *s = (char *) malloc(size+10);
  /* read the first new line (it will be ignored) */
  fgets (s, size+10,f);

  /* read the life board */
  for (int j = 0; j < size; j++) {
    /* get a string */
    fgets (s, size+10,f);
    /* copy the string to the life board */
    for (int i = 0; i < size; i++)
    board[i][j] = s[i] == 'x';
  }
}

int main (int argc, char *argv[]) {
  pthread_mutex_init(&m,NULL); 
  int j, a;
  // ---------- Leitura do argumento ---------- //
  maxth = argc > 1? atoi(argv[1]) : 1;
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
  // ---------- Debug ------------//
  #ifdef DEBUG
  printf("Initial:\n");
  print(prev,size);
  #endif
  // ---------- Criação de ''maxth'' threads ---------- //
  pthread_t threads[maxth];
  pthread_barrier_init (&barreira, NULL, maxth);
  
  // ---------- Inicio da chamada das threads e da execução do GoL ---------- //
  for (a = 0; a < maxth; a++) 
    pthread_create(&threads[a], NULL, play, NULL);

  for (j = 0; j < maxth; j++)
    pthread_join(threads[j], NULL);
  // ---------- RESULTADO -------------//
  #ifdef RESULT	
  printf("FINAL----------\n");
  print(prev,size);
  #endif
  // ---------- Liberando memória --------------//
  pthread_barrier_destroy(&barreira);
  pthread_mutex_destroy(&m);
  free_board(prev,size);
  free_board(next,size);
}
