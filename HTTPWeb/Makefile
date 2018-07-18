ROOT=$(shell pwd)


.PHONY:all
all:httpd cgi

httpd:httpd.c
	gcc -o $@ $^ -pthread
cgi:
	cd $(ROOT)/wwwroot/cgi;make clean;make;cd -

.PHONY:clean
clean:
	rm -f httpd
	cd $(ROOT)/wwwroot/cgi;make clean;cd -
