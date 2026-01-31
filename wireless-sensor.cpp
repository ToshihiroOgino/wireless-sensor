#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

// GPIO 16
#define BTN_PIN 4

void btn_pressed_callback(uint gpio, uint32_t events) {
	if (gpio == BTN_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
		printf("Button Pressed!\n");
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

	bool led_state = false;

	while (true) {
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
		led_state = !led_state;
		printf("Hello, world!\n");
		sleep_ms(1000);
	}
	return 0;
}
