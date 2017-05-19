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

/* Versão 2.0
* Adicionado as variáveis globais
* Feito a leitura e a verificação do argumento passado via terminal
* Criação das threads
* Utilizado exclusão mútua dentro das threads
* Barreira para sincronização
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef unsigned char cell_t;

//Variaveis globais
cell_t **prev, **next, **tmp;
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

  for (k=sk; k<=ek; k++)
  for (l=sl; l<=el; l++)
  count+=board[k][l];
  count-=board[i][j];

  return count;
}

void *play () {
    int i, j, a;
    /* for each cell, apply the rules of Life */
    while (1){                      //condição para fazer com que as threads nunca parem de procurar uma nova coluna, a menos que:
      if(cont >= size) {                //se o contador de colunas coletadas for maior que o tamanho da matriz
        pthread_barrier_wait (&barreira);     //a thread para na barreira e espera todas as outras threads entrarem no mesmo caso,
                              //ou seja, espera que a ultima thread que ainda está executando a ultima coluna termine
          pthread_exit(NULL);             //após todas as threads acabarem elas encerram
      } else {
        pthread_mutex_lock(&m);             //Exclusão mútua para que
        cont++;                     //Se uma thread fazer cont++ (que indica que a coluna cont -1 foi coletada) ela precisa ter certeza que 
        i = cont-1;                   //nenhuma outra thread execute essa linha, pois precisa garantir que o valor de i represente a coluna que foi coletada
        pthread_mutex_unlock(&m);           //Final da exclusão mútua
        for (j=0; j<size; j++) {            //Após a thread coletar uma coluna, ele percorre todas as linhas dessa coluna e verifica o que fazer com ela
          	a = adjacent_to (prev, size, i, j);
          	if (a == 2) next[i][j] = prev[i][j];
          	if (a == 3) next[i][j] = 1;
          	if (a < 2) next[i][j] = 0;
          	if (a > 3) next[i][j] = 0;
      	} 
      }  
  	}
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
  int i, j;
  pthread_mutex_init(&m,NULL); 
  // ---------- Leitura do argumento ---------- //
  maxth = argc > 1? atoi(argv[1]) : 1;
  // ---------- Leitura do file ---------- //
  
  FILE *f;
  f = stdin;
  fscanf(f,"%d %d", &size, &steps);
  cell_t ** prev = allocate_board (size);
  read_file (f, prev,size);
  fclose(f);
  // ---------- Alocação ---------- //
  
  cell_t ** next = allocate_board (size);
  cell_t ** tmp;
  
  #ifdef DEBUG
  printf("Initial:\n");
  print(prev,size);
  #endif
  // ---------- Criação de ''maxth'' threads ---------- //
  pthread_t threads[maxth];
  pthread_barrier_init (&barreira, NULL, maxth);  //Criação da barreira com o número de threads mais a main thread
  
  for (i=0; i<maxth; i++) {
    pthread_create(&threads[i], NULL, play, NULL);
  }
  // ---------- Inicio da chamada das threads e da execução do GoL ---------- //
    for (i=0; i<steps; i++) {         // for para determinar quantas gerações vai ter o GoL
    for (j=0; j<maxth; j++){        // for para dar join em todas as threads e iniciar o GoL
        pthread_join(threads[j], NULL);  
      }
      //pthread_barrier_wait (&barreira);   // APÓS dar join em todas as threads ele só continuará o código após todas as threads acabarem!!!
                          // A main só voltara a rodar depois de todas as threads serem executadas
                        // Apartir dessa parte do código todas as threads estarão rodando
                        // elas irão parar apenas quando a barreira permitir, ou seja, quando todas as threads perceberem que todas as colunas foram processadas
      #ifdef DEBUG
      printf("%d ----------\n", i + 1);
      print (next,size);
      #endif
      
      tmp = next;
      next = prev;
      prev = tmp;
  }

#ifdef RESULT
  printf("Final:\n");
  print (prev,size);
#endif
  pthread_barrier_destroy(&barreira);
  pthread_mutex_destroy(&m);
  free_board(prev,size);
  free_board(next,size);
}
