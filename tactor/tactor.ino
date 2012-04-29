// Demo of the Gravity Plug, based on the GravityPlug class in the Ports library
// 2010-03-19 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <JeeLib.h>

Port activity_led(3);

PortI2C myBus (1);
GravityPlug sensor (myBus);
MilliTimer measureTimer;


void setup () {

	activity_led.digiWrite(1);
	rf12_initialize(7, RF12_868MHZ, 5);
	sensor.begin();
	sensor.sensitivity(2, 25); // 2g, 25hz (lowest bandwidth, highest best signal/noise ratio)
}

void loop () {

	if(measureTimer.poll(50)) {

		const int* p = sensor.getAxes();

		rf12_sleep(RF12_WAKEUP);
		activity_led.digiWrite(1);

		while(! rf12_canSend())
			rf12_recvDone();

		rf12_sendStart(0, p, 3 * sizeof *p, 2);
		activity_led.digiWrite(0);
		rf12_sleep(RF12_SLEEP);
	}
}
