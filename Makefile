all: latency-test check-latency

deb-deps:
	sudo apt-get install librdkafka-dev libpq-dev libhiredis-dev gcc -y

latency-test:
	gcc -Wall -c -o latency-test.o latency-test.c -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis

check-latency: latency-test
	gcc -Wall -c -o check-latency.o check-latency.c -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis
	gcc -Wall -o check-latency check-latency.o latency-test.o -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis

tests: latency-test
	gcc -Wall -c -o tests/unit-tests.o tests/unit-tests.c  -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis
	gcc -Wall -o tests/unit-tests tests/unit-tests.o latency-test.o -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis

latency-test-debug:
	gcc -W -g -ggdb -o latency-test latency-test.c -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis

clean:
	rm -fv *~ *.o *.swp latency-test
	rm -fv tests/*~ tests/*.o tests/*.swp tests/unit-tests

image:
	docker build -t latency-test:latest .
