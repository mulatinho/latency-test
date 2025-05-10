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

#define BUFFER_SIZE 4096
#define LATENCY_VERSION "1.0.9"
#define LATENCY_PORT 8080
#define LATENCY_SERVICE_REDIS 0
#define LATENCY_SERVICE_KAFKA 1
#define LATENCY_SERVICE_POSTGRES 2
#define LATENCY_SERVICES_SIZE 3
#define QUANTITY 10

#ifdef DEBUG
#define LATENCY_DEBUG(fmt, message...) fprintf(stderr, fmt, ## message)
#else
#define LATENCY_DEBUG(fmt, message...)
#endif

struct metric_latency {
    char service[NAME_MAX];
    int port;
    double latency;
};

#define QUANTITY 10

char *latency_host_to_ip(char *hostname);
double latency_calc(char *name, struct timespec start, struct timespec finish);
double latency_collect_kafka(char *server_host, int port);
double latency_collect_postgresql(char *server_host, int port);
double latency_collect_redis(char *server_host, int port);
int latency_collect_metric(char *type, char *hostname, int port, double latency);
int latency_collect(char *type, char *hostname, int port);
int latency_update_metrics(void);
int latency_start_server(void);
void latency_warn_error_and_quit(void);

#endif