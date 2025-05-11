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

#ifndef _LATENCY_TEST_H_
#define _LATENCY_TEST_H_

#include <arpa/inet.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <hiredis.h>
#include <libpq-fe.h>
#include <librdkafka/rdkafka.h>

#define BUFFER_SIZE_MIN 4096
#define BUFFER_SIZE_MAX 8192
#define LATENCY_VERSION "1.0.9"
#define LATENCY_PORT 8080
#define LATENCY_SERVICE_REDIS 0
#define LATENCY_SERVICE_KAFKA 1
#define LATENCY_SERVICE_POSTGRES 2
#define LATENCY_SERVICES_SIZE 3
#define QUANTITY 10

#ifdef DEBUG
#define LATENCY_DEBUG_MACRO(_1, _2, FN, ...) FN
#define LATENCY_DEBUG1(message) fprintf(stderr, message);
#define LATENCY_DEBUG2(fmt, message...) fprintf(stderr, fmt, message);
#define LATENCY_DEBUG(...) LATENCY_DEBUG_MACRO(__VA_ARGS__, LATENCY_DEBUG2, LATENCY_DEBUG1)(__VA_ARGS__)
#else
#define LATENCY_DEBUG_MACRO(_1, _2, FN, ...) FN
#define LATENCY_DEBUG1(message) fprintf(stderr, message);
#define LATENCY_DEBUG2(fmt, message...) fprintf(stderr, fmt, message);
#define LATENCY_DEBUG(...) 
#endif

#define BUFFER_ZERO(bf) memset(bf, sizeof(bf), '\0')

char *latency_host_to_ip(char *hostname);
double latency_calc(char *name, struct timespec start, struct timespec finish);
double latency_collect_kafka(char *server_host, int port);
double latency_collect_postgresql(char *server_host, int port);
double latency_collect_redis(char *server_host, int port);
int latency_collect_metric(char *type, char *hostname, int port, double latency);
int latency_collect(char *type, char *hostname, int port);
int latency_update_metrics(char *);
int latency_start_server(void);
void latency_warn_error_and_quit(void);

#endif