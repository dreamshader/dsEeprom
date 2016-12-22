The idea for the dsEeprom-Library was to make my life easier. I did always the same job in several small projects on ESP-modules or for an Arduino: read/write EEPROM content.
To simplify this, I created this library. Because it is designed for my needs, it may or my not be useful for you.

General:
To install the library, you may download the zip-file or clone the whole ESP8266 repository.
If you are interested only in this library, you can get the subfolder using svn:

svn checkout https://github.com/dreamshader/ESP8266/trunk/sketchbook/libraries/dsEeprom








dsEeprom( unsigned int blockSize = 0, unsigned char magic = 0x00, int logLevel = LOGLEVEL_QUIET )
int init( unsigned int blockSize = 0, unsigned char magic = 0x00, int logLevel = LOGLEVEL_QUIET )
virtual ~dsEeprom()
short getStatus( void )
void setBlocksize( unsigned int newSize )
unsigned int getBlocksize( void )
void setMagic( short newMagic )
unsigned char getMagic( void )
void setLoglevel( short newValue )
short getLoglevel( void )
unsigned char version2Magic( void )
unsigned long crc( int startPos, int length )
void wipe( void )
int storeFieldLength( char* len, int dataIndex )
int restoreFieldLength( char* len, int dataIndex )
int storeBoolean(  char* data, int dataIndex )
int restoreBoolean( char *data, int dataIndex )
int storeRaw( const char* data, short len, int dataIndex )
int restoreRaw( char* data, int dataIndex, int len, int maxLen)
int storeBytes( const char* data, short len, int dataIndex )
int restoreBytes( String& data, int dataIndex, int len, int maxLen)
int storeString( String data, int maxLen, int dataIndex )
int restoreString( String& data, int dataIndex, int maxLen )
bool isValid()
bool validate()



























Jetzt gibt es eine Besonderheit der EEPROM-Library: die Positionen und Längen einige System-Variablen sind bereits vordefiniert. Ihr müsst diese Vorgaben nicht nutzen, ich für meinen Teil fand das Feature einfach praktisch.
Allerdings decken sich die vordefinierten System-Variablen nicht mit denen, die wir oben festgelegt haben.  In der dsEeprom.h sind die Positionen und Löngen für folgende Systemvariablen vordefiniert:
[code]
wlanSSID
wlanPassphrase
wwwServerIP
wwwServerPort
nodeName
adminPasswd
[/code]

In der Headerdatei dsEeprom.h findet ihr diese vordefinierten Längen- und Positions-Angaben. Sie sollten ausreichend gross dimensioniert sein, aber ihr könnt sie ggf. anpassen:
[code]
#define EEPROM_MAXLEN_WLAN_SSID          32  // max. length a SSID may have
#define EEPROM_MAXLEN_WLAN_PASSPHRASE    64  // max. length of a WLAN passphrase
#define EEPROM_MAXLEN_SERVER_IP          19  // max. length for the server IP
#define EEPROM_MAXLEN_SERVER_PORT         4  // max. length for the server port
#define EEPROM_MAXLEN_NODENAME           32  // max. lenght of the (generated) nodename
#define EEPROM_MAXLEN_ADMIN_PASSWORD     32  // max. length for admin password
[/code]

Eine weitere Besonderheit der EEPROM-Library ist, dass die gespeicherten Daten einmal mit einem sog. "magic byte" gekennzeichnet und zudem mit einer CRC32 Prüfsumme versehen sind. Das "magic byte" hat derzeit nur den Zweck festzustellen, ob das EEPROM von der dsEeprom-Library beschrieben wurde.
In der Library gint es bereits eine vordefinierte Funktion 
[code]
    unsigned char version2Magic()
[/code]

Die Idee dahinter ist, dass aufgrund des magic byte auf die Library-Version geschlossen werden kann. Dadurch können z.B. im Falle eines uodate der Library "alte" Daten mit den "alten" Funktionen ausgelesen und diese dann mit den "neuen" Funktionen in ein "neues" Format gebracht werden.
Neben dem "magic byte" sind in der Header-Datei einige weitere Konstanen definiert:
[code]
...
#define EEPROM_MAGIC_BYTE              0x7e
//
#define EEPROM_LEADING_LENGTH             2  // means two byte representing 
//                                           // the real length of the data field
#define EEPROM_MAXLEN_MAGIC               1  // one byte only
#define EEPROM_MAXLEN_CRC32               4  // uint32_t = 4 byte
//
#define EEPROM_MAXLEN_BOOLEAN             1  // true/false = 1 byte
//
...
[/code]

Das vordefinierte Daten-Handling ist wie folgt: ausser dem "magic byte", das an allererster Stelle steht, wird jedes Datenfeld mit seiner tatsächlichen Länge abgelegt. Das vorangestellte Längenfeld hat eine Größe von 2 Byte. Für jedes Datenfeld gibt es zudem eine definierte Maximal-Länge. Im EEPROM wird immer diese Maximal-Länge belegt. Also Achtung! Nicht zu grosszügig sein. Da diese Lib auch die Arduinos unterstützt, bewegen sich die EEPROM-Größen zwischen 512 Byte und 2 kByte. Der ESP ist eine Ausnahme mit 4kByte EEPROM.
Die dsEeprom-Library legt die Daten demnach nach folgendem Schema im EEPROM ab (als Beispiel dient der String "Hello world!", die max. Länge für dieses Feld soll 25 sein):

[code]
Pos.  0 -  1 | magic byte
Pos.  1 -  4 | Länge des CRC32 (4 byte)
Pos.  5 -  8 | CRC32 über den EEPROM-Inhalt
Pos.  9 - 12 | Länge des Beipiel-String (0x000c)
Pos. 13 - 24 | Hello World!
Pos. 25 - 37 | überspringen bis max. Länge
Pos. 38 - 41 | Länge des nächsten Datenfelds
...

[/code]

Dadurch wird die maximale Feldlänge zwar auf 255 Zeichen begrenzt, jedoch sollte das für die meisten Fälle ausreichend sein. 
Das vordefinierte Layout des EEPROM sieht also folgendermassen festgelegt:
[code]
...
//
//
// predefined standard layout of the eeprom:
//
#define EEPROM_HEADER_BEGIN         0
#define EEPROM_POS_MAGIC            0
//
#define EEPROM_POS_CRC32            (EEPROM_POS_MAGIC + EEPROM_MAXLEN_MAGIC)
#define EEPROM_HEADER_END           (EEPROM_POS_CRC32 + EEPROM_MAXLEN_CRC32 + EEPROM_LEADING_LENGTH)
//
// data area begins here
//
#define EEPROM_STD_DATA_BEGIN       EEPROM_HEADER_END       
#define EEPROM_POS_WLAN_SSID        EEPROM_STD_DATA_BEGIN
//
#define EEPROM_POS_WLAN_PASSPHRASE  (EEPROM_POS_WLAN_SSID + EEPROM_MAXLEN_WLAN_SSID + EEPROM_LEADING_LENGTH)
#define EEPROM_POS_SERVER_IP        (EEPROM_POS_WLAN_PASSPHRASE + EEPROM_MAXLEN_WLAN_PASSPHRASE + EEPROM_LEADING_LENGTH)
#define EEPROM_POS_SERVER_PORT      (EEPROM_POS_SERVER_IP + EEPROM_MAXLEN_SERVER_IP + EEPROM_LEADING_LENGTH)
#define EEPROM_POS_NODENAME         (EEPROM_POS_SERVER_PORT + EEPROM_MAXLEN_SERVER_PORT + EEPROM_LEADING_LENGTH)
#define EEPROM_POS_ADMIN_PASSWORD   (EEPROM_POS_NODENAME + EEPROM_MAXLEN_NODENAME + EEPROM_LEADING_LENGTH)
#define EEPROM_STD_DATA_END         (EEPROM_POS_ADMIN_PASSWORD + EEPROM_MAXLEN_ADMIN_PASSWORD + EEPROM_LEADING_LENGTH)
//
#define EEPROM_EXT_DATA_BEGIN       EEPROM_STD_DATA_END
//
...
[/code]

Da ich die Library so wie sie ist verwende, ist die für mich erste, relevante Datenposition:
[code]
#define EEPROM_EXT_DATA_BEGIN       EEPROM_STD_DATA_END
[/code]

Ab hier definiere ich nun die Anordnung der fehlenden Systemdaten:





