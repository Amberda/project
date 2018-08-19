CC=gcc

Stress_Testing:myTestTool.o 
	gcc -Wall -o $@ $^

myTestTool.o:myTestTool.c socket.c

.PHONY:clean
clean:
	rm -f Stress_Testing *.o
