// https://github.com/junkfix/dhtESP32-rmt
#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/rmt_rx.h"
#include "driver/gpio.h"

#define DHT11	0
#define DHT21	1
#define DHT22	1
#define AM2301	1
#define AM2302	1

#define DHT_OK			0
#define DHT_TOO_SOON	1
#define DHT_DRIVER		2
#define DHT_TIMEOUT		3
#define DHT_NACK		4
#define DHT_BAD_DATA	5
#define DHT_CHECKSUM	6
#define DHT_UNDERFLOW	7
#define DHT_OVERFLOW	8

uint8_t read_dht(float &temperature, float &humidity, uint8_t pin, uint8_t dhttype);

bool IRAM_ATTR dhtrx_done(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *udata);
