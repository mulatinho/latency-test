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

#include "latency-test.h"

double latency_measure(char *name, struct timespec start, struct timespec finish)
{
	long seconds = finish.tv_sec - start.tv_sec;
	long ns = finish.tv_nsec - start.tv_nsec;
        double result;

	if (start.tv_nsec > finish.tv_nsec) { --seconds; ns += 1000000000; }
        result = (double)ns/(double)1000000;
	fprintf(stdout, ":. [%s] milliseconds: %fms\n", name, result);

    return result;
}

char *get_ip(char *hostname)
{
        struct hostent *host;
        struct sockaddr_in sock_addr;


        if ((host = gethostbyname(hostname))) {
                sock_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
                return inet_ntoa(sock_addr.sin_addr);
        }

        return NULL;
}

double postgresql_latency(char *server_host, int port)
{
	char conninfo[256];
	PGconn *conn;
	struct timespec t1, t2, t3;
        double result = 0.0;

	char *pg_user = getenv("PGUSER");
	char *pg_pass = getenv("PGPASS");

	if (!pg_user || !pg_pass)
		snprintf(conninfo, sizeof(conninfo)-1, "host=%s port=%d dbname=postgres user=postgres", server_host, port);
	else
		snprintf(conninfo, sizeof(conninfo)-1, "host=%s port=%d dbname=postgres user=%s password=%s",
			server_host, port, pg_user, pg_pass);

	clock_gettime(CLOCK_REALTIME, &t1);

	conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, ":. connection string: %s\n:. connection refused while trying to connect into %s...", conninfo, server_host);
		PQfinish(conn);
		exit(-1);
	}

	clock_gettime(CLOCK_REALTIME, &t2);
	latency_measure("postgres->connection", t1, t2);

	PQexec(conn, "select count(*) from pg_catalog.pg_type");

	clock_gettime(CLOCK_REALTIME, &t3);
	latency_measure("postgres->response", t2, t3);

	result = latency_measure("postgres->total", t1, t3);
	PQfinish(conn);

        return result;
}

double redis_latency(char *server_host, int port)
{
	redisContext *redis_ctx;
	redisReply *reply;
	struct timespec t1, t2, t3;
        double result = 0.0;

	clock_gettime(CLOCK_REALTIME, &t1);

	redis_ctx = redisConnect(server_host, port);

	if (redis_ctx != NULL && redis_ctx->err) {
		fprintf(stderr, ":. connection refused while trying to connect into %s...", server_host);
		exit(-1);
	}

	clock_gettime(CLOCK_REALTIME, &t2);
	latency_measure("redis->connection", t1, t2);

	reply = redisCommand(redis_ctx, "KEYS *");
	clock_gettime(CLOCK_REALTIME, &t3);
	latency_measure("redis->response", t2, t3);
	freeReplyObject(reply);

	result = latency_measure("redis->total", t1, t3);
        redisFree(redis_ctx);

        return result;
}

double kafka_latency(char *server_host, int port)
{
	rd_kafka_t *rk;
	rd_kafka_conf_t *conf;
        rd_kafka_topic_partition_list_t *subscription;
	struct timespec t1, t2, t3;
        double result = 0;
	char broker[512], errstr[512];
        char *topic_name;;
        int running = 1;

	clock_gettime(CLOCK_REALTIME, &t1);

        conf = rd_kafka_conf_new();

	if (rd_kafka_conf_set(conf, "client.id", server_host,
		errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
		goto config_failed;
	}

#ifdef DEBUG
	if (rd_kafka_conf_set(conf, "debug", "broker,topic,metadata,fetch",
		errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
		goto config_failed;
	}
#endif

	snprintf(broker, sizeof(broker)-1, "%s:%d", server_host, port);
	if (rd_kafka_conf_set(conf, "bootstrap.servers", broker,
		errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
		goto config_failed;
	}


	if (rd_kafka_conf_set(conf, "group.id", "latency",
	        errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
		goto config_failed;
	}

	if (!(rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr)))) {
		fprintf(stderr, ":. Failed to create new consumer: %s\n", errstr);
		goto config_failed;
	}

        conf = NULL;

        rd_kafka_poll_set_consumer(rk);

        subscription = rd_kafka_topic_partition_list_new(1);

        if (!(topic_name = getenv("KAFKATOPIC"))) {
		fprintf(stderr, ":. You need to set KAFKATOPIC environment variable\n");
		goto config_failed;
        }

        rd_kafka_topic_partition_list_add(subscription, topic_name, RD_KAFKA_PARTITION_UA);
	
        if (rd_kafka_subscribe(rk, subscription) != RD_KAFKA_RESP_ERR_NO_ERROR) {
                fprintf(stderr, ":. failed to subscribe on topic\n");
                goto config_failed;
        }

        rd_kafka_topic_partition_list_destroy(subscription);

        clock_gettime(CLOCK_REALTIME, &t2);
	latency_measure("kafka->connection", t1, t2);

	while (running) {
		rd_kafka_message_t *rkmessage = rd_kafka_consumer_poll(rk, 100);
		if (rkmessage) {
#ifdef DEBUG
			fprintf(stdout, ":. Message fetched on topic %s, metadata:(offset %ld, %zd bytes):\n",
				rd_kafka_topic_name(rkmessage->rkt), rkmessage->offset, rkmessage->len);
#endif
			
                        clock_gettime(CLOCK_REALTIME, &t3);
			latency_measure("kafka->fetch_topic", t1, t3);

                        rd_kafka_message_destroy(rkmessage);
                        running--;
                }
	}

	result = latency_measure("kafka->total", t1, t3);
	goto exit_nicely;

config_failed:
	fprintf(stderr, ":. critical error: %s\n", errstr);
	rd_kafka_destroy(rk);

	exit(2);

exit_nicely:
	rd_kafka_destroy(rk);
        return result;
}

void warn_error_and_quit()
{
	fprintf(stderr, "usage: ./latency-test <kafka|redis|postgresql> 127.0.0.1 31200\n");
	exit(1);
}

int handle_measure(char *type, char *hostname, int port)
{
	int i = 0;
        double sum = 0.0, result = 0.0;
        double (*func)(char *, int);
        char *ipaddr = NULL;

	if (!strncmp(type, "kafka", 5))
	        func = kafka_latency;
	else if (!strncmp(type, "redis", 5))
		func = redis_latency;
	else if (!strncmp(type, "postgresql", 10))
	        func = postgresql_latency;
	else
		warn_error_and_quit();

        if ((ipaddr = get_ip(hostname)) == NULL) {
                fprintf(stderr, "could not resolve the ip address: %s\n", hostname);
                exit(99);
        }

        while (i++ < RETRY) {
		sum += func(ipaddr, port);
                usleep(1500);
        }

        result = sum / RETRY;
        fprintf(stdout, ":. [%s] latency in average is: %fms (%d tests were executed)\n", type, result, RETRY);

	return 0;
}
