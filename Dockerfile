FROM debian:buster

WORKDIR /opt/app

RUN apt-get update -y && \
	apt-get install curl librdkafka-dev libpq-dev libhiredis-dev gcc default-jre -y

RUN curl -O https://archive.apache.org/dist/kafka/2.0.1/kafka_2.12-2.0.1.tgz && \
	tar xfz kafka_2.12-2.0.1.tgz -C /opt/app/

ENV PATH=$PATH:/opt/app/kafka_2.12-2.0.1/bin

COPY latency-test.* /opt/app/

RUN gcc -o latency-test latency-test.c -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis

CMD [ "./latency-test.sh" ]
