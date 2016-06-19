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


BTSerial::BTSerial(BTSERIAL * HWS, int cmdPin, int powerPin, int statePin) :
		_serial(HWS), _cmdPin(cmdPin), _pwrPin(powerPin), _statePin(statePin), _powered(
		false)
{
	pinMode(_cmdPin, OUTPUT);
	pinMode(_pwrPin, OUTPUT);
	if (_statePin) {
		pinMode(_statePin, INPUT);
	}

	_serial->setTimeout(BT_READ_TO);

	_last=NONE;
}

char* BTSerial::state(){
	return command(BT_AT_STATE, BT_AT_STATE_TIME);
}

char* BTSerial::version() {
	return command(BT_AT_VERSION,BT_AT_VERSION_TIME);
}


char* BTSerial::address() {
	return command(BT_AT_ADDR, BT_AT_ADDR_TIME);
}

int BTSerial::checkModule() {
	command(BT_AT, BT_AT_TIME);
	return (_last==SUCCESS);
}

BTSerial::~BTSerial() {
 _serial->end();
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
	int readCount = 0;
	char lastRead = 0;
	unsigned long lastTime;

	_serial->setTimeout(timeout);
	// todo : add something to detect errors
	while (readCount < size_buff && (readCount == 0 || lastRead != term) && (lastTime=millis()) < end) {
		if (available()){
			lastRead = read();
			buffer[readCount++] = lastRead;
		}
	}
	if(readCount >= size_buff){
		_last=BUFF_OVERFLOW;
	} else if(lastTime>=end){
		_last=TIMEOUT;
	}
	return readCount;

}

int BTSerial::readReturn(char* buffer, int size_buffer, int timeout){

	int read = 0;
	bool success=false;
	bool failure=false;
	unsigned long time=millis();
	unsigned long endBefore=time+timeout;
	char* bufPos = buffer;
	_last=NONE;
	// read lines until OK, FAIL or ERROR
	while(!success && !failure && (time=millis())<endBefore){

		int lineS = readUntil(bufPos,BT_NL_CHAR, size_buffer-read, timeout);
		if(_last==BUFF_OVERFLOW){
			BT_DEBUG_PRINTLN(">> Buffer Overflow.");
			// dump remaining chars in Serial
			dump(BT_READ_TO);
			return 0;
		} else if(_last==TIMEOUT){
			BT_DEBUG_PRINTLN(">> Read Timeout.");
			// dump remaining chars in Serial
			dump(BT_READ_TO);
			return 0;
		}
		if(lineS >= 2 && bufPos[0]=='O' && bufPos[1]=='K'){ // TODO change this awful comparison
			success=true;
			_last=SUCCESS;
		}
		else if (lineS>=2 && bufPos[0]=='F' && bufPos[1]=='A' && bufPos[2]=='I' && bufPos[3]=='L'){
			failure=true;
			_last=FAILURE;
		}
		else if (lineS >=5 && bufPos[0]=='E' && bufPos[1]=='R' && bufPos[2]=='R' && bufPos[3]=='O' && bufPos[4]=='R'){
			failure=true;
			_last=FAILURE;
		}
		// change bufferPosition for next read
		bufPos+=lineS;
		read+=lineS;
	}
	if(time>=endBefore){
		_last=TIMEOUT;
	}
	// reading over. Add termination to string and return result
	*bufPos='\0';
	// log
	BT_DEBUG_PRINT(">> Command return : ");
	BT_DEBUG_PRINTLN(read);
	BT_DEBUG_PRINTLN(buffer);

	return success;

}

BTResult BTSerial::getLastResult(char* result, int size) {
	// TODO I don't really know which strncpy I'm using..
	strncpy(result, _buffer, size);
	return _last;
}

char* BTSerial::name() {
	return command(BT_AT_NAME, BT_AT_NAME_TIME);
}

BTRole BTSerial::getRole() {
	BT_DEBUG_PRINT(">> Asking Role : ");

	command(BT_AT_ROLE_GET, BT_AT_ROLE_GET_TIME);
	if(_last==SUCCESS){
		return _parseRole(_buffer);
	}
	return ROLE_ERROR;
}



BTResult BTSerial::setRole(BTRole role) {
	char cmd[16]=BT_AT_ROLE_SET;
	cmd[8]=role;
	cmd[9]='\0'; // TODO Est-ce vraiment nécessaire ?

	command(cmd, BT_AT_ROLE_SET_TIME);

	return _last;
}

BTRole BTSerial::_parseRole(char* cmdResult){
	char* idx=strstr(cmdResult, "ROLE:"); // TODO : #define this
	if(idx != NULL){
		idx+=5; // TODO no magic number
		switch(*idx){
			case '0':
				return SLAVE;
			case '1':
				return MASTER;
			case '2':
				return SLAVE_LOOP;
		}
	}
	BT_DEBUG_PRINTLN("ERROR");
	return ROLE_ERROR;
}

void BTSerial::dump(long timeout){
	unsigned long end = millis() + timeout;
	while(millis()<end){
		while(_serial->available()){
			_serial->read();
		}
	}
}

void BTSerial::_cmd(bool cmd) {
	if (cmd) {
		digitalWrite(_cmdPin, HIGH);
		BT_DEBUG_PRINTLN(">> Bluetooth entering command mode.");
	} else {
		digitalWrite(_cmdPin, LOW);
		_serial->setTimeout(BT_READ_TO);
		BT_DEBUG_PRINTLN(">> Bluetooth leaving command mode.");
	}

}



char* BTSerial::command(const char cmd[], int timeout /*=50*/) {
	_cmd(true);
	BT_DEBUG_PRINT(">> Bluetooth sending command : ");
	BT_DEBUG_PRINTLN(cmd);
	print(cmd);
	print("\r\n");

	readReturn(_buffer, BT_BUF_SIZE, timeout);

	delay(BT_READ_TO);
	_cmd(false);
	return _buffer;
}


