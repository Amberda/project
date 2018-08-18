CC=gcc

Stress_Testing:webbench.o 
	gcc -Wall -o $@ $^

webbench.o:webbench.c socket.c

.PHONY:clean
clean:
	rm -f Stress_Testing *.o
