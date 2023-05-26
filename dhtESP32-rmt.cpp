#include <Arduino.h>
#include "dhtESP32-rmt.h"

uint8_t read_dht(float &temperature, float &humidity, uint8_t pin, uint8_t dhttype, uint8_t rx){
	//check last read
	static uint8_t devices = 1;
	struct pin_grp {
		uint8_t id;
		unsigned long lastRead;
	};
	static pin_grp *pins = new pin_grp[1]{{pin, 0}};
	int8_t devid = -1;
	for(int i = 0; i < devices; i++){if(pins[i].id == pin){devid = i;break;}}
	if (devid == -1) {
		pin_grp* tmp_pin = new pin_grp[devices + 1];
		for(int i = 0; i < devices; i++){tmp_pin[i] = pins[i];}
		tmp_pin[devices].id = pin;
		devid = devices;
		delete[] pins;
		pins = tmp_pin;
		devices++;
	}
	if(pins[devid].lastRead && millis() - pins[devid].lastRead < (dhttype == DHT22 ? 2000 : 1000)){
		return DHT_TOO_SOON;
	}
	pins[devid].lastRead = millis();
	//end
	
	gpio_num_t dhtpin = static_cast<gpio_num_t>(pin);
	rmt_channel_t dhtrx = static_cast<rmt_channel_t>(rx);
	rmt_config_t rmt_rx = {
		.rmt_mode = RMT_MODE_RX,
		.channel = dhtrx,
		.gpio_num = dhtpin,
		.clk_div = 80,
		.mem_block_num = 1,
		.flags = 0,
		.rx_config = {
			.idle_threshold = 250,
			.filter_ticks_thresh = 30,
			.filter_en = true,
		}
	};
	if(rmt_config(&rmt_rx) == ESP_OK && rmt_driver_install(rmt_rx.channel, 512, ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED) == ESP_OK){
		RingbufHandle_t dhtbuf;
		rmt_get_ringbuf_handle(dhtrx, &dhtbuf);
		gpio_set_level(dhtpin, 1);
		gpio_pullup_dis(dhtpin);
		gpio_pulldown_dis(dhtpin);
		gpio_set_direction(dhtpin, GPIO_MODE_INPUT_OUTPUT_OD);
		gpio_set_intr_type(dhtpin, GPIO_INTR_DISABLE);
		gpio_set_level(dhtpin, 0);
		vTaskDelay((dhttype == DHT22 ? 2 : 22) / portTICK_PERIOD_MS);
		gpio_set_level(dhtpin, 1);
		rmt_rx_start(dhtrx, true);
		size_t rx_size;
		rmt_item32_t *rx_items = (rmt_item32_t *)xRingbufferReceive(dhtbuf, &rx_size, pdMS_TO_TICKS(100));
		uint8_t error = DHT_OK;
		if(rx_items){
			int tot_items = rx_size / sizeof(rmt_item32_t);
			uint8_t pulse = rx_items[0].duration0 + rx_items[0].duration1;
			if(tot_items < 41){
				error = DHT_UNDERFLOW;
			}else if(tot_items > 42){
				error = DHT_OVERFLOW;
			}else if((pulse) < 130 || (pulse) > 180){
				error = DHT_NACK;
			}else{
				uint8_t data[6];
				for(uint8_t i = 0; i < 40; i++){
					pulse = rx_items[i+1].duration0 + rx_items[i+1].duration1;
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
			vRingbufferReturnItem(dhtbuf, (void *)rx_items);
		}else{
			error = DHT_TIMEOUT;
		}
		gpio_set_level(dhtpin, 1);
		rmt_rx_stop(dhtrx);
		rmt_driver_uninstall(dhtrx);
		return error;
	}
	return DHT_DRIVER;
}




