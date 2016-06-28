/*
 * BTSerial.cpp
 *
 *  Created on: Apr 16, 2016
 *  Author: Vincent Limorté
 *
 *  Software under license GPLv3 : http://www.gnu.org/licenses/gpl-3.0.fr.html
 *
 */
#include <Arduino.h>
#define BT_DEBUG

#include "BTSerial.h"
#include "BTSerial_cmds.h"

char* strreplace(char* str, char pattern, char replace) {
	char* pos = str;
	while (*pos) {
		if (*pos == pattern) {
			*pos = replace;
		}
		++pos;
	}
	return str;
}

// Constructor
BTSerial::BTSerial(BTSERIAL * HWS, int cmdPin, int powerPin, int statePin) :
		_serial(HWS), _cmdPin(cmdPin), _pwrPin(powerPin), _statePin(statePin), _powered(
		false) {
	pinMode(_cmdPin, OUTPUT);
	pinMode(_pwrPin, OUTPUT);
	if (_statePin) {
		pinMode(_statePin, INPUT);
	}

	_serial->setTimeout(BT_READ_TO);

	_last = NONE;
}

BTSerial::~BTSerial() {
	_serial->end();
}

// Serial functions
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

// Module functions

void BTSerial::powerOn(bool cmd, int baud) {
	if (!_powered) {
		_cmd(cmd);
		int b = cmd ? CMD_BAUDS : baud;
		digitalWrite(_pwrPin, HIGH);
		_serial->begin(b);
		_powered = true;
		BT_DEBUG_PRINT(">> Bluetooth module power on. Baud rate : ");BT_DEBUG_PRINTLN(
				b);
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

// BlueTooth commands
char* BTSerial::state() {
	return command(BT_AT_STATE, BT_AT_STATE_TIME);
}

char* BTSerial::version() {
	return command(BT_AT_VERSION, BT_AT_VERSION_TIME);
}

char* BTSerial::address() {
	command(BT_AT_ADDR, BT_AT_ADDR_TIME);
	if (_last == SUCCESS) {
		return _storeAddress(_buffer);
	}
	return NULL;

}

int BTSerial::checkModule() {
	command(BT_AT, BT_AT_TIME);
	return (_last == SUCCESS);
}

char* BTSerial::name() {
	return command(BT_AT_NAME, BT_AT_NAME_TIME);
}

BTResult BTSerial::setName(const char* name) {

	char buff[41]; // Name limited to 32 bytes

	size_t len_cmd = strlen(BT_AT_SET_NAME);
	size_t len_name = strlen(name);
	size_t buf_end = min(40, len_name + len_cmd);
	strncpy(buff, BT_AT_SET_NAME, len_cmd);

	strncpy(buff + len_cmd, name, buf_end - len_cmd);
	buff[buf_end] = 0;

	int timeout = BT_AT_SET_NAME_TIME;
	if (len_name > 20) {
		timeout *= 2;
	}

	command(buff, timeout);

	return _last;
}

BTRole BTSerial::getRole() {
	command(BT_AT_ROLE_GET, BT_AT_ROLE_GET_TIME);
	if (_last == SUCCESS) {
		return _parseRole(_buffer);
	}
	return ROLE_ERROR;
}

BTResult BTSerial::setRole(BTRole role) {
	char cmd[16] = BT_AT_ROLE_SET;
	cmd[8] = role + '0';
	cmd[9] = '\0'; // TODO Est-ce vraiment nécessaire ?

	command(cmd, BT_AT_ROLE_SET_TIME);
	// wait a little for module to operate
	delay(BT_SET_ROLE_WAIT);
	return _last;
}

BTResult BTSerial::init() {
	command(BT_AT_INIT, BT_AT_INIT_TIME);
	return _last;
}

BTResult BTSerial::setPasswd(const char* passwd) {
	char cmd[16] = BT_AT_SET_PSWD;
	strncat(cmd, passwd, BT_PSWD_LEN);

	command(cmd, BT_AT_SET_PSWD_TIME);
	return _last;
}

char* BTSerial::getPasswd() {
	command(BT_AT_GET_PSWD, BT_AT_GET_PSWD_TIME);
	_parsePswd(_buffer);
	return _pswd;
}

BTResult BTSerial::deletePairList() {
	command(BT_AT_RMAAD, BT_AT_RMAAD_TIME);
	return _last;
}

int BTSerial::countPairList() {
	command(BT_AT_ADCN, BT_AT_ACDN_TIME);
	if (_last == SUCCESS) {
		return _parseInt(_buffer, "ADCN:");
	}

	return -1;
}

BTResult BTSerial::setCMode(BTCMode cMode) {
	char cmd[16] = BT_AT_SET_CMODE;
	size_t cmd_len = strlen(BT_AT_SET_CMODE);
	_last = NONE;
	if (cMode != CMODE_ERROR) {
		cmd[cmd_len] = cMode;
		//cmd[cmd_len+1]='\0';
		command(cmd, BT_AT_SET_CMODE_TIME);
	}
	return _last;
}

BTCMode BTSerial::getCMode() {
	command(BT_AT_GET_CMODE, BT_AT_GET_CMODE_TIME);
	if (_last == SUCCESS) {
		char*idx = strnxt(_buffer, "CMOD:");
		switch (*idx) {
		case '0':
			return FIXED;
		case '1':
			return ANY;
		case '2':
			return CMODE_SLAVE_LOOP;
		}
	}
	return CMODE_ERROR;
}

int BTSerial::link(char* addr) {
	strreplace(addr, ':', ',');
	char cmd[32] = BT_AT_LINK;
	size_t len = strlen(BT_AT_LINK);
	strncpy(cmd + len, addr, 14);
	command(cmd, BT_AT_LINK_TIME);

	return _last == SUCCESS;
}

int BTSerial::seekDevice(char* addr) {
	strreplace(addr, ':', ',');
	char cmd[32] = BT_AT_FSAD;
	size_t len = strlen(BT_AT_FSAD);
	strncpy(cmd + len, addr, 14);
	command(cmd, BT_AT_FSAD_TIME);

	return _last == SUCCESS;
}

BTResult BTSerial::setInqAC(char* accessCode) {
	char cmd[32]="";
	buildCmd(cmd,BT_AT_SET_IAC,accessCode);
	command(cmd, BT_SHORT);
	return _last;
}

int BTSerial::inquireDevices() {
	return 0;
}

// Utilities

int BTSerial::readUntil(char* buffer, char term, int size_buff, int timeout) {
	unsigned long end = millis() + timeout;
	int readCount = 0;
	char lastRead = 0;
	unsigned long lastTime;

	_serial->setTimeout(timeout);
	// todo : add something to detect errors
	while (readCount < size_buff && (readCount == 0 || lastRead != term)
			&& (lastTime = millis()) < end) {
		if (available()) {
			lastRead = read();
			buffer[readCount++] = lastRead;
		}
	}
	if (readCount >= size_buff) {
		_last = BUFF_OVERFLOW;
	} else if (lastTime >= end) {
		_last = TIMEOUT;
	}
	return readCount;
}

int BTSerial::readReturn(char* buffer, int size_buffer, int timeout) {

	int read = 0;
	bool success = false;
	bool failure = false;
	unsigned long time = millis();
	unsigned long endBefore = time + timeout;
	char* bufPos = buffer;
	_last = NONE;
	// read lines until OK, FAIL or ERROR
	while (!success && !failure && (time = millis()) < endBefore) {

		int lineS = readUntil(bufPos, BT_NL_CHAR, size_buffer - read, timeout);
		if (_last == BUFF_OVERFLOW) {
			BT_DEBUG_PRINTLN(">> Buffer Overflow.");
			// dump remaining chars in Serial
			dump(BT_READ_TO);
			return 0;
		} else if (_last == TIMEOUT) {
			BT_DEBUG_PRINTLN(">> Read Timeout.");
			// dump remaining chars in Serial
			dump(BT_READ_TO);
			return 0;
		}
		if (lineS >= 2 && bufPos[0] == 'O' && bufPos[1] == 'K') { // TODO change this awful comparison
			success = true;
			_last = SUCCESS;
		} else if (lineS >= 2 && bufPos[0] == 'F' && bufPos[1] == 'A'
				&& bufPos[2] == 'I' && bufPos[3] == 'L') {
			failure = true;
			_last = FAILURE;
		} else if (lineS >= 5 && bufPos[0] == 'E' && bufPos[1] == 'R'
				&& bufPos[2] == 'R' && bufPos[3] == 'O' && bufPos[4] == 'R') {
			failure = true;
			_last = FAILURE;
		}
		// change bufferPosition for next read
		bufPos += lineS;
		read += lineS;
	}
	if (time >= endBefore) {
		_last = TIMEOUT;
	}
	// reading over. Add termination to string and return result
	*bufPos = '\0';
	// log
	BT_DEBUG_PRINT(">> Command return : ");
	BT_DEBUG_PRINTLN(read);
	BT_DEBUG_PRINT("<<");
	BT_DEBUG_PRINTLN(buffer);

	return success;
}

BTResult BTSerial::getLastResult(char* result, int size) {
	// TODO I don't really know which strncpy I'm using..
	strncpy(result, _buffer, size);
	return _last;
}

int BTSerial::_parseInt(const char* cmdResult, const char* token) {
	char* idx = strnxt(cmdResult, token);
	if (idx != NULL) {
		return atoi(idx);
	}
	return -1;
}

char* BTSerial::strnxt(const char* str, const char* token) {
	char* idx = strstr(str, token);
	if (idx != NULL) {
		idx += strlen(token);
	}
	return idx;
}

char* BTSerial::_storeAddress(const char* cmdResult) {
	char*idx = strnxt(cmdResult, "ADDR:");
	char* ad = _address;
	while (*idx != '\r') {
		*ad++ = *idx++;
	}
	*idx = '\0';
	return _address;

}

BTRole BTSerial::_parseRole(char* cmdResult) {
	char* idx = strstr(cmdResult, "ROLE:"); // TODO : #define this
	if (idx != NULL) {
		idx += 5; // TODO no magic number
		switch (*idx) {
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

void BTSerial::_parsePswd(const char* cmdResult) {
	char* idx = strstr(cmdResult, "PSWD:"); // TODO : #define this
	if (idx != NULL) {
		idx += 5; // TODO no magic number
		strncpy(_pswd, idx, BT_PSWD_LEN);
		_pswd[4] = '\0';
	}
}

void BTSerial::dump(long timeout) {
	unsigned long end = millis() + timeout;
	int read = 0;
	BT_DEBUG_PRINT("<< DUMP ");
	while (millis() < end) {
		if (_serial->available()) {
			BT_DEBUG_PRINT((char )_serial->read());
			read++;
		}
	}
	BT_DEBUG_PRINT(" ## ");
	BT_DEBUG_PRINT(read);
	BT_DEBUG_PRINTLN(" chars dumped.");
}

char* BTSerial::command(const char cmd[], int timeout /*default=BT_READ_TO*/) {
	_cmd(true);
	BT_DEBUG_PRINT(">> Bluetooth sending command : ");
	BT_DEBUG_PRINTLN(cmd);
	print(cmd);
	print("\r\n");

	readReturn(_buffer, BT_BUF_SIZE, timeout);

//	delay(BT_READ_TO);
//	dump(BT_READ_TO);
	_cmd(false);
	return _buffer;
}

char* BTSerial::buildCmd(char* cmdBuff, const char* cmd,
		const char* arg) {
	size_t cmdLen = strlen(cmd);
	strncpy(cmdBuff, cmd, cmdLen);
	char* pos = cmdBuff + cmdLen;
	strncpy(pos, arg, strlen(arg));
	return cmdBuff;
}

char* BTSerial::buildCmd(char* cmdBuff, const char* cmd, char sep,
		const char* arg1, const char* arg2) {
	size_t cmdLen = strlen(cmd);
	size_t arg1Len = strlen(arg1);
	size_t arg2Len = strlen(arg2);
	strncpy(cmdBuff, cmd, cmdLen);
	char* pos = cmdBuff + cmdLen;
	strncpy(pos, arg1, arg1Len);
	pos += arg1Len;
	*pos++ = sep;
	strncpy(pos, arg2, arg2Len);

	return cmdBuff;
}

char* BTSerial::buildCmd(char* cmdBuff, const char* cmd, char sep,
		const char* arg1, const char* arg2, const char* arg3) {
	size_t cmdLen = strlen(cmd);
	size_t arg1Len = strlen(arg1);
	size_t arg2Len = strlen(arg2);
	size_t arg3Len = strlen(arg3);
	strncpy(cmdBuff, cmd, cmdLen);
	char* pos = cmdBuff + cmdLen;
	strncpy(pos, arg1, arg1Len);
	pos += arg1Len;
	*pos++ = sep;
	strncpy(pos, arg2, arg2Len);
	pos += arg2Len;
	*pos++ = sep;
	strncpy(pos, arg3, arg3Len);

	return cmdBuff;
}
