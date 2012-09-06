/*
 * librd - Rapid Development C library
 *
 * Copyright (c) 2012, Magnus Edenhill
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Apache Kafka consumer & producer example programs
 * using the Kafka driver from librd (https://github.com/edenhill/librd)
 */

/* Typical include path would be <librd/rd..h>, but this program
 * is builtin from within the librd source tree and thus differs. */
#include "rd.h"       /* librd base */
#include "rdopt.h"    /* for rd_opt*() */
#include "rdlog.h"    /* for rd_hexdump() */
#include "rdkafka.h"  /* for Kafka driver */


static int run = 1;

static void stop (int sig) {
	run = 0;
}


int main (int argc, char **argv) {
	rd_kafka_t *rk;
	char *broker = NULL;
	int mode_p = 0;
	int mode_c = 0;
	char *topic;
	int partition;
	/* Command line argument option definition. */
	rd_opt_t opts[] = {
		{ RD_OPT_BOOL|RD_OPT_MUT1|RD_OPT_REQ, 'P', "produce",
		  0, &mode_p, "Run as producer" },
		{ RD_OPT_BOOL|RD_OPT_MUT1|RD_OPT_REQ, 'C', "consume",
		  0, &mode_c, "Run as consumer" },
		{ RD_OPT_STR|RD_OPT_REQ, 't', "topic", 1, &topic, "Topic" },
		{ RD_OPT_INT|RD_OPT_REQ, 'p', "partition", 1, &partition,
		  "Partition" },
		{ RD_OPT_STR, 'b', "broker", 1, &broker, "Broker host:port" },
		{ RD_OPT_END },
	};
	

	/* Initialize librd */
	rd_init();

	/* Parse command line arguments. */
	if (!rd_opt_get(opts, argc, argv, NULL, NULL))
		exit(1);

	signal(SIGINT, stop);

	if (mode_p) {
		/*
		 * Producer
		 */
		char buf[1024];

		/* Create Kafka handle */
		if (!(rk = rd_kafka_new(RD_KAFKA_PRODUCER, broker, NULL))) {
			perror("kafka_new producer");
			exit(1);
		}

		fprintf(stderr, "%% Type stuff and hit enter to send\n");
		while (run && (fgets(buf, sizeof(buf), stdin))) {
			int len = strlen(buf);
			/* Send/Produce message. */
			rd_kafka_produce(rk, topic, partition, 0, buf, len);
			fprintf(stderr, "%% Sent %i bytes to topic "
				"%s partition %i\n", len, topic, partition);
		}

		/* Destroy the handle */
		rd_kafka_destroy(rk);

	} else {
		/*
		 * Consumer
		 */
		rd_kafka_op_t *rko;
		/* Base our configuration on the default config. */
		rd_kafka_conf_t conf = rd_kafka_defaultconf;


		/* The offset storage file is optional but its presence
		 * avoids starting all over from offset 0 again when
		 * the program restarts.
		 * ZooKeeper functionality will be implemented in future
		 * versions and then the offset will be stored there instead. */
		conf.consumer.offset_file = "."; /* current directory */

		/* Indicate to rdkafka that the application is responsible
		 * for storing the offset. This allows the application to
		 * succesfully handle a message before storing the offset.
		 * If this flag is not set rdkafka will store the offset
		 * just prior to returning the message from rd_kafka_consume().
		 */
		conf.flags |= RD_KAFKA_CONF_F_APP_OFFSET_STORE;



		/* Use the consumer convenience function
		 * to create a Kafka handle. */
		if (!(rk = rd_kafka_new_consumer(broker, topic,
						 (uint32_t)partition,
						 0, &conf))) {
			perror("kafka_new_consumer");
			exit(1);
		}

		while (run) {
			/* Fetch an "op" which is one of:
			 *  - a kafka message (if rko_len>0 && rko_err==0)
			 *  - an error (if rko_err)
			 */
			if (!(rko = rd_kafka_consume(rk, 1000/*timeout ms*/)))
				continue;
			
			if (rko->rko_err)
				fprintf(stderr, "%% Error: %.*s\n",
					rko->rko_len, rko->rko_payload);
			else if (rko->rko_len) {
				fprintf(stderr, "%% Message with "
					"next-offset %"PRIu64" is %i bytes\n",
					rko->rko_offset, rko->rko_len);
				rd_hexdump(stdout, "Message",
					   rko->rko_payload, rko->rko_len);
			}

			/* rko_offset contains the offset of the _next_
			 * message. We store it when we're done processing
			 * the current message. */
			if (rko->rko_offset)
				rd_kafka_offset_store(rk, rko->rko_offset);

			/* Destroy the op */
			rd_kafka_op_destroy(rk, rko);
		}

		/* Destroy the handle */
		rd_kafka_destroy(rk);
	}

	return 0;
}
