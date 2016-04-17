/*
 * BTSerial.h
 *
 *  Created on: Apr 16, 2016
 *      Author: Vincent Limorté
 *
 *  Software under license GPLv3 : http://www.gnu.org/licenses/gpl-3.0.fr.html
 */

#include <HardwareSerial.h>

#ifndef BTSERIAL_H_
#define BTSERIAL_H_

#define CMD_BAUDS 38400
#define BT_BUF_SIZE 64
#define BT_READ_TO 100

#ifdef BT_DEBUG

#ifndef BT_DEBUG_PORT
#define BT_DEBUG_PORT Serial
#endif //BT_DEBUG_PORT

#define BT_DEBUG_BEGIN(baud) BT_DEBUG_PORT.begin(baud)
#define BT_DEBUG_WRITE(...) BT_DEBUG_PORT.write(__VA_ARGS__)
#define BT_DEBUG_PRINT(...) BT_DEBUG_PORT.print(__VA_ARGS__)
#define BT_DEBUG_PRINTLN(...) BT_DEBUG_PORT.println(__VA_ARGS__)

#else
#define DEBUG_BEGIN(baud)
#define DEBUG_WRITE(...)
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)

#endif  // BT_DEBUG


class BTSerial {
public:
	BTSerial(HardwareSerial * HWS, int cmdPin, int pwrPin, int statePin = 0 );

	/**
	 * Power on + init serial transmission.
	 * cmd : true : start on command mode at 38400 bauds
	 * false start on communication mode at 'baud' bauds
	 */
	void powerOn(bool cmd, int baud=9600);
	void powerOff();

	void begin(int baud);
	void end();
	int available() const;
	int peek() const;
	int read();
	size_t write(uint8_t);
	size_t print(const char[]);
	int readUntil(char* buffer, char term, int size_buff, int timeout);
	void _cmd(bool);

	char* command(const char[], int timeout = 50);

	virtual ~BTSerial();
private:
	HardwareSerial* _serial;
	int _cmdPin;
	int _pwrPin;
	int _statePin;
	bool _powered;
	char _buffer[BT_BUF_SIZE];


};

#endif /* BTSERIAL_H_ */
