#include "sensor.hpp"

#include <stdio.h>

Sensor *singleton_sensor = nullptr;

void Sensor::read_sensor_data() {
	AHT21B_result_t result;
	if (aht21B.read_data(&result) == 0) {
		mutex_enter_blocking(&data_mutex);
		recent_data.timestamp = to_ms_since_boot(get_absolute_time());
		recent_data.humidity = result.humidity;
		recent_data.temperature = result.temperature;
		mutex_exit(&data_mutex);
		printf("Sensor data updated: Humidity=%.2f%%, Temperature=%.2f°C\n",
		       result.humidity, result.temperature);
	} else {
		printf("Failed to read data from AHT21B sensor\n");
	}
}

Sensor::Sensor() : aht21B(i2c0) {
	aht21B.begin();
	mutex_init(&data_mutex);
}

data_t Sensor::get_recent_data() {
	data_t data_copy;
	mutex_enter_blocking(&data_mutex);
	data_copy = recent_data;
	mutex_exit(&data_mutex);
	return data_copy;
}
