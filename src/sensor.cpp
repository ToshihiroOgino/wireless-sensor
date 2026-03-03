#include "sensor.hpp"

#include <algorithm>
#include <stdio.h>

Sensor *singleton_sensor = nullptr;

void Sensor::read_sensor_data() {
	AHT21B_result_t result;
	if (aht21B.read_data(&result) == 0) {
		data_t data;
		data.timestamp_ms = to_ms_since_boot(get_absolute_time());
		data.humidity = result.humidity;
		data.temperature = result.temperature;

		mutex_enter_blocking(&data_mutex);
		recent_data = data;
		history[history_head] = data;
		history_head = (history_head + 1) % HISTORY_CAPACITY;
		if (history_count < HISTORY_CAPACITY) {
			history_count++;
		}
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
	history_head = 0;
	history_count = 0;
	recent_data = {0, 0.0f, 0.0f};
}

data_t Sensor::get_recent_data() {
	data_t data_copy;
	mutex_enter_blocking(&data_mutex);
	data_copy = recent_data;
	mutex_exit(&data_mutex);
	return data_copy;
}

size_t Sensor::get_history(data_t *buffer, size_t max_count) {
	if (!buffer || max_count == 0) {
		return 0;
	}

	mutex_enter_blocking(&data_mutex);
	const size_t count = std::min(max_count, history_count);
	// history_head always points to the next write position (one past the newest entry).
	// To retrieve the most recent 'count' items, we move 'count' steps back from history_head,
	// adding HISTORY_CAPACITY before subtracting to avoid underflow, then wrap with modulo.
	const size_t start = (history_head + HISTORY_CAPACITY - count) % HISTORY_CAPACITY;
	for (size_t i = 0; i < count; ++i) {
		buffer[i] = history[(start + i) % HISTORY_CAPACITY];
	}
	mutex_exit(&data_mutex);

	return count;
}

size_t Sensor::get_history_capacity() const {
	return HISTORY_CAPACITY;
}
