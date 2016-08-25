// FDC1004_differential.h
/************************
This is a library for differential measurement with the FDC1004 Capacitive Sensor.
Created for use with the Capacitive-Based Liquid Level Sensing Sensor Reference Design:
http://www.ti.com/lit/pdf/tidu736

Written by Andreas Østensen
Student at NTNU, Norway
************************/

#ifndef _FDC1004_DIFFERENTIAL_h
#define _FDC1004_DIFFERENTIAL_h

	#include "Arduino.h"
	#include "Wire.h"

	//Constants and limits for FDC1004
	#define FDC1004_ADDRESS (uint8_t)0b1010000

	#define FDC1004_100HZ (0x01)
	#define FDC1004_200HZ (0x02)
	#define FDC1004_400HZ (0x03)
	#define FDC1004_IS_RATE(x) (x == FDC1004_100HZ || x == FDC1004_200HZ || x == FDC1004_400HZ)

	#define FDC1004_CHANNEL_MAX (0x03)
	#define FDC1004_IS_CHANNEL(x) (x >= 0 && x <= FDC1004_CHANNEL_MAX)
	#define FDC1004_CIN1 0
	#define FDC1004_CIN2 1
	#define FDC1004_CIN3 2
	#define FDC1004_CIN4 3


	#define FDC1004_MEAS_MAX (0x03)
	#define FDC1004_IS_MEAS(x) (x >= 0 && x <= FDC1004_MEAS_MAX)
	#define FDC1004_MEAS1 0
	#define FDC1004_MEAS2 1
	#define FDC1004_MEAS3 2
	#define FDC1004_MEAS4 3

	#define PICOFARAD_CONVERSION_CONSTANT (524288) // 2^19

	#define FDC_REGISTER (0x0C)

	/******************************************************************************************
	* Function Declarations
	*******************************************************************************************/
	class FDC1004 {
	public:
		FDC1004(uint8_t rate = FDC1004_100HZ);
		uint16_t read16(uint8_t reg);
		void write16(uint8_t reg, uint16_t data);
		uint8_t configureMeasurement(uint8_t measurement, uint8_t channel_1, uint8_t channel_2);
		uint8_t triggerSingleMeasurement(uint8_t measurement);
		uint8_t readMeasurement(uint8_t measurement, uint16_t value[]);
		uint8_t measureChannel(uint8_t measurement, uint8_t channel_1, uint8_t channel_2, uint16_t * value);
		double getCapacitance(uint8_t measurement, uint8_t channel_1, uint8_t channel_2);
		uint8_t getRawCapacitance(uint8_t measurement, uint8_t channel_1, uint8_t channel_2, int32_t * value);
		double measureLevel(double prefactor);
		void setBaseCapacitance();

	private:
		double value;
		double _base_cap;
		uint8_t _rate;
	};


#endif

