# a simple makefile
#

run: build
	./emcu

build: emcu emcu_test_server

emcu: emcu.o
	gcc -o emcu emcu.c

emcu_test_server: emcu_test_server.o
	gcc -o emcu_test_server emcu_test_server.o

emcu.o: emcu.c
	gcc -c emcu.c

emcu_test_server.o: emcu_test_server.c
	gcc -c emcu_test_server.c

.PHONY: run build
