#DEBUG  = -g -O0
DEBUG   = -O3
CC      = g++
INCLUDE = 
CFLAGS  = $(DEBUG) -Wall $(INCLUDE) -Winline -pipe

LDFLAGS = -L/usr/local/lib -L/usr/lib 
LDLIBS    =  


SRC     =       bmp280.c

OBJ     =       $(SRC:.c=.o)

BINS    =       $(SRC:.c=)

bmp280.o : bmp280.c
	$(CC) -c -o $@ $< $(CFLAGS)

all:    bmp280

clean : 
	rm bmp280.o bmp280
        
bmp280: bmp280.o
