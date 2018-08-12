
main:threadpool.o condition.o main.o
	gcc -g -Wall $^ -o $@ -lpthread -lrt

%.o:%.c
	gcc -c $^ -o $@

.PHONY:clean

clean:
	rm -f *.o
