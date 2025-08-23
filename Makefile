CC = gcc
CFLAGS = -Wall -O2 -Wformat-zero-length
SRC = ./src
OBJS = $(SRC)/tjpgd.o main.o

all: jpeg_decode

jpeg_decode: $(OBJS)
	$(CC) $(CFLAGS) -o jpeg_decode $(OBJS)

main.o: main.c
	$(CC) $(CFLAGS) -I $(SRC) -c main.c

$(SRC)/tjpgd.o: $(SRC)/tjpgd.c $(SRC)/tjpgd.h $(SRC)/tjpgdcnf.h
	$(CC) $(CFLAGS) -c $(SRC)/tjpgd.c -o $(SRC)/tjpgd.o

clean:
	rm -f *.o $(SRC)/*.o jpeg_decode
