ROOT=$(shell pwd)


.PHONY:all
all:httpd cgi

httpd:httpd.c ./threadpool/condition.c ./threadpool/threadpool.c
	gcc -o $@ $^ -pthread -lrt
cgi:
	cd $(ROOT)/wwwroot/cgi;make clean;make;cd -

.PHONY:clean
clean:
	rm -f httpd
	cd $(ROOT)/wwwroot/cgi;make clean;cd -
