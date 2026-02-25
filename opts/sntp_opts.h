#ifndef SNTP_OPTS_H
#define SNTP_OPTS_H

// #include <sys/time.h>
// #define SNTP_GET_SYSTEM_TIME_NTP(sec, us) do { \
//     struct timeval tv = { .tv_sec = (time_t)(sec), .tv_usec = (us) }; \
//     settimeofday(&tv, NULL); \
// } while(0)

#define SNTP_COMP_ROUNDTRIP 1
#define SNTP_DNS 1
#define SNTP_SERVER_0 "time.cloudflare.com"
#define SNTP_SERVER_1 "time.google.com"
#define SNTP_SERVER_2 "ntp.nict.jp"

#endif // SNTP_OPTS_H
