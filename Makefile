all: roue

roue: roue.c
	gcc roue.c -lwiringPi -oroue

install: roue
	install -m a+x,u+s roue /usr/bin
