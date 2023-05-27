#include <dhtESP32-rmt.h>

float temperature = 0.0;
float humidity = 0.0;

/*
void tempTask(void *pvParameters){
	for(;;){
		uint8_t error=read_dht(temperature, humidity, 13, DHT22, 1);
		if(error){
			Serial.println(error);
		}else{
			Serial.println(temperature);
			Serial.println(humidity);
		}		
		vTaskDelay(3000 / portTICK_PERIOD_MS);
	}
}
/* tempTask */

void setup() {
	delay(1000);
	Serial.begin(115200);
	//xTaskCreatePinnedToCore(tempTask, "tempTask", 8192,  NULL,  1,  NULL, 0);
}

void loop() {
	//read_dht(temperature, humidity, pin, type (DHT11, DHT21, DHT22, AM2301, AM2302), RMT channel (0-7) 
	uint8_t error=read_dht(temperature, humidity, 13, DHT22, 0);
	if(error){
		Serial.println(error);
	}else{
		Serial.println(temperature);
		Serial.println(humidity);
	}
	delay(3000);	
}

/*
Error codes
0: OK
1: TOO_SOON
2: DRIVER
3: TIMEOUT
4: NACK
5: BAD_DATA
6: CHECKSUM
7: UNDERFLOW
8: OVERFLOW
*/
