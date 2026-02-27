#include "hardware/watchdog.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "http.h"
#include "net.h"
#include "sensor.hpp"

#define BTN_PIN 4

// #define SSID "TTHocLieuT2"
// #define PASSWORD "hoclieut2"
#define SSID "Pixel_6686"
#define PASSWORD "4ebc7jt4"

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

	if (cyw43_arch_init_with_country(CYW43_COUNTRY_WORLDWIDE)) {
		printf("Wi-Fi init failed\n");
		return -1;
	}

	// Set the button pin as input with pull-up resistor
	gpio_init(BTN_PIN);
	gpio_set_dir(BTN_PIN, GPIO_IN);
	gpio_pull_up(BTN_PIN);

	gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_FALL, true,
	                                   &btn_pressed_callback);

	if (connect_wifi(SSID, PASSWORD) != 0) {
		printf("Failed to connect to WiFi\n");
		return -1;
	}

	if (init_mdns("pico") != 0) {
		printf("Failed to initialize mDNS\n");
		return -1;
	}

	Sensor sensor = Sensor();
	singleton_sensor = &sensor;

	start_http_server(80);

	while (true) {
		sensor.read_sensor_data();
		sleep_ms(5 * 1000);
	}
}
