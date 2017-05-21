CC=gcc

FLAGS=-O3 -Wno-unused-result
LDFLAGS=-pthread
#DEBUG=-DDEBUG
RESULT=-DRESULT

all: gol

gol: gol.c
	$(CC) $(DEBUG) $(RESULT) $(FLAGS) $(LDFLAGS) gol.c -o gol
	#$(CC) --std=c11 -D_XOPEN_SOURCE=600 $(DEBUG) $(RESULT) $(FLAGS) golT.c -o golT -lpthread

clean:
	rm -rf gol
