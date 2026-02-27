#ifndef SNTP_OPTS_H
#define SNTP_OPTS_H

#include <time.h>
#include "hardware/rtc.h"
#include "pico/util/datetime.h"

#define SNTP_SUPPORT_MULTIPLE_SERVERS 1
#define SNTP_DNS 1
#define SNTP_SERVER_0 "time.cloudflare.com"
#define SNTP_SERVER_1 "time.google.com"
#define SNTP_SERVER_2 "ntp.nict.jp"

#define SNTP_DEBUG 1

void sntp_sync_rtc(uint32_t sec) {
    time_t raw_time = (time_t)sec;
    struct tm *t = gmtime(&raw_time);
    datetime_t dt = {
        .year  = (int16_t)(t->tm_year + 1900),
        .month = (int8_t)(t->tm_mon + 1),
        .day   = (int8_t)t->tm_mday,
        .dotw  = (int8_t)t->tm_wday, // 0=Sunday
        .hour  = (int8_t)t->tm_hour,
        .min   = (int8_t)t->tm_min,
        .sec   = (int8_t)t->tm_sec
    };
    rtc_set_datetime(&dt);
}

#define SNTP_SET_SYSTEM_TIME(sec) sntp_sync_rtc(sec)

#endif // SNTP_OPTS_H
