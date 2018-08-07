ROOT=$(shell pwd)


dict:dict.c:
	gcc -o $@ $^ -I ../include -L ../lib -lmysqlclient

.PHONY:clean
clean:
	rm -f dict 
