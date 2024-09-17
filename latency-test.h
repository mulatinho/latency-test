/*
 * Latency tests on Kafka, Redis, PostgreSQL
 * Alexandre Mulatinho - 2020
 *
 * requirements:
 * $ apt-get install postgresql-dev librdkafka-dev libpq-dev libhiredis-dev
 *
 * compile:
 * $ gcc -o latency-test latency-test.c -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis
 *
 * usage:
 * $ export PGUSER=postgres PGPASS=password KAFKATOPIC=latency KAFKAGROUP=latency\n");
 * $ ./latency-test postgresql 127.0.0.1 5432
 * $ ./latency-test kafka 127.0.0.1 9092
 * $ ./latency-test redis 127.0.0.1 6379
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <librdkafka/rdkafka.h>
#include <libpq-fe.h>
#include <hiredis.h>

#define RETRY 10

char *get_ip(char *hostname);
double kafka_latency(char *server_host, int port);
double latency_measure(char *name, struct timespec start, struct timespec finish);
double postgresql_latency(char *server_host, int port);
double redis_latency(char *server_host, int port);
int handle_measure(char *type, char *hostname, int port);
void warn_error_and_quit();
