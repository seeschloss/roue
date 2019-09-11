#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

void wait_for_status(int gpio, int status_expected, int repeats_needed, int delay) {
  int repeats_seen = 0;
  int status = -1;

  while (repeats_seen < repeats_needed) {
    delayMicroseconds(delay);
    status = digitalRead(gpio);

    if (status == status_expected) {
      repeats_seen += 1;
    } else {
      repeats_seen = 0;
    }
  }

  return;
}

int main(int argc, char** argv) {

  if (wiringPiSetup() == -1) {
    exit(1);
  }

  // pause between polling events
  int delay = 5000;

  // gpio pin to poll
  int gpio = 7;

  // how many turns per kWh
  int rate = 120;

  // duration in microseconds until change is registered
  int hysteresis = 200000;

  // stop after this many measures
  int count = 1;

  // print debug messages
  int debug = 0;

  int opt;

  // put ':' in the starting of the
  // string so that program can
  //distinguish between '?' and ':'
  while((opt = getopt(argc, argv, "d:r:h:g:vc:")) != -1) {
    switch(opt) {
      case 'c':
	count = atoi(optarg);
	break;
      case 'v':
	debug = 1;
	break;
      case 'd':
	delay = atoi(optarg);
	break;
      case 'r':
	rate = atoi(optarg);
	break;
      case 's':
	hysteresis = atoi(optarg);
	break;
      case 'g':
	gpio = atoi(optarg);
	break;
      case '?':
	fprintf(stderr, "unknown option: %c\n", optopt);
	return 1;
    }
  }

  pinMode(gpio, INPUT);

  int status = -1;
  int newstatus = 0;
  
  int last_change = 0;
  int length_0 = 0;
  int length_1 = 0;

  int min_samples = hysteresis / delay;
  if (debug) fprintf(stderr, "min samples: %i\n", min_samples);

  int discarding = 1;

  int samples = 0;
  float sum = 0;

  wait_for_status(gpio, 1, min_samples, delay);
  wait_for_status(gpio, 0, min_samples, delay);

  int start = 0;
  int length = 0;

  float watts_hour = 1.0/rate * 1000;
  float watts = 0;

  while (count != 0) {
    start = micros();
    wait_for_status(gpio, 1, min_samples, delay);
    wait_for_status(gpio, 0, min_samples, delay);
    length = micros() - start;

    watts = watts_hour / (length / 1000000 / 3600.0);

    if (debug) fprintf(stderr, "length: %i µs (%f s)\n", length, length / 1000000.0);
    if (debug) fprintf(stderr, "details: %f W (%f Wh over %fs)\n", watts, watts_hour, length / 1000000.0);

    fprintf(stdout, "power: %f W\n", watts);
    fflush(stdout);

    count--;
  }

  return 0;
}
