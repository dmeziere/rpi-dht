/*
 *  dht22.c:
 *	Simple test program to test the wiringPi functions
 *	Based on the existing dht11.c
 */

#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAXTIMINGS 85

static int DHTPIN       = 0;
static int dht22_dat[5] = {0, 0, 0, 0, 0};

static int read_dht22_dat()
{
	uint8_t laststate = HIGH;
	uint8_t counter   = 0;
	uint8_t j         = 0, i;
	float   f; // fahrenheit
	
	dht22_dat[0] = dht22_dat[1] = dht22_dat[2] = dht22_dat[3] = dht22_dat[4] = 0;

	// pull pin down for 18 milliseconds
	pinMode(DHTPIN, OUTPUT);
	// digitalWrite(DHTPIN, HIGH);
	// delay(10);
	digitalWrite(DHTPIN, LOW);
	delay(18);

	// then pull it up for 40 microseconds
	digitalWrite(DHTPIN, HIGH);
	delayMicroseconds(40); 

	// prepare to read the pin
	pinMode(DHTPIN, INPUT);

	// detect change and read data
	for (i = 0; i < MAXTIMINGS; i++) {
		counter = 0;

		while (digitalRead(DHTPIN) == laststate) {
			counter++;
			delayMicroseconds(1);
			if (counter == 255) {
				break;
			}
		}
		laststate = digitalRead(DHTPIN);
	
		if (counter == 255) {
			break;
		}

		// ignore first 3 transitions
		if ((i >= 4) && (i % 2 == 0)) {

			// shove each bit into the storage bytes
			dht22_dat[j/8] <<= 1;
			if (counter > 16) {
				dht22_dat[j/8] |= 1;
			}
			j++;
		}
	}

	// check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
	// print it out if data is good
	if ((j >= 40) && (dht22_dat[4] == ((dht22_dat[0] + dht22_dat[1] + dht22_dat[2] + dht22_dat[3]) & 0xFF)) ) {
		float t, h;
		h = (float) dht22_dat[0] * 256 + (float) dht22_dat[1];
		h /= 10;
		t = (float) (dht22_dat[2] & 0x7F) * 256 + (float) dht22_dat[3];
		t /= 10.0;

		if ((dht22_dat[2] & 0x80) != 0) {
			t *= -1;
		}
		
		f = t * 9. / 5. + 32;
	
		printf("Humidity = %.2f %% Temperature = %.2f *C (%.1f *F).\n", h, t, f);

		return 1;
	
	} else {
		printf("Data not good, skip.\n");

		return 0;
	}
}

int main (int argc, char *argv[])
{
	if (argc != 2) {
		printf ("usage: %s <pin>\ndescription: pin is the wiringPi pin number\nusing 0 (GPIO 11).\n", argv[0]);
	} else {
		DHTPIN = atoi(argv[1]);
	}
	
	printf ("Raspberry Pi wiringPi DHT22 Temperature test program.\n");

	if (wiringPiSetup () == -1) {
		exit(EXIT_FAILURE);
	}
		
	if (setuid(getuid()) < 0) {
		perror("Dropping privileges failed.\n");
		exit(EXIT_FAILURE);
	}
	
	while (read_dht22_dat() == 0) {
		delay(1000); // wait 1sec to refresh
	}
	
	return 0;
}