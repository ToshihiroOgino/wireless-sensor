#include "AHT21B.hpp"
#include "hardware/watchdog.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "net.h"
#include "http.h"

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

err_t server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
	if (err != ERR_OK || client_pcb == NULL) {
		printf("server_accept: error %d\n", err);
		return ERR_VAL;
	}
	printf("accepted client connection\n");

	return ERR_OK;
}

int main() {
	stdio_init_all();

	// Initialise the Wi-Fi chip
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

	if (init_mdns("sensor") != 0) {
		printf("Failed to initialize mDNS\n");
		return -1;
	}

	// start_server(80);
	start_http_server(80);

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
