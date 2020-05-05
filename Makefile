all: latency-test

latency-test:
	gcc -o latency-test latency-test.c -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis

latency-test-debug:
	gcc -W -g -ggdb -o latency-test latency-test.c -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis

clean:
	rm -fv *~ latency-test
