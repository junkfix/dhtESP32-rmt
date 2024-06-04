# dhtESP32-rmt
Arduino Library for ESP32 DHT11, DHT21, DHT22, AM2301, AM2302

This is a minimal library with just one function to get the reading, it is using ESP32 RMT peripheral and Arduino to implement a onewire communication with the sensor.

This library is a non-blocking and does not use any CPU delays or disable interrupts, can also be used with other RTM libraries as it installs the driver as shared and gets the readings and uninstall it after every readings.

The example is provided and tested using Arduino 3.0.0 based on ESP-IDF 5.1.4

* Version 1.x is for Arduino 2.x based on ESP-IDF 4.x
* Version 2.x is for Arduino 3.0.0 based on ESP-IDF 5.1.4
---

<a href="https://www.buymeacoffee.com/htmltiger"><img src="https://www.buymeacoffee.com/assets/img/custom_images/white_img.png" alt="Buy Me A Coffee"></a>
