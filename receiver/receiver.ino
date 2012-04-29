#include <JeeLib.h>

MilliTimer measureTimer;

void setup() {

    Serial.begin(57600);

    Serial.println("\nJeeNode here.");

    rf12_initialize(1, RF12_868MHZ, 5);

    Serial.println("RF12 Initialized.");


}

#define STELLARIUM_NODE	7

void loop() {

	if(! rf12_recvDone())
	    return;

        if(rf12_crc == 0) {
		if(rf12_hdr == STELLARIUM_NODE) {
			if(rf12_len == 6) {
				// Assuming gravplug data
				int x, y, z; // signed (-255 to 255 on each axis, lost a bit???)

//				if(measureTimer.poll(100)) {
					Serial.write((const uint8_t *) rf12_data, rf12_len);
					Serial.println();

#if 0
					// recombine the 6 recv:d uint8's to 3 int16's 
					x = (rf12_data[1] << 8) ^ rf12_data[0];
					y = (rf12_data[3] << 8) ^ rf12_data[2];
					z = (rf12_data[5] << 8) ^ rf12_data[4];

					x /= 16;
					y /= 16;
					z /= 16;

					Serial.print("X: ");
					for(int i = -16; i < x; i++)
						Serial.print(" ");
					Serial.println("X");

					Serial.print("Y: ");
					for(int i = -16; i < y; i++)
						Serial.print(" ");
					Serial.println("Y");

					Serial.print("X: ");
					for(int i = -16; i < z; i++)
						Serial.print(" ");
					Serial.println("Z");

					Serial.print("X: "); Serial.println(x);
					Serial.print("Y: "); Serial.println(y);
					Serial.print("Z: "); Serial.println(z);
					Serial.println();
#endif
//				}
			}
		}
	}

}
