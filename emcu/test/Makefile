# a simple makefile
#

LDFLAGS = `pkg-config --libs-only-L ncursesw`
LDLIBS = `pkg-config --libs-only-l ncursesw`

build: emcu

test_build: test_emcu emcu_test_server

test_emcu: test_emcu.o utils.o draw.o tcp.o
	gcc -o test_emcu test_emcu.o utils.o draw.o tcp.o $(LDFLAGS) $(LDLIBS)

emcu: emcu.o utils.o draw.o tcp.o
	gcc -o emcu emcu.o utils.o draw.o tcp.o $(LDFLAGS) $(LDLIBS)

## object files

test_emcu.o: emcu.c emcu.h tcp.h draw.h
	gcc -c emcu.c -o test_emcu.o -D__TEST_BUILD__

emcu.o: emcu.c emcu.h tcp.h draw.h
	gcc -c emcu.c

utils.o: utils.h emcu.h utils.c
	gcc -c utils.c

draw.o: draw.h draw.c
	gcc -c draw.c

tcp.o: tcp.h tcp.c
	gcc -c tcp.c

# TEST SERVER
emcu_test_server: emcu_test_server.o esp_log.o
	gcc -o emcu_test_server emcu_test_server.o esp_log.o

emcu_test_server.o: emcu_test_server.c emcu_test_server.h emcu.h esp_log.h esp_err.h
	gcc -c emcu_test_server.c

esp_log.o: esp_log.c esp_log.h esp_err.h
	gcc -c esp_log.c

clean:
	rm *.o emcu test_emcu emcu_test_server

.PHONY: build test_build clean
