#pragma once
// https://platformio.org/lib/show/5632/DHT22%20lib%20for%20esp-idf/installation
/* 

	DHT22 temperature sensor driver

*/
#include "driver/gpio.h"

#define DHT_OK 0
#define DHT_CHECKSUM_ERROR -1
#define DHT_TIMEOUT_ERROR -2

class Dht22
{
  public:
	Dht22();

	gpio_num_t _pin;

	void init(gpio_num_t gpio);
	void errorHandler(int response);
	int readDht();
	float getHumidity();
	float getTemperature();

  private:
	float _humidity = 0.;
	float _temperature = 0.;

	int getSignalLevel(int usTimeOut, bool state);
};
