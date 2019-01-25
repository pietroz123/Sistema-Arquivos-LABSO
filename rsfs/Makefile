CC = gcc
CFLAGS = -Wall -g

OBJS = disk.o shell.o fs.o

rsfs: $(OBJS)
	$(CC) -o rsfs $(OBJS)

disk.o: disk.h
fs.o: fs.h disk.h
shell.o: disk.h fs.h

.PHONY : clean
clean:
	rm -f *.o *~ rsfs

# gcc -g shell.c disk.c disk.h fs.c fs.h -o t1