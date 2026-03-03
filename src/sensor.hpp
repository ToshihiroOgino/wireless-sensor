#ifndef SENSOR_H
#define SENSOR_H

#include "AHT21B.hpp"
#include <stddef.h>
#include <stdint.h>
#include <pico/mutex.h>

typedef struct {
	uint64_t timestamp_ms;
	float humidity;
	float temperature;
} data_t;

class Sensor {
public:
	Sensor();
	void read_sensor_data();
	data_t get_recent_data();
	size_t get_history(data_t *buffer, size_t max_count);
	size_t get_history_capacity() const;

private:
	static constexpr size_t HISTORY_CAPACITY = 512;
	AHT21B aht21B;
	mutex_t data_mutex;
	data_t recent_data;
	data_t history[HISTORY_CAPACITY];
	size_t history_head;
	size_t history_count;
	struct repeating_timer timer;
};

extern Sensor *singleton_sensor;

#endif // SENSOR_H
