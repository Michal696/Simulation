CFLAGSC=-std=c++11 -Wall -Werror -pedantic -o simulace -lsimlib -lm
CLIENT=simulace
CC=g++

all: $(CLIENT)


$(CLIENT) : $(CLIENT).cpp
		$(CC) $(CLIENT).cpp $(CFLAGSC)

run:
	./simulace

clean :
	rm -f simulace *.o
