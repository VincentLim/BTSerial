/*
 * BTSerial_cmds.h
 *
 *  Created on: Jun 21, 2016
 *  Author: Vincent Limorté
 *
 *  Software under license GPLv3 : http://www.gnu.org/licenses/gpl-3.0.fr.html
 *
 */

#ifndef BTSERIAL_CMDS_H_
#define BTSERIAL_CMDS_H_

// Commands
#define BT_AT "AT"
#define BT_AT_TIME  50

#define BT_AT_VERSION  "AT+VERSION?"
#define BT_AT_VERSION_TIME  150

#define BT_AT_STATE "AT+STATE?"
#define BT_AT_STATE_TIME 150

#define BT_AT_ADDR "AT+ADDR?"
#define BT_AT_ADDR_TIME 150

#define BT_AT_NAME "AT+NAME?"
#define BT_AT_NAME_TIME 150

#define BT_AT_SET_NAME "AT+NAME="
#define BT_AT_SET_NAME_TIME 150

#define BT_AT_ROLE_GET "AT+ROLE?"
#define BT_AT_ROLE_GET_TIME 250

#define BT_AT_ROLE_SET "AT+ROLE="
#define BT_AT_ROLE_SET_TIME 150
#define BT_SET_ROLE_WAIT 500

#define BT_AT_INIT "AT+INIT"
#define BT_AT_INIT_TIME 150

#define BT_AT_SET_PSWD "AT+PSWD="
#define BT_AT_SET_PSWD_TIME 150

#define BT_AT_GET_PSWD "AT+PSWD?"
#define BT_AT_GET_PSWD_TIME 150

#define BT_AT_RMAAD "AT+RMAAD"
#define BT_AT_RMAAD_TIME 200

#define BT_AT_ADCN "AT+ADCN?"
#define BT_AT_ACDN_TIME 150

#endif /* BTSERIAL_CMDS_H_ */
