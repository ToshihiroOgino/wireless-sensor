#ifndef NET_H
#define NET_H

int connect_wifi(const char *ssid, const char *password);
int init_mdns(const char *hostname);

void init_sntp_client();

#endif // NET_H
