#include "http.h"

#include "lwip/apps/fs.h"
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

static const char *ssi_tags[] = {"state"};

static constexpr size_t HTTP_HISTORY_RESPONSE_POINTS = 300;
static constexpr size_t HTTP_HISTORY_JSON_BUFFER_SIZE = 12288;
static char history_json[HTTP_HISTORY_JSON_BUFFER_SIZE];

static int build_history_json(char *out, size_t out_size) {
	if (!out || out_size < 32 || !singleton_sensor) {
		return 0;
	}

	data_t history[HTTP_HISTORY_RESPONSE_POINTS];
	const size_t count =
	    singleton_sensor->get_history(history, HTTP_HISTORY_RESPONSE_POINTS);

	int written = snprintf(
	    out, out_size,
	    "HTTP/1.0 200 OK\r\n"
	    "Content-Type: application/json\r\n"
	    "Cache-Control: no-store\r\n"
	    "\r\n"
	    "{\"points\":[");
	if (written < 0 || (size_t)written >= out_size) {
		return 0;
	}

	size_t pos = (size_t)written;
	for (size_t i = 0; i < count; ++i) {
		written = snprintf(out + pos, out_size - pos, "%s[%llu,%.2f,%.2f]",
		                   (i == 0) ? "" : ",",
		                   (unsigned long long)history[i].timestamp_ms,
		                   (double)history[i].temperature,
		                   (double)history[i].humidity);
		if (written < 0 || (size_t)written >= out_size - pos) {
			return 0;
		}
		pos += (size_t)written;
	}

	written = snprintf(out + pos, out_size - pos, "]}");
	if (written < 0 || (size_t)written >= out_size - pos) {
		return 0;
	}

	pos += (size_t)written;
	return (int)pos;
}

int fs_open_custom(struct fs_file *file, const char *name) {
	if (!file || !name || strcmp(name, "/api/history.json") != 0) {
		return 0;
	}

	const int len = build_history_json(history_json, sizeof(history_json));
	if (len <= 0) {
		return 0;
	}

	file->data = history_json;
	file->len = (int)len;
	file->index = file->len;
	file->flags = FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_CUSTOM;
	return 1;
}

void fs_close_custom(struct fs_file *file) {
	(void)file;
}

uint16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
	printf("SSI handler called for tag index %d\n", iIndex);
	uint16_t written = 0;
	switch (iIndex) {
	case 0:
		written =
		    snprintf(pcInsert, iInsertLen, "%s",
		             cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN) ? "ON" : "OFF");
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
