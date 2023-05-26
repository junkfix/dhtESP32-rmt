#include "dhtESP32-rmt.h"

float temp;
float humi;


/*
void tempTask(void *pvParameters){
	for(;;){
		uint8_t err=read_dht(temp, humi, 13, DHT22, 1);
		if(err){
			Serial.println(err);
		}else{
			Serial.println(temp);
			Serial.println(humi);
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
	//read_dht(temperature, humidity, pin, type (DHT11, DHT21, DHT22, AM2301, AM2302), rmt channel (0-7) 
	uint8_t err=read_dht(temp, humi, 13, DHT22, 0);
	if(err){
		Serial.println(err);
	}else{
		Serial.println(temp);
		Serial.println(humi);
	}
	delay(3000);	
}
