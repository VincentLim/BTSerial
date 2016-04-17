/*
 * BTSerial.cpp
 *
 *  Created on: Apr 16, 2016
 *      Author: Vincent Limorté
 *   Software under license GPLv3 : http://www.gnu.org/licenses/gpl-3.0.fr.html
 */
#include <Arduino.h>

#define BT_DEBUG

#include "BTSerial.h"


BTSerial::BTSerial(HardwareSerial * HWS, int cmdPin, int powerPin, int statePin) :
		_serial(HWS), _cmdPin(cmdPin), _pwrPin(powerPin), _statePin(statePin), _powered(
		false) {

	pinMode(_cmdPin, OUTPUT);
	pinMode(_pwrPin, OUTPUT);
	if (_statePin) {
		pinMode(_statePin, INPUT);
	}

	_serial->setTimeout(BT_READ_TO);



}
;

BTSerial::~BTSerial() {

}

void BTSerial::powerOn(bool cmd, int baud) {
	if (!_powered) {
		_cmd(cmd);
		int b = cmd ? CMD_BAUDS:baud;
		digitalWrite(_pwrPin, HIGH);
		_serial->begin(b);


		_powered = true;
		BT_DEBUG_PRINT(">> Bluetooth module power on. Baud rate : ");
		BT_DEBUG_PRINTLN(b);

	}
}

void BTSerial::powerOff() {
	if (_powered) {
		_serial->end();
		delay(100);
		digitalWrite(_pwrPin, LOW);
		_powered = false;
		BT_DEBUG_PRINTLN(">> Bluetooth module power off");
	}
}

void BTSerial::begin(int baud) {
	_serial->begin(baud);
}

void BTSerial::end() {
	_serial->end();
}

int BTSerial::available() const {
	return _serial->available();
}

int BTSerial::peek() const {
	return _serial->peek();
}

int BTSerial::read() {
	return _serial->read();
}

size_t BTSerial::write(uint8_t unsignedChar) {
	return _serial->write(unsignedChar);
}

size_t BTSerial::print(const char charArray[]) {
	return _serial->print(charArray);
}

int BTSerial::readUntil(char* buffer, char term, int size_buff, int timeout) {
	unsigned long end = millis() + timeout;
	int cread = 0;
	char lread = 0;

	while (cread < size_buff && (cread == 0 || lread != term) && millis() < end) {
		if (available()) {
			lread = read();
			buffer[cread++] = lread;
		}
	}
	return cread;

}

void BTSerial::_cmd(bool cmd) {
	if (cmd) {
		digitalWrite(_cmdPin, HIGH);
		BT_DEBUG_PRINTLN(">> Bluetooth entering command mode.");
	} else {
		digitalWrite(_cmdPin, LOW);
		BT_DEBUG_PRINTLN(">> Bluetooth leaving command mode.");
	}
	delay(200);
}



char* BTSerial::command(const char cmd[], int timeout) {
	char* result = 0;
	_cmd(true);
	BT_DEBUG_PRINT(">> Bluetooth sending command : ");
	BT_DEBUG_PRINTLN(cmd);
	print(cmd);
	print("\r\n");



	// read return
	int recvd = readUntil(_buffer, '\n',BT_BUF_SIZE, timeout);
	BT_DEBUG_PRINT("Received ");
	BT_DEBUG_PRINTLN(recvd);
	if (recvd > 0) {
		_buffer[recvd]='\0';
		BT_DEBUG_PRINTLN(_buffer);

		result = _buffer;

	}
	delay(BT_READ_TO);
	_cmd(false);
	return result;
}
