all: latency-test

#TODO deb:

deb-deps:
	sudo apt-get install librdkafka-dev libpq-dev libhiredis-dev gcc -y

clean:
	rm -fv src/*~ src/*.o src/*.swp src/latency-test
	rm -fv tests/*~ tests/*.o tests/*.swp tests/unit-tests

image:
	docker build -t latency-test:latest .

push-image:
	docker push mulatinho/latency-test:latest

latency-test:
	gcc -Wall -o src/latency-test src/latency-test.c -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis

latency-test-debug:
	gcc -W -g -ggdb -o src/latency-test src/latency-test.c -lrdkafka -lpq -lhiredis -I/usr/include/postgresql/ -I/usr/include/hiredis

install:
	install -m 0755 src/check-latency /usr/local/bin/check-latency
	install -m 0644 doc/latency-test.1 /usr/local/share/man/man1/latency-test.1

