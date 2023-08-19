#Shelly Makefile

all: shelly

shelly: shell.c
	gcc shell.c -Wall -oshelly -lreadline -lhistory

clean:
	rm -f a.out shelly
