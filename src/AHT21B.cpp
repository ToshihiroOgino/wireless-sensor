#include "AHT21B.hpp"
#include <cstdio>

bool write(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len,
           bool nostop) {
	const int ret = i2c_write_blocking(i2c, addr, src, len, nostop);
	if (ret < PICO_OK) {
		printf("I2C write error: %d\n", ret);
		return false;
	}
	return true;
}

bool read(i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len,
          bool nostop) {
	const int ret = i2c_read_blocking(i2c, addr, dst, len, nostop);
	if (ret < PICO_OK) {
		printf("I2C read error: %d\n", ret);
		return false;
	}
	return true;
}

AHT21B::AHT21B(i2c_inst_t *i2c_inst) : i2c(i2c_inst) {}

void AHT21B::begin(uint sda_pin, uint scl_pin) {
	i2c_init(i2c, 100 * 1000); // 100kHz Standard Mode
	gpio_set_function(sda_pin, GPIO_FUNC_I2C);
	gpio_set_function(scl_pin, GPIO_FUNC_I2C);
	gpio_pull_up(sda_pin);
	gpio_pull_up(scl_pin);

	sleep_ms(500); // 上電後の安定待ち

	// 状態チェックと初期化
	if ((read_status() & 0x18) != 0x18) {
		reset_registers();
		sleep_ms(10);
	}
	return;
}

uint8_t AHT21B::read_status() {
	constexpr uint8_t reg = 0x71;
	uint8_t status = 0xFF;
	if (!write(i2c, ADDR, &reg, 1, true)) {
		printf("Failed to write get status command\n");
		return 0xFF;
	}
	if (!read(i2c, ADDR, &status, 1, false)) {
		printf("Failed to read status\n");
		return 0xFF;
	}
	return status;
}

void AHT21B::reset_registers() {
	// 0x1B, 0x1C, 0x1E の初期化
	constexpr uint8_t regs[] = {0x1B, 0x1C, 0x1E};
	for (uint8_t r : regs) {
		const uint8_t cmd[] = {r, 0x00, 0x00};
		if (!write(i2c, ADDR, cmd, 3, false)) {
			printf("Failed to reset register %02X\n", r);
			return;
		}
		sleep_ms(5);
	}
}

int AHT21B::read_data(AHT21B_result_t *result) {
	if (result == nullptr) {
		return -1;
	}

	constexpr uint8_t trigger[] = {0xAC, 0x33, 0x00};
	if (!write(i2c, ADDR, trigger, 3, false)) {
		printf("Failed to trigger measurement\n");
		return -1;
	}

	sleep_ms(80); // 測定待ち

	// 忙しい状態を確認
	while (read_status() & 0x80) {
		sleep_ms(10);
	}

	uint8_t data[7]; // status + 5 bytes data + CRC
	if (!read(i2c, ADDR, data, 7, false)) {
		printf("Failed to read measurement data\n");
		return -1;
	}

	const uint32_t raw_humidity =
	    ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
	const uint32_t raw_temperature =
	    ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

	result->humidity = (float)raw_humidity / 1048576.0f * 100.0f;
	result->temperature = (float)raw_temperature / 1048576.0f * 200.0f - 50.0f;

	return 0;
}
