ROOT=$(shell pwd)

.PHONY:all
all:insert select push

#cgi:cgi.c
#	gcc -o $@ $^ -I ../include -L ../lib -lmysqlclient

insert:insert.c
	gcc -o $@ $^ -I ../include -L ../lib -lmysqlclient
select:select.c
	gcc -o $@ $^ -I ../include -L ../lib -lmysqlclient

push:pushmail.c
	gcc -o $@ $^ -I ../include -L ../lib -lmysqlclient

.PHONY:clean
clean:
	rm -f insert select push
