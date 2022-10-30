all: psort.c
	gcc -o psort psort.c -Wall -Werror -pthread -O
clean:
	rm psort
