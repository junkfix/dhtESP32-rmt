// https://github.com/htmltiger/dhtESP32-rmt

#include <Arduino.h>
#include "dhtESP32-rmt.h"
#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define MAX_BLOCKS	64
#else
#define MAX_BLOCKS	48
#endif

uint8_t read_dht(float &temperature, float &humidity, uint8_t pin, uint8_t dhttype) {
	gpio_num_t dhtpin = static_cast<gpio_num_t>(pin);
	rmt_channel_handle_t rx_channel = NULL;
	rmt_symbol_word_t symbols[MAX_BLOCKS];
	rmt_rx_done_event_data_t rx_data;
	
	rmt_receive_config_t rx_config = {
		.signal_range_min_ns = 3000,
		.signal_range_max_ns = 150000,
	};
	
	rmt_rx_channel_config_t rx_ch_conf = {
		.gpio_num = dhtpin,
		.clk_src = RMT_CLK_SRC_DEFAULT,
		.resolution_hz = 1000000,
		.mem_block_symbols = MAX_BLOCKS,
	};
	uint8_t error = DHT_OK;
	if(rmt_new_rx_channel(&rx_ch_conf, &rx_channel) != ESP_OK) {
		return DHT_DRIVER;
	}
	QueueHandle_t rx_queue = xQueueCreate(1, sizeof(rx_data));
	assert(rx_queue);
	rmt_rx_event_callbacks_t cbs = {
		.on_recv_done = dhtrx_done,
	};

	rmt_rx_register_event_callbacks(rx_channel, &cbs, rx_queue);

	gpio_set_level(dhtpin, 1);
	gpio_pullup_dis(dhtpin);
	gpio_pulldown_dis(dhtpin);
	gpio_set_direction(dhtpin, GPIO_MODE_INPUT_OUTPUT_OD);
	gpio_set_intr_type(dhtpin, GPIO_INTR_DISABLE);
	gpio_set_level(dhtpin, 0);
	vTaskDelay((dhttype == DHT22 ? 2 : 22) / portTICK_PERIOD_MS);
	gpio_set_level(dhtpin, 1);
	
	
	if(rmt_enable(rx_channel) || rmt_receive(rx_channel, symbols, sizeof(symbols), &rx_config) != ESP_OK ) {
		return DHT_DRIVER;
	}

	if (xQueueReceive(rx_queue, &rx_data, pdMS_TO_TICKS(100)) == pdPASS) {
		
		char buf[128];
		size_t len = rx_data.num_symbols;

		uint32_t code = 0;
		rmt_symbol_word_t *cur = rx_data.received_symbols;
		uint8_t pulse = cur[0].duration0 + cur[0].duration1;
		if(len < 41){
			error = DHT_UNDERFLOW;
		}else if(len > 42){
			error = DHT_OVERFLOW;
		}else if((pulse) < 130 || (pulse) > 180){
			error = DHT_NACK;
		}else{
			uint8_t data[6];
			for(uint8_t i = 0; i < 40; i++){
				pulse = cur[i+1].duration0 + cur[i+1].duration1;
				if(pulse > 55 && pulse < 145){
					data[i / 8] <<= 1;
					if(pulse > 110){
						data[i / 8] |= 1;
					}
				}else{
					error = DHT_BAD_DATA;
				}
			}
			if(!error){
				uint8_t total = data[0] + data[1] + data[2] + data[3];
				if(data[4] == total){
					if(dhttype == DHT22){
						humidity = (data[0] * 256 + data[1]) * 0.1;
						temperature = ((data[2] & 0x7f) * 256 + data[3]) * 0.1;
						if(data[2] & 0x80){temperature = -temperature;}
					}else{
						humidity = data[0];
						temperature = data[2];
					}
				}else{
					error = DHT_CHECKSUM;
				}
			}
		}
	} else {
		error = DHT_TIMEOUT;
	}

	gpio_set_level(dhtpin, 1);
	vQueueDelete(rx_queue);
	rmt_disable(rx_channel);
	rmt_del_channel(rx_channel);
	return error;
}

bool IRAM_ATTR dhtrx_done(rmt_channel_handle_t ch, const rmt_rx_done_event_data_t *edata, void *udata){
	BaseType_t w = pdFALSE;
	QueueHandle_t d = (QueueHandle_t)udata;
	xQueueSendFromISR(d, edata, &w);
	return w == pdTRUE;
}
