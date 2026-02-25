#include "AHT21B.hpp"
#include "hardware/watchdog.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "http.h"
#include "net.h"

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

void check_time(void) {
	time_t now;
	struct tm timeinfo;
	char str_time[64];

	// time()でシステム時刻を取得
	cyw43_arch_poll();
	time(&now);

	// 1970年などの初期値でないか確認（同期完了の判定）
	if (now > 1000000000L) {
		// 日本標準時(JST)にする場合は9時間加算
		now += (9 * 3600);

		localtime_r(&now, &timeinfo);
		strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", &timeinfo);
		printf("Current time: %s\n", str_time);
	} else {
		printf("Time not synchronized yet...\n");
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

	if (init_mdns("sensor") != 0) {
		printf("Failed to initialize mDNS\n");
		return -1;
	}

	init_sntp_client();

	// start_server(80);
	start_http_server(80);

	AHT21B sensor(i2c0);
	if (sensor.begin()) {
		printf("AHT21B Initialized.\n");
	}

	while (true) {
		check_time();

		float h, t;
		if (sensor.read_data(h, t)) {
			printf("Humidity: %.2f %%RH, Temperature: %.2f degC\n", h, t);
		} else {
			printf("Sensor read failed\n");
		}
		sleep_ms(10 * 1000);
	}
}
