#include "sensor.hpp"

#include "hardware/timer.h"
#include "pico/stdlib.h"
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

bool repeating_timer_callback(struct repeating_timer *t) {
	if (t == nullptr || t->user_data == nullptr) {
		printf("Invalid timer callback data(t=%p, user_data=%p)\n", t,
		       t ? t->user_data : nullptr);
		return false;
	}
	Sensor *sensor = (Sensor *)t->user_data;
	sensor->read_sensor_data();
	return true;
}

Sensor::Sensor() : aht21B(i2c0) {
	aht21B.begin();
	mutex_init(&data_mutex);

	add_repeating_timer_ms(5000, repeating_timer_callback, this, &timer);
}

data_t Sensor::get_recent_data() {
	data_t data_copy;
	mutex_enter_blocking(&data_mutex);
	data_copy = recent_data;
	mutex_exit(&data_mutex);
	return data_copy;
}
