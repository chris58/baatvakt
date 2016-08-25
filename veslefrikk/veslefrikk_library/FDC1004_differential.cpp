/***********************************************************************
FDC1004_differential Library

This library provides functions for using TI's FDC1004 Capacitance to Digital Sensor in differential measurement

Written by Andreas Østensen
NTNU, Norway

************************************************************************/

#include "FDC1004_differential.h"

uint8_t MEAS_CONFIG[] = { 0x08, 0x09, 0x0A, 0x0B };
uint8_t MEAS_MSB[] = { 0x00, 0x02, 0x04, 0x06 };
uint8_t MEAS_LSB[] = { 0x01, 0x03, 0x05, 0x07 };
uint8_t SAMPLE_DELAY[] = { 11, 11, 6, 3 };

FDC1004::FDC1004(uint8_t rate) {
	this->_rate = rate;
}

//Read function
uint16_t FDC1004::read16(uint8_t reg) {
	Wire.beginTransmission(FDC1004_ADDRESS);
	Wire.write(reg);
	Wire.endTransmission();
	uint16_t value;
	Wire.beginTransmission(FDC1004_ADDRESS);
	Wire.requestFrom(FDC1004_ADDRESS, (uint8_t)2);
	value = Wire.read();
	value <<= 8;
	value |= Wire.read();
	Wire.endTransmission();
	return value;
}

//Write function
void FDC1004::write16(uint8_t reg, uint16_t data) {
	Wire.beginTransmission(FDC1004_ADDRESS);
	Wire.write(reg); //send address
	Wire.write((uint8_t)(data >> 8));
	Wire.write((uint8_t)data);
	Wire.endTransmission();
}

uint8_t FDC1004::configureMeasurement(uint8_t measurement, uint8_t channel_1, uint8_t channel_2) {
	//Verify data
	if (!FDC1004_IS_MEAS(measurement) || !FDC1004_IS_CHANNEL(channel_1) || !FDC1004_IS_CHANNEL(channel_2) || (channel_1 == channel_2)) {
		Serial.println("bad measurement configuration");
		return 1;
	}

	//build 16 bit configuration
	uint16_t configuration_data;
	configuration_data = ((uint16_t)channel_1) << 13; //CHA
	configuration_data |= ((uint16_t)channel_2) << 10; //CHB
	write16(MEAS_CONFIG[measurement], configuration_data);
	return 0;
}

uint8_t FDC1004::triggerSingleMeasurement(uint8_t measurement) {
	//verify data
	if (!FDC1004_IS_MEAS(measurement) || !FDC1004_IS_RATE(_rate)) {
		Serial.println("bad trigger request");
		return 1;
	}
	uint16_t trigger_data;
	trigger_data = ((uint16_t)_rate) << 10; // sample rate
	trigger_data |= 0 << 8; //repeat disabled
	trigger_data |= (1 << (7 - measurement)); // 0 > bit 7, 1 > bit 6, etc
	write16(FDC_REGISTER, trigger_data);
	return 0;
}

uint8_t FDC1004::readMeasurement(uint8_t measurement, uint16_t value[]) {
	if (!FDC1004_IS_MEAS(measurement)) {
		Serial.println("bad read request");
		return 1;
	}

	//check if measurement is complete
	uint16_t fdc_register = read16(FDC_REGISTER);
	if (!(fdc_register & (1 << (3 - measurement)))) {
		Serial.println("measurement not completed");
		return 2;
	}

	//read the value
	uint16_t msb = read16(MEAS_MSB[measurement]);
	uint16_t lsb = read16(MEAS_LSB[measurement]);
	value[0] = msb;
	value[1] = lsb;
	return 0;
}

uint8_t FDC1004::measureChannel(uint8_t measurement, uint8_t channel_1, uint8_t channel_2, uint16_t * value) {
	if (configureMeasurement(measurement, channel_1, channel_2)) return 1;
	if (triggerSingleMeasurement(measurement)) return 1;
	delay(SAMPLE_DELAY[_rate]);
	return readMeasurement(measurement, value);
}

double FDC1004::getCapacitance(uint8_t measurement, uint8_t channel_1, uint8_t channel_2) {
	int32_t value;
	uint8_t result = getRawCapacitance(measurement, channel_1, channel_2, &value);
	if (result) return 0x80000000;

	return (double)value / PICOFARAD_CONVERSION_CONSTANT; //picofarads
}

uint8_t FDC1004::getRawCapacitance(uint8_t measurement, uint8_t channel_1, uint8_t channel_2, int32_t * value) {
	if (!FDC1004_IS_CHANNEL(channel_1) || !FDC1004_IS_CHANNEL(channel_1) || (channel_1 == channel_2)) return 1;
	uint16_t raw_value[2];

	if (measureChannel(measurement, channel_1, channel_2, raw_value)) {
		Serial.println("error");
		return 1;
	}

	*value = ((uint32_t)raw_value[1] >> 8) | ((uint32_t)raw_value[0] << 8);

	return 0;
}

double FDC1004::measureLevel(double prefactor) {
	/*
	MEAS1 = CIN1(CHA) - CIN4(CHB) //Level sensor
	MEAS2 = CIN2(CHA) - CIN4(CHB) //Reference Environment sensor
	MEAS3 = CIN3(CHA) - CIN4(CHB) //Reference Level sensor

	level = h_RL * (C_level - C_level(0)) / (C_RL - C_RE)
	*/

	double c_level = getCapacitance(FDC1004_MEAS1, FDC1004_CIN1, FDC1004_CIN4);
	double c_rl = getCapacitance(FDC1004_MEAS2, FDC1004_CIN2, FDC1004_CIN4);
	double c_re = getCapacitance(FDC1004_MEAS3, FDC1004_CIN3, FDC1004_CIN4);

	return prefactor * ((c_level - _base_cap) / (c_rl - c_re) * -1);
}

void FDC1004::setBaseCapacitance() {
	_base_cap = getCapacitance(FDC1004_MEAS1, FDC1004_CIN1, FDC1004_CIN4);
}