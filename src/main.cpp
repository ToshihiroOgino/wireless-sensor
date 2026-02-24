#include "AHT21B.hpp"
#include "hardware/watchdog.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "net.h"

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

err_t server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
	if (err != ERR_OK || client_pcb == NULL) {
		printf("server_accept: error %d\n", err);
		return ERR_VAL;
	}
	printf("accepted client connection\n");

	return ERR_OK;
}

void start_server(const int port) {
	auto pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		printf("Failed to create TCP PCB\n");
		return;
	}
	if (tcp_bind(pcb, nullptr, port) != ERR_OK) {
		printf("Failed to bind TCP PCB\n");
		tcp_close(pcb);
		return;
	}

	auto server_pcb = tcp_listen_with_backlog(pcb, 3);
	if (!server_pcb) {
		if (pcb) {
			tcp_close(pcb);
		}
		printf("Failed to listen on TCP PCB\n");
		return;
	}

	tcp_arg(server_pcb, nullptr);
	tcp_accept(server_pcb, server_accept);
}

int main() {
	stdio_init_all();

	// Initialise the Wi-Fi chip
	if (cyw43_arch_init_with_country(CYW43_COUNTRY_WORLDWIDE)) {
		printf("Wi-Fi init failed\n");
		return -1;
	}

	if (connect_wifi("Pixel_6686", "4ebc7jt4", "10.78.156.155") != 0) {
		printf("Failed to connect to WiFi\n");
		return -1;
	}

	if (init_mdns("sensor") != 0) {
		printf("Failed to initialize mDNS\n");
		return -1;
	}

	start_server(80);

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
