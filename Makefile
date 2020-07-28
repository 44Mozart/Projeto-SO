CC = gcc
CFLAGS = -g -Wall

all: argus argusd
	test -e fifo || mkfifo fifo
	test -e fifo_resposta || mkfifo fifo_resposta

argus:
	$(CC) $(CFLAGS) argus.c argus.h -o argus

argusd:
	$(CC) $(CFLAGS) argusd.c argus.h slist.c slist.h -o argusd

clean:
	rm argus
	rm argusd
	unlink fifo
	unlink fifo_resposta
	rm log*
