all: copier

clean:
	rm copier

copier: Copier.c BoundedBuffer.c
	gcc -Wall -pthread -o copier Copier.c BoundedBuffer.c