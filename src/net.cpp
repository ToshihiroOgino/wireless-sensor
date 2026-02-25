#include "net.h"

#include "sntp_opts.h"

#include "lwip/apps/mdns.h"
#include "lwip/apps/sntp.h"
#include "lwip/dns.h"
#include "lwip/ip.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>


#define DNS_1 "1.1.1.1"
#define DNS_2 "1.0.0.1"

int connect_wifi(const char *ssid, const char *password) {
	cyw43_arch_enable_sta_mode();
	if (cyw43_arch_wifi_connect_timeout_ms(
	        ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000) != PICO_ERROR_NONE) {
		return 1;
	}

	auto current_ip = ip4addr_ntoa(netif_ip4_addr(netif_list));
	printf("Connected to WiFi, My IP: %s\n", current_ip);
	return 0;
}

int init_mdns(const char *hostname) {
#if LWIP_MDNS_RESPONDER
	cyw43_arch_lwip_begin();
	mdns_resp_init();
	mdns_resp_add_netif(&cyw43_state.netif[CYW43_ITF_STA], hostname);
	mdns_resp_add_service(&cyw43_state.netif[CYW43_ITF_STA], "sensor", "_http",
	                      DNSSD_PROTO_TCP, 80, nullptr, 0);
	cyw43_arch_lwip_end();
	printf("mdns host name %s.local\n", hostname);
	return 0;
#else
	printf("mDNS responder not enabled\n");
	return 1;
#endif
}

ip_addr_t *resolve(const char *hostname) {
	ip_addr_t *addr = new ip_addr_t;
	err_t err = dns_gethostbyname(hostname, addr, nullptr, nullptr);
	if (err == ERR_OK) {
		return addr;
	} else if (err == ERR_INPROGRESS) {
		// DNSクエリが進行中の場合は、完了するまで待機
		while (dns_gethostbyname(hostname, addr, nullptr, nullptr) ==
		       ERR_INPROGRESS) {
			sleep_ms(100);
		}
		return addr;
	} else {
		delete addr;
		return nullptr;
	}
}

void init_sntp_client() {
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, SNTP_SERVER_0);
	sntp_setservername(1, SNTP_SERVER_1);
	sntp_setservername(2, SNTP_SERVER_2);
	sntp_init();

	const int max_retries = 15;
	for (int i = 0; i < max_retries && (sntp_getreachability(0) == 0 ); i++) {
		printf("Waiting for SNTP server to become reachable(%d/%d)...\n", i + 1, max_retries);
		sleep_ms(1000);
	}

	auto addr = sntp_getserver(0);
	printf("NTP server address resolved: %s=%s\n", SNTP_SERVER_0, ipaddr_ntoa(addr));
}
