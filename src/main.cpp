#include "AHT21B.hpp"
#include "hardware/watchdog.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define BTN_PIN 4

void btn_pressed_callback(uint gpio, uint32_t events) {
	if (gpio == BTN_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
		printf("Rebooting...\n\n\n");
		cyw43_arch_deinit();
		watchdog_enable(1, 1);
		while (true) {
			// wait for watchdog reset
		}
	}
}

int main() {
	stdio_init_all();

	// Initialise the Wi-Fi chip
	if (cyw43_arch_init()) {
		printf("Wi-Fi init failed\n");
		return -1;
	}

	// Set the button pin as input with pull-up resistor
	gpio_init(BTN_PIN);
	gpio_set_dir(BTN_PIN, GPIO_IN);
	gpio_pull_up(BTN_PIN);

	gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_FALL, true,
	                                   &btn_pressed_callback);

	AHT21B sensor(i2c0);
	if (sensor.begin()) {
		printf("AHT21B Initialized.\n");
	}

	while (true) {
		float h, t;
		if (sensor.read_data(h, t)) {
			printf("Humidity: %.2f %%RH, Temperature: %.2f degC\n", h, t);
		} else {
			printf("Sensor read failed\n");
		}
		sleep_ms(10 * 1000);
	}
}
