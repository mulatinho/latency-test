#!/bin/bash
#
# Latency tests on Kafka, Redis, PostgreSQL
# Alexandre Mulatinho - 2020
#
# Init Script

#if [ ! -d "kafka_2.12-2.0.1" ] ; then
#        curl -O https://archive.apache.org/dist/kafka/2.0.1/kafka_2.12-2.0.1.tgz && \
#                tar xfz kafka_2.12-2.0.1.tgz -C . && rm -fv kafka_2.12-2.0.1.tgz
#fi

export PATH=$PATH:./kafka_2.12-2.0.1/bin

KAFKAENABLE=$KAFKAENABLE
POSTGRESENABLE=$POSTGRESENABLE
REDISENABLE=$REDISENABLE

PGPORT=$PGPORT
REDISPORT=$REDISPORT
KAFKAPORT=$KAFKAPORT

if [ -z "$KAFKAENABLE" ] ; then KAFKAENABLE=0; fi
if [ -z "$POSTGRESENABLE" ] ; then POSTGRESENABLE=0; fi
if [ -z "$REDISENABLE" ] ; then REDISENABLE=0; fi

if [ -z "$KAFKAPORT" ] ; then KAFKAPORT=9092; fi
if [ -z "$ZOOKEEPERPORT" ] ; then ZOOKEEPERPORT=2181; fi
if [ -z "$PGPORT" ] ; then PGPORT=5432; fi
if [ -z "$REDISPORT" ] ; then REDISPORT=6379; fi
        
##
## MEASURING LATENCY
##

if [ $KAFKAENABLE -eq 1 ] ; then
        if [ -z "$KAFKAHOST" -o -z "$ZOOKEEPERHOST" ] ; then
                echo ":. error: please make sure all environment variables are set"
                echo "export KAFKAHOST=127.0.0.1 ZOOKEEPERHOST=127.0.0.1"
                exit 1
        fi

        echo
        echo :. measuring kafka...

        # 01. Create a Topic
        #KAFKATOPIC=latency
        #kafka-topics.sh --create --topic $KAFKATOPIC \
        #        --zookeeper $ZOOKEEPERHOST:$ZOOKEEPERPORT --replication-factor 2 --partitions 2
        #sleep 2
        # 02. Create some messages
        #echo :. Producing one message
        #printf "%0.sA" {1..1000} | kafka-console-producer.sh --broker-list $KAFKAHOST:$KAFKAPORT --topic $KAFKATOPIC --property group.id=latency

        # 03. Now we can measure...
        ./latency-test kafka $KAFKAHOST $KAFKAPORT

        # 04. Drop the test topic
        #kafka-topics.sh --delete --topic $KAFKATOPIC --zookeeper $ZOOKEEPERHOST:$ZOOKEEPERPORT
fi

if [ $POSTGRESENABLE -eq 1 ] ; then
        if [ -z "$PGHOST" -o -z "$PGUSER" -o -z "$PGPASS" ] ; then
                echo ":. error: please make sure all environment variables are set"
                echo export PGHOST=127.0.0.1 PGUSER=postgres PGPASS=reverser00t
                exit 1
        fi

        echo
        echo :. measuring postgresql...
        ./latency-test postgresql $PGHOST $PGPORT
fi

if [ $REDISENABLE -eq 1 ] ; then
        if [ -z "$REDISHOST" ] ; then
                echo export REDISHOST=127.0.0.1
                exit 1
        fi

        echo
        echo :. measuring redis-server...
        ./latency-test redis $REDISHOST $REDISPORT
fi
