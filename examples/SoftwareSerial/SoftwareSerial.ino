/*
 * SoftwareSerial.ino
 *
 *  Created on: May 6, 2016
 *      Author: Vincent Limorté
 *
 *  Software under license GPLv3 : http://www.gnu.org/licenses/gpl-3.0.fr.html
 *
 *
 */

#define BT_DEBUG
#define BT_USE_SOFTWARE_SERIAL

#include <BTSerial.h>
#include <SoftwareSerial.h>

#define BT_CMD 42
#define BT_PWR 43

// SoftwareSerial for BlueTooth communications RX on 8, TX on 9
SoftwareSerial SwSerial(8, 9);

BTSerial BT(&SwSerial, BT_CMD, BT_PWR);

void setup() {
	// Init Serials.
	// Hardware Serial for debug & monitor
	Serial.begin(9600);
	BT.begin(9600);

	BT.checkModule();

	delay(500);
}

// The loop function is called in an endless loop
void loop() {
	while (BT.available()) {
		Serial.write(BT.read());
	}

	while (Serial.available()) {
		BT.write(Serial.read());
	}
}

