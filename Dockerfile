FROM debian:bookworm-slim
ENV PATH=$PATH:/opt/app/kafka_2.12-2.0.1/bin

WORKDIR /app

RUN apt-get update -y && \
	apt-get install build-essential make curl librdkafka-dev libpq-dev libhiredis-dev gcc default-jre -y

COPY . /app/

RUN make

FROM debian:bookworm-slim 
WORKDIR /app

EXPOSE 8080

COPY --from=0 /app/src/latency-test /app/latency-test

CMD [ "/app/latency-test", "-s" ]
