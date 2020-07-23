/**************************************************************************/
/*!
    @file     Adafruit_ADS1015.cpp
    @author   K.Townsend (Adafruit Industries)
    @license  BSD (see license.txt)

    Driver for the ADS1015/ADS1115 ADC

    This is a library for the Adafruit MPL115A2 breakout
    ----> https://www.adafruit.com/products/???

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "Adafruit_ADS1015.h"

#include <Wire.h>

#include <SoftWire.h>
SoftWire Wire2( 0, 0 );

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino SoftWire library
*/
/**************************************************************************/
static uint8_t i2cread( void ) {
  #if ARDUINO >= 100
  return Wire2.read();
  #else
  return Wire2.receive();
  #endif
}

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino SoftWire library
*/
/**************************************************************************/
static void i2cwrite( uint8_t x ) {
  #if ARDUINO >= 100
  Wire2.write((uint8_t)x);
  #else
  Wire2.send(x);
  #endif
}

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino Wire library
*/
/**************************************************************************/
static uint8_t i2c_HW_read( void ) {
  #if ARDUINO >= 100
  return Wire.read();
  #else
  return Wire.receive();
  #endif
}

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino Wire library
*/
/**************************************************************************/
static void i2c_HW_write( uint8_t x ) {
  #if ARDUINO >= 100
  Wire.write((uint8_t)x);
  #else
  Wire.send(x);
  #endif
}

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1015 class w/appropriate properties
*/
/**************************************************************************/
Adafruit_ADS1015::Adafruit_ADS1015( const uint8_t i2cAddress, const uint8_t pinSDA, const uint8_t pinSCL ) :
  m_i2cAddress( i2cAddress ),
  pin_SDA( pinSDA ),
  pin_SCL( pinSCL )
{
   m_conversionDelay = ADS1015_CONVERSIONDELAY;
   m_bitShift = 4;
   m_gain = GAIN_TWOTHIRDS; /* +/- 6.144V range (limited to VDD +0.3V max!) */

   if ( ( pin_SDA == 0 ) && ( pin_SCL == 0 ) ) {
    Wire.begin();
    Wire.setClock( 400000 );
   }
   else {
    Wire2.setSda( pin_SDA );
    Wire2.setScl( pin_SCL );
    Wire2.setDelay_us( 5 );
    Wire2.setRxBuffer( I2C_buffer, sizeof( I2C_buffer ) );
    Wire2.setTxBuffer( I2C_buffer, sizeof( I2C_buffer ) );
    Wire2.setClock( 400000 );
    Wire2.begin();
   }
}

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1115 class w/appropriate properties
*/
/**************************************************************************/
Adafruit_ADS1115::Adafruit_ADS1115( const uint8_t i2cAddress, const uint8_t pinSDA, const uint8_t pinSCL )
{
   m_i2cAddress = i2cAddress;
   m_conversionDelay = ADS1015_CONVERSIONDELAY;
   m_bitShift = 4;
   m_gain = GAIN_TWOTHIRDS; /* +/- 6.144V range (limited to VDD +0.3V max!) */
   pin_SDA = pinSDA;
   pin_SCL = pinSCL;

   if ( ( pin_SDA == 0 ) && ( pin_SCL == 0 ) ) {
    Wire.begin();
    Wire.setClock( 400000 );
   }
   else {
    Wire2.setSda( pin_SDA );
    Wire2.setScl( pin_SCL );
    Wire2.setDelay_us( 5 );
    Wire2.setRxBuffer( I2C_buffer, sizeof( I2C_buffer ) );
    Wire2.setTxBuffer( I2C_buffer, sizeof( I2C_buffer ) );
    Wire2.setClock( 400000 );
    Wire2.begin();
   }
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
void Adafruit_ADS1015::writeRegister( uint8_t i2cAddress, uint8_t reg, uint16_t value ) {
  if ( ( pin_SDA == 0 ) && ( pin_SCL == 0 ) ) {
    Wire.beginTransmission(i2cAddress);
    i2c_HW_write((uint8_t)reg);
    i2c_HW_write((uint8_t)(value>>8));
    i2c_HW_write((uint8_t)(value & 0xFF));
    Wire.endTransmission();
  }
  else {
    Wire2.beginTransmission(i2cAddress);
    i2cwrite((uint8_t)reg);
    i2cwrite((uint8_t)(value>>8));
    i2cwrite((uint8_t)(value & 0xFF));
    Wire2.endTransmission();
  }
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
uint16_t Adafruit_ADS1015::readRegister( uint8_t i2cAddress, uint8_t reg ) {
  if ( ( pin_SDA == 0 ) && ( pin_SCL == 0 ) ) {
    Wire.beginTransmission(i2cAddress);
    i2c_HW_write(ADS1015_REG_POINTER_CONVERT);
    Wire.endTransmission();
    Wire.requestFrom( i2cAddress, (uint8_t)2 );
    return ((i2c_HW_read() << 8) | i2c_HW_read());
  }
  else {
    Wire2.beginTransmission(i2cAddress);
    i2cwrite(ADS1015_REG_POINTER_CONVERT);
    Wire2.endTransmission();
    Wire2.requestFrom( i2cAddress, (uint8_t)2 );
    return ((i2cread() << 8) | i2cread());
  }
}

/**************************************************************************/
/*!
    @brief  Gets a single-ended ADC reading from the specified channel
*/
/**************************************************************************/
uint16_t Adafruit_ADS1015::readADC_SingleEnded( uint8_t channel ) {
  if ( channel > 3 ) {
    return 0;
  }

  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_DR_1600SPS   | // 1600 samples per second
                    ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set single-ended input channel
  switch ( channel )
  {
    case (0):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
      break;
    case (1):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
      break;
    case (2):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
      break;
    case (3):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
      break;
  }

  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister( m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config );

  // Wait for the conversion to complete
  delay( m_conversionDelay );

  // Read the conversion results
  // Shift 12-bit results right 4 bits for the ADS1015
  return readRegister( m_i2cAddress, ADS1015_REG_POINTER_CONVERT ) >> m_bitShift;
}

/**************************************************************************/
/*!
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN0) and N (AIN1) input.  Generates
            a signed value since the difference can be either
            positive or negative.
*/
/**************************************************************************/
int16_t Adafruit_ADS1015::readADC_Differential_0_1( void ) {
  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set channels
  config |= ADS1015_REG_CONFIG_MUX_DIFF_0_1;          // AIN0 = P, AIN1 = N

  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister( m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config );

  // Wait for the conversion to complete
  delay( m_conversionDelay );

  // Read the conversion results
  uint16_t res = readRegister( m_i2cAddress, ADS1015_REG_POINTER_CONVERT ) >> m_bitShift;
  if ( m_bitShift == 0 ) {
    return (int16_t)res;
  }
  else {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if (res > 0x07FF) {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}

/**************************************************************************/
/*!
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN2) and N (AIN3) input.  Generates
            a signed value since the difference can be either
            positive or negative.
*/
/**************************************************************************/
int16_t Adafruit_ADS1015::readADC_Differential_2_3( void ) {
  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set channels
  config |= ADS1015_REG_CONFIG_MUX_DIFF_2_3;          // AIN2 = P, AIN3 = N

  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister( m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config );

  // Wait for the conversion to complete
  delay( m_conversionDelay );

  // Read the conversion results
  uint16_t res = readRegister( m_i2cAddress, ADS1015_REG_POINTER_CONVERT ) >> m_bitShift;
  if ( m_bitShift == 0 ) {
    return (int16_t)res;
  }
  else {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if (res > 0x07FF) {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}

/**************************************************************************/
/*!
    @brief  Sets up the comparator to operate in basic mode, causing the
            ALERT/RDY pin to assert (go from high to low) when the ADC
            value exceeds the specified threshold.

            This will also set the ADC in continuous conversion mode.
*/
/**************************************************************************/
void Adafruit_ADS1015::startComparator_SingleEnded( uint8_t channel, int16_t threshold ) {
  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_1CONV   | // Comparator enabled and asserts on 1 match
                    ADS1015_REG_CONFIG_CLAT_LATCH   | // Latching mode
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1015_REG_CONFIG_MODE_CONTIN  | // Continuous conversion mode
                    ADS1015_REG_CONFIG_MODE_CONTIN;   // Continuous conversion mode

  // Set PGA/voltage range
  config |= m_gain;

  // Set single-ended input channel
  switch ( channel )
  {
    case (0):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
      break;
    case (1):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
      break;
    case (2):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
      break;
    case (3):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
      break;
  }

  // Set the high threshold register
  // Shift 12-bit results left 4 bits for the ADS1015
  writeRegister( m_i2cAddress, ADS1015_REG_POINTER_HITHRESH, threshold << m_bitShift );

  // Write config register to the ADC
  writeRegister( m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config );
}

/**************************************************************************/
/*!
    @brief  In order to clear the comparator, we need to read the
            conversion results.  This function reads the last conversion
            results without changing the config value.
*/
/**************************************************************************/
int16_t Adafruit_ADS1015::getLastConversionResults( void ) {
  // Wait for the conversion to complete
  delay( m_conversionDelay );

  // Read the conversion results
  uint16_t res = readRegister( m_i2cAddress, ADS1015_REG_POINTER_CONVERT ) >> m_bitShift;
  if ( m_bitShift == 0 ) {
    return (int16_t)res;
  }
  else {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if ( res > 0x07FF ) {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}

/**************************************************************************/
/*!
    @brief  Sets the gain and input voltage range
*/
/**************************************************************************/
void Adafruit_ADS1015::setGain( adsGain_t gain ) {
  m_gain = gain;
}

/**************************************************************************/
/*!
    @brief  Gets a gain and input voltage range
*/
/**************************************************************************/
adsGain_t Adafruit_ADS1015::getGain( void ) {
  return m_gain;
}

