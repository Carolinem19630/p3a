all: psort.c
	gcc -o psort psort.c
clean:
	rm psort
