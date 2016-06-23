/*
 * BTSerial.h
 *
 *  Created on: Apr 16, 2016
 *      Author: Vincent Limorté
 *
 *  Software under license GPLv3 : http://www.gnu.org/licenses/gpl-3.0.fr.html
 *
 *
 */

#ifndef BTSERIAL_H_
#define BTSERIAL_H_

#ifdef BT_USE_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
#define BTSERIAL SoftwareSerial
#else
#include <HardwareSerial.h>
#define BTSERIAL HardwareSerial
#endif

#define CMD_BAUDS 38400
#define BT_BUF_SIZE 64
#define BT_READ_TO 50
#define BT_NL_CHAR '\n'

#ifdef BT_DEBUG

#ifndef BT_DEBUG_PORT
#define BT_DEBUG_PORT Serial
#endif //BT_DEBUG_PORT

#define BT_DEBUG_BEGIN(baud) BT_DEBUG_PORT.begin(baud)
#define BT_DEBUG_WRITE(...) BT_DEBUG_PORT.write(__VA_ARGS__)
#define BT_DEBUG_PRINT(...) (BT_DEBUG_PORT.print(__VA_ARGS__))
#define BT_DEBUG_PRINTLN(...) BT_DEBUG_PORT.println(__VA_ARGS__)

#else
#define BT_DEBUG_BEGIN(baud)
#define BT_DEBUG_WRITE(...)
#define BT_DEBUG_PRINT(...)
#define BT_DEBUG_PRINTLN(...)

#endif  // BT_DEBUG


enum BTRole{
	SLAVE=0,
	MASTER=1,
	SLAVE_LOOP=2,
	ROLE_ERROR=-1
};

enum BTResult {
	SUCCESS,
	FAILURE,
	TIMEOUT,
	BUFF_OVERFLOW,
	NONE
};

struct Device {
	char address[14];
	char name[64];
};

class BTSerial {
public:


	BTSerial(BTSERIAL * SWS, int cmdPin, int pwrPin, int statePin = 0 );

	/**
	 * Power on + init serial transmission.
	 * cmd : true : start on command mode at 38400 bauds
	 * false start on communication mode at 'baud' bauds
	 */
	void powerOn(bool cmd, int baud = 9600);
	void powerOff();

	void begin(int baud);
	void end();
	int available() const;
	int peek() const;
	int read();
	size_t write(uint8_t);
	size_t print(const char[]);

	void _cmd(bool);

	char* command(const char[], int timeout = BT_READ_TO);

	// AT commands
	char* version();
	int checkModule();
	char* state();
	char* address();
	char* name();
	BTResult setName(const char* name);
	BTRole getRole();
	BTResult setRole(BTRole role);
	BTResult init();


	BTResult getLastResult(char* result, int size);

	virtual ~BTSerial();
private:

	BTSERIAL* _serial;
	int _cmdPin;
	int _pwrPin;
	int _statePin;
	bool _powered;
	char _buffer[BT_BUF_SIZE];
	BTResult _last;

	int readUntil(char* buffer, char term, int size_buff, int timeout);
	int readReturn(char* buffer, int size_buff, int timeout);
	void dump(long timeout);
	BTRole _parseRole(char* cmdResult);
};

#endif /* BTSERIAL_H_ */
