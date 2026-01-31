#ifndef AHT21B_HPP
#define AHT21B_HPP

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#define SDA_PIN 16
#define SCL_PIN 17

class AHT21B {
public:
	AHT21B(i2c_inst_t *i2c_inst = i2c0);
	bool begin(uint sda_pin = SDA_PIN, uint scl_pin = SCL_PIN);
	bool read_data(float &humidity, float &temperature);

private:
	static constexpr uint8_t ADDR = 0x38;
	i2c_inst_t *i2c;
	uint8_t read_status();
	void reset_registers();
	uint8_t calculate_crc8(uint8_t *data, uint8_t len);
};

#endif
