CC=gcc
CFLAGS=-Wall -I/usr/include/hiredis -I/usr/include -I/usr/include/postgresql
TEST_CFLAGS=-Wall -g -ggdb -DMLT_TESTING=1 -I/usr/include/hiredis -I/usr/include -I/usr/include/postgresql
LDLIBS=-lpq -lhiredis -lrdkafka

all: latency-test test

deb-deps:
	sudo apt-get install librdkafka-dev libpq-dev libhiredis-dev gcc -y

clean:
	rm -fv src/*~ src/*.o src/*.swp src/latency-test
	rm -fv tests/*~ tests/*.o tests/*.swp tests/*_test

image:
	docker build -t latency-test:latest .

push-image:
	docker push mulatinho/latency-test:latest

latency-test:
	gcc $(CFLAGS) -o src/latency-test src/latency-test.c $(LDLIBS)

latency-test-debug:
	gcc $(TEST_CFLAGS) -o src/latency-test src/latency-test.c $(LDLIBS)

test:
	gcc $(TEST_CFLAGS) -o src/latency-test.o -c src/latency-test.c $(LDLIBS)
	gcc $(TEST_CFLAGS) -o tests/latency-test_test.o -c tests/latency-test_test.c $(LDLIBS)
	gcc $(TEST_CFLAGS) -o tests/latency-test_test src/latency-test.o tests/latency-test_test.o $(LDLIBS) 


install:
	install -m 0755 src/latency-test /usr/local/bin/latency-test
	install -m 0644 doc/latency-test.1 /usr/local/share/man/man1/latency-test.1

