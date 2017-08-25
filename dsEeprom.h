//
// ************************************************************************
// dsEeprom
// (C) 2016 Dirk Schanz aka dreamshader
// ************************************************************************
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ************************************************************************
//
//   A library to provide simplified access to the onchip EEPROM of an
//   Arduino or ESP8266.
//   This includes a checksum over the whole content, that is 
//   automatically stored and checked on EEPROM access, a mgic byte, 
//   that signals a valid content and provides information regarding 
//   the software version with that the EEPROM was written, simplified 
//   store and restore of strings, char array and so on.
//   Please refer to the specific funtion descrition for further
//   information.
//
// ************************************************************************
//
//
//-------- History --------------------------------------------------------
//
// 2016/10/28: initial version 
// 
//
// ************************************************************************

#ifndef _DSEEPROM_H_
#define _DSEEPROM_H_

#include <inttypes.h>
#include <stdarg.h>
#include <EEPROM.h>

#ifdef USE_SIMPLE_LOG
#include <SimpleLog.h>
#else
#define LOGLEVEL_QUIET    0
#define LOGLEVEL_DEFAULT  0
#define LOGLEVEL_INFO     0
#endif // USE_SIMPLE_LOG


#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

extern "C" {
}

//
//
// EEPROM_MAX_SIZE depending on MCU in use
// 
//   512 bytes on ATmega168 and ATmega8, 
//  1024 bytes on ATmega328  
//  4096 bytes on ATmega1280 and ATmega2560
//  up to 4096 bytes on ESP8266
//
#if defined (__AVR_ATmega8__) || defined (__AVR_ATmega168__)
#define EEPROM_MAX_SIZE                   512
#endif
//
#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega32U4__)
#define EEPROM_MAX_SIZE                  1024
#endif
//
#if defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
#define EEPROM_MAX_SIZE                  4096
#endif
//
#ifdef ESP8266
#define EEPROM_MAX_SIZE                  4096
#endif // ESP8266
//
//
// minimal error codes
//
#define E_SUCCESS        0
#define E_BAD_CRC       -3
#define E_INVALID_MAGIC -2
//
// The member function version2Magic() has no real funtionality at this time.
// It simply returns the defined value of EEPROM_MAGIC_BYTE
// in a later version this may be replaced by a real calculation or
// mapping 
//
#define EEPROM_MAGIC_BYTE              0x7e
//
#define EEPROM_LEADING_LENGTH             2  // means two byte representing 
//                                           // the real length of the data field
#define EEPROM_MAXLEN_MAGIC               1
#define EEPROM_MAXLEN_CRC32               4
//
#define EEPROM_MAXLEN_BOOLEAN             1
#define EEPROM_MAXLEN_LONG                4
#define EEPROM_MAXLEN_SHORT               2
#define EEPROM_MAXLEN_CHAR                1
//
#define EEPROM_MAXLEN_WLAN_SSID          32  // max. length a SSID may have
#define EEPROM_MAXLEN_WLAN_PASSPHRASE    64  // max. length of a WLAN passphrase
#define EEPROM_MAXLEN_SERVER_IP          19  // max. length for the server IP
#define EEPROM_MAXLEN_SERVER_PORT         4  // max. length for the server port
#define EEPROM_MAXLEN_NODENAME           32  // max. lenght of the (generated) nodename
#define EEPROM_MAXLEN_ADMIN_PASSWORD     32  // max. length for admin password
//
//
// predefined standard layout of the eeprom:
//
#define EEPROM_HEADER_BEGIN         0
//
#define EEPROM_POS_MAGIC            0
//
#define EEPROM_POS_CRC32            (EEPROM_POS_MAGIC + EEPROM_MAXLEN_MAGIC)
//
#define EEPROM_HEADER_END           (EEPROM_POS_CRC32 + EEPROM_MAXLEN_CRC32 + EEPROM_LEADING_LENGTH)
//
// data area begins here
//
#define EEPROM_STD_DATA_BEGIN       EEPROM_HEADER_END       
//
#define EEPROM_POS_WLAN_SSID        EEPROM_STD_DATA_BEGIN
//
#define EEPROM_POS_WLAN_PASSPHRASE  (EEPROM_POS_WLAN_SSID + EEPROM_MAXLEN_WLAN_SSID + EEPROM_LEADING_LENGTH)
//
#define EEPROM_POS_SERVER_IP        (EEPROM_POS_WLAN_PASSPHRASE + EEPROM_MAXLEN_WLAN_PASSPHRASE + EEPROM_LEADING_LENGTH)
//
#define EEPROM_POS_SERVER_PORT      (EEPROM_POS_SERVER_IP + EEPROM_MAXLEN_SERVER_IP + EEPROM_LEADING_LENGTH)
//
#define EEPROM_POS_NODENAME         (EEPROM_POS_SERVER_PORT + EEPROM_MAXLEN_SERVER_PORT + EEPROM_LEADING_LENGTH)
//
#define EEPROM_POS_ADMIN_PASSWORD   (EEPROM_POS_NODENAME + EEPROM_MAXLEN_NODENAME + EEPROM_LEADING_LENGTH)
//
#define EEPROM_STD_DATA_END         (EEPROM_POS_ADMIN_PASSWORD + EEPROM_MAXLEN_ADMIN_PASSWORD + EEPROM_LEADING_LENGTH)
//
#define EEPROM_EXT_DATA_BEGIN       EEPROM_STD_DATA_END
//
// ----- the above region is reserved for standard values
//
// EEPROM status byte may be a combination of the following values
//
// ----- non failure/error indicator
//
#define EE_STATUS_OK_AND_READY   0
#define EE_STATUS_MODIFIED       1
#define EE_STATUS_COMMITED       2
//
// ----- failure/error indicators
//
#define EE_STATUS_INVALID_CRC    4
#define EE_STATUS_INVALID_MAGIC  8
#define EE_STATUS_INVALID_SIZE  16

// macro to check whether log output is done
//
#define DOLOG            (logLevel > LOGLEVEL_QUIET)

class dsEeprom {

  private:
    short logLevel;
    short status;
    unsigned char magic;
    int blockSize;
    unsigned int reSized;
    unsigned long crc32Old;
    unsigned long crc32New;

  public:
    dsEeprom( unsigned int blockSize = 0, unsigned char magic = 0x00, int logLevel = LOGLEVEL_QUIET );
    int init( unsigned int blockSize = 0, unsigned char magic = 0x00, int logLevel = LOGLEVEL_QUIET );
    virtual ~dsEeprom();
    short getStatus( void );
    void setBlocksize( unsigned int newSize );
    unsigned int getBlocksize( void );
    void setMagic( short newMagic );
    unsigned char getMagic( void );
    void setLoglevel( short newValue );
    short getLoglevel( void );
    unsigned char version2Magic( void );
    unsigned long crc( int startPos, int length );
    void wipe( void );
    int storeFieldLength( char* len, int dataIndex );
    int restoreFieldLength( char* len, int dataIndex );
    int storeBoolean(  char* data, int dataIndex );
    int restoreBoolean( char *data, int dataIndex );
    int storeRaw( const char* data, short len, int dataIndex );
    int restoreRaw( char* data, int dataIndex, int len, int maxLen);
    int storeBytes( const char* data, short len, int dataIndex );
    int restoreBytes( String& data, int dataIndex, int len, int maxLen);
    int storeString( String data, int maxLen, int dataIndex );
    int restoreString( String& data, int dataIndex, int maxLen );
    bool isValid();
    bool validate();
};



#endif // _DSEEPROM_H_
