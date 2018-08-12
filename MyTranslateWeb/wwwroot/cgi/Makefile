ROOT=$(shell pwd)

.PHONY:all
all:insert dict

insert:insert.c
	gcc -o $@ $^ -I ../include -L ../lib -lmysqlclient
dict:dict.c
	gcc -o $@ $^ -I ../include -L ../lib -lmysqlclient

.PHONY:clean
clean:
	rm -f insert dict
