https://ad7zj.net/kd7lmo/ground_nixie_clock_hardware.html
/** 
 * @mainpage Nixie Tube Clock
 *
 * @section overview_sec Overview
 *
 * Nixie Tube based real-time clock using GPS for the time base.
 *
 * @section history_sec Revision History
 *
 * @subsection v10 V1.1
 * <b>9 Aug 2008</b>, Software updates include the following; (A) 
 *
 *
 * @subsection v10 V1.0
 * <b>16 Jul 2006</b>, Initial release
 */

// Hardware specific configuration.
#include <18f252.h>

// External crystal oscillator.
#fuses HS

// Disable watch dog timer.
#fuses NOWDT

// Protect the code so it can't be read back.
#fuses NOPROTECT

// Disable the power-up enable timer.
#fuses NOPUT

// Disable the brown out detect.
#fuses NOBROWNOUT

// Disable the low voltage programming mode.
#fuses NOLVP

// We define types that are used for all variables.  These are declared
// because each processor has a different sizes for int and long.
// The PIC compiler defines boolean, int8, int16, and int32.

/// Boolean value { false, true }
typedef boolean bool_t;

/// Signed 8-bit number in the range -128 through 127.
typedef signed int8 int8_t;

/// Unsigned 8-bit number in the range 0 through 255.
typedef unsigned int8 uint8_t;

/// Signed 16-bit number in the range -32768 through 32767.
typedef signed int16 int16_t;

/// Unsigned 16-bit number in the range 0 through 65535.
typedef unsigned int16 uint16_t;

/// Signed 32-bit number in the range -2147483648 through 2147483647.
typedef signed int32 int32_t;

/// Unsigned 32-bit number in the range 0 through 4294967296.
typedef unsigned int32 uint32_t;

/// IEEE 32-bit floating point number.
typedef struct
{
    /// Array that holds 32-bit IEEE floating pointer number.
    unsigned int8 value[4];
} float_t;

/// IEEE 64-bit floating point number.
typedef struct
{
    /// Array that holds 64-bit IEEE floating point number.
    unsigned int8 value[8];
} double_t;

void nixieData (bool_t state);
void nixieDisplay (uint8_t hours, uint8_t minutes, uint8_t seconds, bool_t colon1, bool_t colon2);
void nixieHVSupply (bool_t state);
void nixieStrobeClock();
void nixieStrobeLE();

void gpsInit();
bool_t gpsIsReady();
void gpsUpdate();

bool_t serialHasData();
void serialInit();
uint8_t serialRead();
void serialUpdate();

void sysInit();
inline uint16_t sysUint16GPSToHost (uint16_t value);
inline uint32_t sysUint32GPSToHost (uint32_t value);

// RCSTA (RECEIVE STATUS AND CONTROL REGISTER) - CREN: Continuous Receive Enable bit
#bit CREN = 0xfab.4

// These compiler directives set the clock, SPI/I2C ports, and I/O configuration.

// Frequency of crystal
#use delay(clock=3686400)

// GPS engine 
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7)

#use rs232(baud=9600, xmit=PIN_B7, STREAM=ENG)

// We'll set the I/O direction so we can just read/write I/O pins.
#use fast_io(A)
#use fast_io(B)
#use fast_io(C)

/**
 *  @defgroup GPS GPS Engine
 *
 *  Functions to control the Trimble Resolution T GPS engine in TSIP binary mode.
 *  See the January 2005 (Revision A) of the Resolution T System Designer Reference 
 *  Manual part number 54655-05 for details on the Resolution T GPS engine.<br>
 *
 *  On start-up, the GPS engines send two binary messages after each 1-PPS event.
 *  The messages are the primary and supplemental timing packets.
 *
 *  @{
 */

/// The maximum length of a binary GPS engine message.
#define GPS_BUFFER_SIZE 80

/// No timing packets recevied.
#define GPS_UPDATE_NO_PACKET 0x00

/// Primary packet (0x8f - 0xab) processed.
#define GPS_UPDATE_PRIMARY_PACKET 0x01

/// Supplemental packet (0x8f - 0xac) processed.
#define GPS_UPDATE_SUPP_PACKET 0x02

/// All packets received.
#define GPS_UPDATE_ALL_PACKETS 0x03

/// GPS time information from primary and supplemental packets.
typedef struct 
{
    /// Month in GPS time.
    uint8_t month;

    /// Day of month in GPS time.
    uint8_t day;

    /// Year in GPS time.
    uint16_t year;

    /// Hours in GPS time.
    uint8_t hours;

    /// Minutes in GPS time.
    uint8_t minutes;

    /// Seconds in GPS time.
    uint8_t seconds;

    /// UTC offset in seconds.
    uint16_t utcOffset;

    /// GPS decoding status.  See reference manual for table.
    uint8_t decodingStatus;

    /// GPS seconds of week.
    uint32_t timeOfWeek;

    /// GPS Week Number.
    uint16_t weekNumber;
} GPS_TIME_STRUCT;

/// GPS parse engine state machine values.
enum GPS_PARSE_STATE_MACHINE 
{
    /// Start of message delimiter.
    GPS_DLE,

    /// Message data bytes.
    GPS_DATA,

    /// Extra DLE within message.
    GPS_EXTRA_DLE
};

/// Resolution-T message packet format for Primary Timing Packet (0x8f-ab).
typedef struct
{
    /// TSIP Super-packet identifier, always 0x8f for this message.
    uint8_t messageID;

    /// Always 0xab for this message.
    uint8_t subcode;

    /// GPS seconds of week.
    uint32_t timeOfWeek;

    /// GPS Week Number.
    uint16_t weekNumber;

    /// UTC Offset (seconds).
    uint16_t utcOffset;

    /// Timing flag bit field.  Reference manual for bit usage.
    uint8_t timingFlag;

    /// Time in seconds.
    uint8_t seconds;

    /// Time in minutes.
    uint8_t minutes;

    /// Time in hours.
    uint8_t hours;

    /// Day of month.
    uint8_t day;

    /// Month of year.
    uint8_t month;

    /// Four digits of year.
    uint16_t year;
} GPS_PRIMARY_PACKET;

/// Resolution-T message packet format for Supplemental Timing Packet (0x8f-ac).
typedef struct
{
    /// TSIP Super-packet identifier, always 0x8f for this message.
    uint8_t messageID;

    /// Always 0xac for this message.
    uint8_t subcode;

    /// Reeiver mode.  Reference manual for table.
    uint8_t receiverMode;

    /// Reserved.
    uint8_t reserved1;

    /// Self survey progress in the range 0 to 100%.
    uint8_t selfSurveyProgress;

    /// Reserved.
    uint32_t reserved2;

    /// Reserved.
    uint16_t reserved3;

    /// Minor alarms bit field.  Reference manual for bit usage.
    uint16_t minorAlarms;

    /// GPS decoding status.  Reference manual for table.
    uint8_t decodingStatus;

    /// Reserved.
    uint8_t reserved4;

    /// Spare status 1.
    uint8_t spareStatus1;

    /// Spare status 2.
    uint8_t spareStatus2;

    /// Local clock bias.
    float_t localClockBias;

    /// Local clock bias rate.
    float_t localClockBiasRate;

    /// Reserved.
    uint32_t reserved5;

    /// Reserved.
    float_t reserved6;

    /// Temperature in degrees C.
    float_t temperature;

    /// Latitude in radians.
    double_t latitude;

    /// Longitude in radians.
    double_t longitude;

    /// Altitude in meters.
    double_t altitude;

    /// PPS Quantization error in seconds.
    float_t ppsQuantizationError;

    /// Spare / future expansion.
    uint8_t spare[4];
} GPS_SUPPLEMENTAL_PACKET;

/// Local time and date calcualted from GPS time, UTC/GPS offset, and time zone.
typedef struct 
{
    /// seconds after the minute - [0,59]
    uint8_t tm_sec;

    /// minutes after the hour - [0,59]
    uint8_t tm_min;

    /// hours since midnight - [0,23]
    uint8_t tm_hour;
   
    /// day of the month - [1,31]
    uint8_t tm_mday;

    /// months since January - [0,11]
    uint8_t tm_mon;

    /// years since 1900
    uint16_t tm_year;

    /// days since Sunday - [0,6]
    uint8_t tm_wday;

    /// days since January 1 - [0,365]
    uint16_t tm_yday;
} LOCAL_TIME;


/// Index into gpsBuffer used to store message data.
uint8_t gpsIndex;

/// State machine used to parse the GPS message stream.
GPS_PARSE_STATE_MACHINE gpsParseState;

/// Buffer to store data as it is read from the GPS engine.
uint8_t gpsBuffer[GPS_BUFFER_SIZE]; 

/// Last complete timing report.
GPS_TIME_STRUCT gpsTime;

/// Bitmap used to track when each of the timing reports is received.
uint8_t gpsUpdateBitmap;

/** 
 *    Initialize the GPS subsystem.
 */
void gpsInit()
{
    // Initial parse state.
    gpsParseState = GPS_DLE;

    // Initial bitmap state.
    gpsUpdateBitmap = GPS_UPDATE_NO_PACKET;

    // Clear the structure that stores the position message.
    memset (&gpsTime, 0x00, sizeof(gpsTime));
}

/**
 *   Determine if new GPS message is ready to process.  This function is a one shot and
 *   typically returns true once a second for each GPS position fix.
 *
 *   @return true if new message available; otherwise false
 */
bool_t gpsIsReady()
{
    if (gpsUpdateBitmap == GPS_UPDATE_ALL_PACKETS) 
    {
        gpsUpdateBitmap = GPS_UPDATE_NO_PACKET;
        return true;
    } // END if

    return false;
}

/**
 *   Process the primary timing packet.
 *
 *   @param packet pointer to GPS_PRIMARY_PACKET structure
 */
void gpsProcessPrimaryPacket (GPS_PRIMARY_PACKET *packet)
{
    gpsTime.month = packet->month;
    gpsTime.day = packet->day;
    gpsTime.year = sysUint16GPSToHost(packet->year);

    gpsTime.hours = packet->hours;
    gpsTime.minutes = packet->minutes;
    gpsTime.seconds = packet->seconds;

    gpsTime.utcOffset = sysUInt16GPSToHost(packet->utcOffset);

    gpsTime.timeOfWeek = sysUint32GPSToHost(packet->timeOfWeek);

    gpsTime.weekNumber = sysUInt16GPSToHost(packet->weekNumber);

    // Update the bitmap that indicates we processed this message.
    gpsUpdateBitmap |= GPS_UPDATE_PRIMARY_PACKET;
}

/**
 *  Process the supplemental timing packet.
 *
 *  @param packet pointer to GPS_SUPPLEMENTAL_PACKET structure
 */
void gpsProcessSupplementalPacket (GPS_SUPPLEMENTAL_PACKET *packet)
{
    gpsTime.decodingStatus = packet->decodingStatus;

    // Update the bitmap that indicates we processed this message.
    gpsUpdateBitmap |= GPS_UPDATE_SUPP_PACKET;
}

/**
 *   Determine the type of GPS message and parse it.
 */
void gpsParseMessage()
{
    // Byte 0 is the messageID.
    switch (gpsBuffer[0])
    {
        case 0x45:
            return;

        case 0x8f:
            // Byte 1 identifies the super packet type.
            switch (gpsBuffer[1])
            {
                case 0xab:
                    gpsProcessPrimaryPacket (gpsBuffer);
                    return;            

                case 0xac:
                    gpsProcessSupplementalPacket (gpsBuffer);
                    return; 
            } // END switch
    } // END swich
}

/**
 *   Read the serial FIFO and process complete GPS messages.
 */
void gpsUpdate() 
{
    uint8_t value;

    // This state machine handles each characters as it is read from the GPS serial port.
    // This parses the Resolution T TSIP binary message protocol.
    while (serialHasData()) 
    {
        // Get the character value.
        value = serialRead();

        // Process based on the state machine.
        switch (gpsParseState) 
        {
            case GPS_DLE:
                // Wait for the message delimiter character.
                if (value == 0x10)
                {
                    gpsParseState = GPS_DATA;

                    gpsIndex = 0;
                } // END if
                break;

            case GPS_DATA:
                    // Save each value to the data buffer.
                    gpsBuffer[gpsIndex] = value;

                    // If we filled the buffer without detecting the <DLE><ETX> sequence start over.
                    if (++gpsIndex == GPS_BUFFER_SIZE)
                        gpsParseState = GPS_DLE;

                // If the DLE character is detected, it either means the end of message or a byte stuffed value.
                if (value == 0x10)
                    gpsParseState = GPS_EXTRA_DLE;

                break;

            case GPS_EXTRA_DLE:
                switch (value)
                {
                    // End of message character.
                    case 0x03:
                        gpsParseMessage();

                        gpsParseState = GPS_DLE;
                        break;

                    case 0x10:
                        gpsParseState = GPS_DATA;
                        break;

                    default:
                        gpsParseState = GPS_DLE;
                } // END switch


                break;

        } // END switch
    } // END while
}

// this array represents the number of days in one non-leap year at the beginning of each month.
static uint16_t DaysToMonth[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

/// Number of seconds between time_t epoch (1 Jan 1970) and GPS epoch (6 Jan 1980).  10 years and 7 days = 3657 day * 24 hours * 60 minutes * 60 seconds
#define EpochTimeDifference 315964800

/// Number of seconds per week; 7 days * 24 hours/day * 60 minutes/hour * 60 seconds/minute
#define SecondsPerWeek 604800

#define SecondsPerHour 3600

void gpsConvertTime(uint16_t gpsWeekNumber, uint32_t gpsTimeOfWeek, uint8_t utcOffset, int8_t timeZoneOffset, LOCAL_TIME *datetime) 
{
    uint32_t hour;
    uint32_t day;
    uint32_t minute;
    uint32_t second;
    uint32_t month;
    uint32_t year;
    uint32_t whole_minutes;
    uint32_t whole_hours;
    uint32_t whole_days;
    uint32_t whole_days_since_1968;
    uint32_t leap_year_periods;
    uint32_t days_since_current_lyear;
    uint32_t whole_years;
    uint32_t days_since_first_of_year;
    uint32_t days_to_month;
    uint32_t day_of_week;
    uint32_t binary;

    // Time base difference between time_t epoch (1 Jan 1970) and GPS epoch (6 Jan 1980).
    binary = gpsTimeOfWeek + EpochTimeDifference;
    
    // Add GPS Week Number offset.
    binary += ((uint32_t) gpsWeekNumber * SecondsPerWeek);
    
    // Offset between GPS and UTC time base.
    binary -= utcOffset;

    // Time zone offset.
    binary += (uint32_t) timeZoneOffset * SecondsPerHour;

    whole_minutes = binary / 60;
    second = binary - (60 * whole_minutes); // leftover seconds
    whole_hours = whole_minutes / 60;
    minute = whole_minutes - (60 * whole_hours); // leftover minutes
    whole_days = whole_hours / 24;
    hour = whole_hours - (24 * whole_days); // leftover hours
    whole_days_since_1968 = whole_days + 365 + 366;
    leap_year_periods = whole_days_since_1968 / ((4 * 365) + 1);
    days_since_current_lyear = whole_days_since_1968 % ((4 * 365) + 1);

    // if days are after a current leap year then add a leap year period
    if ((days_since_current_lyear >= (31 + 29)))
        leap_year_periods++;

    whole_years = (whole_days_since_1968 - leap_year_periods) / 365;

    days_since_first_of_year = whole_days_since_1968 - (whole_years * 365) - leap_year_periods;

    if ((days_since_current_lyear <= 365) && (days_since_current_lyear >= 60))
        days_since_first_of_year++;

    year = whole_years + 68;
    // setup for a search for what month it is based on how many days have past
    // within the current year

    month = 13;

    days_to_month = 366;

    while (days_since_first_of_year < days_to_month)
    {
        month--;

        days_to_month = DaysToMonth[month];

        if ((month >= 2) && ((year % 4) == 0))
            days_to_month++;
    }

    day = days_since_first_of_year - days_to_month + 1;

    day_of_week = (whole_days + 4) % 7;

    datetime->tm_yday = days_since_first_of_year; /* days since January 1 - [0,365] */
    datetime->tm_sec = second; /* seconds after the minute - [0,59] */
    datetime->tm_min = minute; /* minutes after the hour - [0,59] */
    datetime->tm_hour = hour; /* hours since midnight - [0,23] */
    datetime->tm_mday = day; /* day of the month - [1,31] */
    datetime->tm_wday = day_of_week; /* days since Sunday - [0,6] */
    datetime->tm_mon = month + 1; /* months since January - [0,11] */
    datetime->tm_year = year + 1900; /* years since 1900 */
}

/** @} */

/**
 *  @defgroup nixie Nixie Tube Driver
 *
 *  Drives the Nixie and Neon bulbs through the HV5522 high voltage shift register/driver.
 *
 *  @{
 */

// Map I/O pin names to hardware pins.

/// HV5522 clock - Port C0
#define NIXIE_CLK PIN_C0

/// HV5522 Latch Eable - Port C1
#define NIXIE_LE PIN_C1

/// HV5522 Data - Port C2
#define NIXIE_DATA PIN_C2

/// High Voltage Supply Enable - Port C3
#define NIXIE_HV PIN_C3

/// Total number of bits in the shift register chain (2 (two) HV5522 devices).
#define NIXIE_BIT_COUNT 64

#define HV1A 0
#define HV2A 1
#define HV3A 2
#define HV4A 3
#define HV5A 4
#define HV6A 5
#define HV7A 6
#define HV8A 7

#define HV9A 8
#define HV10A 9
#define HV11A 10
#define HV12A 11
#define HV13A 12
#define HV14A 13
#define HV15A 14
#define HV16A 15

#define HV17A 16
#define HV18A 17
#define HV19A 18
#define HV20A 19
#define HV21A 20
#define HV22A 21
#define HV23A 22
#define HV24A 23

#define HV25A 24
#define HV26A 25
#define HV27A 26
#define HV28A 27
#define HV29A 28
#define HV30A 29
#define HV31A 30
#define HV32A 31


#define HV1B 32
#define HV2B 33
#define HV3B 34
#define HV4B 35
#define HV5B 36
#define HV6B 37
#define HV7B 38
#define HV8B 39

#define HV9B 40
#define HV10B 41
#define HV11B 42
#define HV12B 43
#define HV13B 44
#define HV14B 45
#define HV15B 46
#define HV16B 47

#define HV17B 48
#define HV18B 49
#define HV19B 50
#define HV20B 51
#define HV21B 52
#define HV22B 53
#define HV23B 54
#define HV24B 55

#define HV25B 56
#define HV26B 57
#define HV27B 58
#define HV28B 59
#define HV29B 60
#define HV30B 61
#define HV31B 62
#define HV32B 63


const uint8_t NIXIE_HOURS_10[] = { HV4A, HV5A, HV6A, HV8A, HV2A, HV1A, HV10A, HV9A, HV7A, HV3A };
const uint8_t NIXIE_HOURS_1[] = { HV14A, HV15A, HV16A, HV18A, HV12A, HV11A, HV20A, HV19A, HV17A, HV13A };

const uint8_t NIXIE_MINUTES_10[] = { HV25A, HV26A, HV27A, HV29A, HV23A, HV22A, HV31A, HV30A, HV28A, HV24A };
const uint8_t NIXIE_MINUTES_1[] = { HV4B, HV5B, HV6B, HV8B, HV2B, HV1B, HV10B, HV9B, HV7B, HV3B };

const uint8_t NIXIE_SECONDS_10[] = { HV15B, HV16B, HV17B, HV19B, HV13B, HV12B, HV21B, HV20B, HV18B, HV14B };
const uint8_t NIXIE_SECONDS_1[] = { HV25B, HV26B, HV27B, HV29B, HV23B, HV22B, HV31B, HV30B, HV28B, HV24B };

#define NIXIE_COLON_1 HV21A
#define NIXIE_COLON_2 HV11B

#define NIXIE_BLANK_DIGIT 15

/// Number of micro-seconds to delay after setting clock, data, or latch enable line.
#define NIXIE_DRIVER_DELAY 5

/**
 *   Set the HV5522 data line.
 *
 *   @param state 
 */
void nixieData (bool_t state)
{
    output_bit (NIXIE_DATA, (state ? false : true));
    delay_us(NIXIE_DRIVER_DELAY);
}

/**
 *   Initialize the Nixie driver.
 */
void nixieInit()
{
    uint8_t i;

    // Turn off the HV supply.
    nixieHVSupply (false);

    // Set the latch and clock low.  Note the pins are INVERTED between the PIC and the HV5522.
    output_high (NIXIE_LE);
    output_high (NIXIE_CLK);

    // set all outputs low.
    nixieData (false);

    for (i = 0; i < NIXIE_BIT_COUNT; ++i)
        nixieStrobeClock();

    nixieStrobeLE();
}

/**
 *  Set the digits based on the time values.
 *
 *  @param hours time in the range 0 to 23
 *  @param minutes time in the range 0 to 59
 *  @param seconds time in the rnage 0 to 59
 *  @param colon1 true to enable colon between hours and minutes digits
 *  @param colon2 true to enable colon between minutes and seconds digits
 */
void nixieDisplay (uint8_t hours, uint8_t minutes, uint8_t seconds, bool_t colon1, bool_t colon2)
{
    uint8_t bitArray[64];
    uint8_t i;

    // Convert 00 hours to 12.
    if (hours == 0)
        hours = 12;

    // Clear the array of 64-bits that represents each HV5522 output pin.
    for (i = 0; i < NIXIE_BIT_COUNT; ++i)
        bitArray[i] = false;

    // Set the output driver bit that corresponds to the time.
    if ((hours / 10) != 0)
        bitArray[NIXIE_HOURS_10[hours / 10]] = true;

    if ((hours % 10) != NIXIE_BLANK_DIGIT)
        bitArray[NIXIE_HOURS_1[hours % 10]] = true;

    // Set the output driver bit that corresponds to the time.
    if ((minutes / 10) != NIXIE_BLANK_DIGIT)
        bitArray[NIXIE_MINUTES_10[minutes / 10]] = true;

    if ((minutes % 10) != NIXIE_BLANK_DIGIT)
        bitArray[NIXIE_MINUTES_1[minutes % 10]] = true;


    // Set the output driver bit that corresponds to the time.
    if ((seconds / 10) != NIXIE_BLANK_DIGIT)
        bitArray[NIXIE_SECONDS_10[seconds / 10]] = true;

    if ((seconds % 10) != NIXIE_BLANK_DIGIT)
        bitArray[NIXIE_SECONDS_1[seconds % 10]] = true;


    // Set the output driver bit that coresponds to the desired colon.
    if (colon1)
        bitArray[NIXIE_COLON_1] = true;

    if (colon2)
        bitArray[NIXIE_COLON_2] = true;

    // Write the 64-bit stream in reverse order.
    for (i = 0; i < NIXIE_BIT_COUNT; ++i)
    {
        nixieData (bitArray[NIXIE_BIT_COUNT - i - 1]);
        nixieStrobeClock();
    } // END for
    
    // Latch the new data set.
    nixieStrobeLE();
}

/**
 *  Control the Nixie display high voltage supply.
 *
 *  @param state true to enable HV supply; otherwise false
 */
void nixieHVSupply (bool_t state)
{
    output_bit (NIXIE_HV, state);
}

/**
 *   Strobe the HV5522 clock line.
 */
void nixieStrobeClock()
{
    output_low (NIXIE_CLK);
    delay_us(NIXIE_DRIVER_DELAY);

    output_high (NIXIE_CLK);
    delay_us(NIXIE_DRIVER_DELAY);
}

/**
 *  Strobe the HV5522 latch enable line.
 */
void nixieStrobeLE()
{
    output_low (NIXIE_LE);
    delay_us(NIXIE_DRIVER_DELAY);

    output_high (NIXIE_LE);
    delay_us(NIXIE_DRIVER_DELAY);
}

/** @} */


/**
 *  @defgroup serial Serial Port FIFO
 *
 *  FIFO for the built-in serial port.
 *
 *  @{
 */

/// Size of serial input port FIFO in bytes.  Must be a power of 2, i.e. 2, 4, 8, 16, etc.
#define SERIAL_BUFFER_SIZE 128

/// Mask to wrap around at end of circular buffer.  (SERIAL_IN_BUFFER_SIZE - 1)
#define SERIAL_BUFFER_MASK 0x7f

/// Index to the next free location in the buffer.
uint8_t serialHead;

/// Index to the next oldest data in the buffer.
uint8_t serialTail;

/// Circular buffer (FIFO) to hold serial data.
uint8_t serialBuffer[SERIAL_BUFFER_SIZE];

/**
 *   Determine if the FIFO contains data.
 *
 *   @return true if data present; otherwise false
 */
bool_t serialHasData()
{
    if (serialHead == serialTail)
        return false;

    return true;
}

/** 
 *   Initialize the serial FIFO.
 */
void serialInit()
{
    serialHead = 0;
    serialTail = 0;
}

/**
 *   Get the oldest character from the FIFO.
 *
 *   @return oldest character; 0 if FIFO is empty
 */
uint8_t serialRead()
{
    uint8_t value;

    // Make sure we have something to return.
    if (serialHead == serialTail)
        return 0;

    // Save the value.
    value = serialBuffer[serialTail];

    // Update the pointer.
    serialTail = (serialTail + 1) & SERIAL_BUFFER_MASK;

    return value;
}

/**
 *   Read and store any characters in the PIC serial port in a FIFO.
 */
#INT_RDA
void serialISR()
{
    // Save the value in the FIFO.
    serialBuffer[serialHead] = getc();

    // Move the pointer to the next open space.
    serialHead = (serialHead + 1) & SERIAL_BUFFER_MASK;
}

/** @} */

/**
 *  @defgroup sys System Library Functions
 *
 *  Generic system functions similiar to the C run-time library.
 *
 *  @{
 */

/**
 *  Convert a 16-bit value's endian order.
 *
 *  @param value to convert
 *
 *  @return reserve endian value
 */
inline uint16_t sysUint16GPSToHost (uint16_t value)
{
    return ((value >> 8) & 0x00ff) | ((value << 8) & 0xff00);
}

/**
 *  Convert a 32-bit value's endian order.
 *
 *  @param value to convert
 *
 *  @return reserve endian value
 */
inline uint32_t sysUint32GPSToHost (uint32_t value)
{
    return ((value >> 24) & 0x0000ff) | ((value >> 8) & 0x0000ff00) | ((value << 8) & 0x00ff0000) | ((value << 24) & 0xff000000);
}

/**
 *   Initialize the system library and global resources.
 */
void sysInit()
{
    // Set all outputs low.
    output_a (0x00);
    output_b (0x00);
    output_c (0x00);
    
    // Configure the port direction (input/output).
    set_tris_a (0x00);
    set_tris_b (0x41);
    set_tris_c (0xb0);

    // Disable all ADC ports.
    setup_adc_ports (NO_ANALOGS);

    port_b_pullups (true);
}

/** @} */

/**
 *  @defgroup app Device application
 *
 *  Provides the main functionality for the device.
 *
 *  @{
 */

/**
 *   Initialize the application specific variables and required processor resources.
 */
void appInit()
{
    // Read the hardware UART port until it is empty.
    while (kbhit())
        getch();

    // Clear any detected hardware UART errors.
    CREN = false;
    CREN = true;

    // Setup the interrupts.
    enable_interrupts(INT_RDA);
    enable_interrupts(GLOBAL);

    // Turn on the HV supply and wait for it to stabilize.
    nixieHVSupply (true);
    delay_ms(100);

    // Set all 8s on the display
    nixieDisplay (88, 88, 88, true, true);
}

/**
 *   Enable and display the display based on the time of day and day of week.
 */
bool_t appIsDisplayActive (LOCAL_TIME *localTime)
{
    if (localTime->tm_hour <= 5)
        return true;

    switch (localTime->tm_wday)
    {
        case 0:
        case 6:
            if (localTime->tm_hour >= 13)
                return true;

            break;

        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            if (localTime->tm_hour == 13 || localTime->tm_hour == 14)
                return true;

            break;
    } // END switch
            
    return false;
}

/**
 *    User application.
 */
void appRun()
{
    bool_t colonFlag, displayActiveFlag;
    uint16_t activeCounter;
    LOCAL_TIME localTime;

    activeCounter = 0;

    // This is the main loop that process GPS data and waits for the once per second timer tick.
    for (;;) 
    {
        if (!input(PIN_C5))
            if (activeCounter == 0)
                activeCounter = 600;

        // Read the GPS engine serial port FIFO and process the GPS data.
        gpsUpdate();

        // Process the GPS data set when we get a complete set.
        if (gpsIsReady())
        {
            gpsConvertTime (gpsTime.weekNumber, gpsTime.timeOfWeek, gpsTime.utcOffset, -7, &localTime);


            fprintf (ENG, "%02d:%02d:%02d ", localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
            fprintf (ENG, "%d/%d/%ld  %d\n\r", localTime.tm_mon, localTime.tm_mday, localTime.tm_year, localTime.tm_wday);

            displayActiveFlag = appIsDisplayActive (&localTime);

            if (activeCounter != 0)
            {
                displayActiveFlag = true;
                --activeCounter;
            }

            nixieHVSupply (displayActiveFlag);

            // Strobe the status LED based on the time of day.
            output_bit (PIN_A0, (localTime.tm_sec & 0x01 ? true : false));

            // When the decode status is 0x00, it indicates the GPS is tracking, otherwise we'll flash the colon.
            colonFlag = false;

            if (gpsTime.decodingStatus == 0x00)
                colonFlag = true;
            else
                if (gpsTime.seconds & 0x01)
                    colonFlag |= true;  

            nixieDisplay (localTime.tm_hour, localTime.tm_min, localTime.tm_sec, true, colonFlag);
        } // END if

    } // END for
}

/** @} */

/// Function where everything gets started.
void main()
{
    // Configure the basic system.
    sysInit();

    // Wait for the power converter chain to stabilize.
    delay_ms (20);

    // Setup the subsystems.
    gpsInit();
    serialInit();
    nixieInit();
    appInit();

    // Run the end application.
    appRun();
}