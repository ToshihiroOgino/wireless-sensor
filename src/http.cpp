#include "http.h"

#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <string.h>

#include "sensor.hpp"

// 1. CGIハンドラ関数の定義
const char *cgi_handler_led(int iIndex, int iNumParams, char *pcParam[],
                            char *pcValue[]) {
	printf("Received CGI request with %d parameters\n", iNumParams);
	// パラメータ "status" が "on" かどうかをチェック
	for (int i = 0; i < iNumParams; i++) {
		if (strcmp(pcParam[i], "status") == 0) {
			if (strcmp(pcValue[i], "on") == 0) {
				cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
			} else if (strcmp(pcValue[i], "off") == 0) {
				cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
			}
		}
	}
	// 処理後に表示するHTMLファイル名を返す
	return "/index.shtml";
}

// 2. ハンドラ構造体の配列（URIと関数の紐付け）
static const tCGI cgi_handlers[] = {
    // {"/led.cgi", cgi_handler_led},
    {"/led", cgi_handler_led},
};

static const char *ssi_tags[] = {"state", "temp", "humidity"};

uint16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
	printf("SSI handler called for tag index %d\n", iIndex);
	uint16_t written = 0;
	data_t data;
	switch (iIndex) {
	case 0:
		written =
		    snprintf(pcInsert, iInsertLen, "%s",
		             cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN) ? "ON" : "OFF");
		break;
	case 1:
		data = singleton_sensor->get_recent_data();
		written = snprintf(pcInsert, iInsertLen, "%.2f", data.temperature);
		break;
	case 2:
		data = singleton_sensor->get_recent_data();
		written = snprintf(pcInsert, iInsertLen, "%.2f", data.humidity);
		break;
	default:
		return HTTPD_SSI_TAG_UNKNOWN;
	}
	return written;
}

void start_http_server(const int port) {
	http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
	http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
	httpd_init();
}
