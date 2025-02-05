all: check-latency

deb-deps:
	sudo apt-get install librdkafka-dev libpq-dev libhiredis-dev gcc -y

clean:
	rm -fv src/*~ src/*.o src/*.swp src/latency-test src/check-latency
	rm -fv tests/*~ tests/*.o tests/*.swp tests/unit-tests

image:
	docker build -t latency-test:latest .

latency-test:
	gcc -Wall -c -o src/latency-test.o src/latency-test.c -lrdkafka -lpq -lhiredis -I/usr/include/postgresql/ -I/usr/include/hiredis

check-latency: latency-test
	gcc -Wall -c -o src/check-latency.o src/check-latency.c -lrdkafka -lpq -lhiredis -I/usr/include/postgresql/ -I/usr/include/hiredis
	gcc -Wall -o src/check-latency src/check-latency.o src/latency-test.o -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis

latency-test-debug:
	gcc -W -g -ggdb -o src/latency-test src/latency-test.c -lrdkafka -lpq -lhiredis -I/usr/include/postgresql/ -I/usr/include/hiredis
