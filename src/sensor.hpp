#ifndef SENSOR_H
#define SENSOR_H

#include "AHT21B.hpp"
#include <pico/mutex.h>
#include <queue>
#include <time.h>

typedef struct {
	time_t timestamp;
	float humidity;
	float temperature;
} data_t;

class Sensor {
public:
	Sensor();
	void read_sensor_data();
	data_t get_recent_data();

private:
	AHT21B aht21B;
	mutex_t data_mutex;
	data_t recent_data;
	struct repeating_timer timer;
};

extern Sensor *singleton_sensor;

#endif // SENSOR_H
