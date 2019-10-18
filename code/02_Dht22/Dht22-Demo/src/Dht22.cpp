
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>

#include "Dht22.h"

static char TAG[] = "DHT";

//const int LENGTH_LOGGER_MESSAGE = 300;

/*-----------------------------------------------------------------------
;
;	Constructor & default settings
;
;------------------------------------------------------------------------*/

Dht22::Dht22()
{

    _pin = GPIO_NUM_4;
    _humidity = -100;
    _temperature = -100;
}

/*
DHT::~DHT( void )
{
}
*/

// ----------------------------------------------------------------------

void measurementInLoopTask(void *pvParameter)
{
    Dht22 *dhtPtr = (Dht22 *)pvParameter;

    while (1)
    {
        int ret = dhtPtr->readDht();
        dhtPtr->errorHandler(ret);
        vTaskDelay(5000 / portTICK_RATE_MS); // 5 Seconds
    }
}

void Dht22::init(gpio_num_t gpio)
{
    _pin = gpio;
    xTaskCreate(measurementInLoopTask,   /* Task function. */
                "measurementInLoopTask", /* String with name of task. */
                10000,                   /* Stack size in words. */
                this,                    /* Parameter passed as input of the task */
                1,                       /* Priority of the task ==> lowest priority */
                NULL                     /* Task handle. */
    );
}

// == get temp & hum =============================================

float Dht22::getHumidity() { return _humidity; }
float Dht22::getTemperature() { return _temperature; }

// == error handler ===============================================

void Dht22::errorHandler(int errorCode)
{
    switch (errorCode)
    {
    case DHT_TIMEOUT_ERROR:
        ESP_LOGE(TAG, "Sensor Timeout\n");
        break;

    case DHT_CHECKSUM_ERROR:
        ESP_LOGE(TAG, "CheckSum error\n");
        break;

    case DHT_OK:
        break;

    default:
        ESP_LOGE(TAG, "Unknown error\n");
    }
}

/**
 * Warten bis der gewünschte Zustand des Pins erreicht ist.
 * Timeout als Notausgang.
 * Läuft im Hintergrund in eigenem Task.
 */
int Dht22::getSignalLevel(int usTimeOut, bool state)
{

    int uSec = 0;
    while (gpio_get_level(_pin) == state)
    {

        if (uSec > usTimeOut)
            return -1;

        ++uSec;
        ets_delay_us(1); // uSec delay
    }

    return uSec;
}

/*----------------------------------------------------------------------------
;
;	read DHT22 sensor

	copy/paste from AM2302/DHT22 Docu:
	DATA: Hum = 16 bits, Temp = 16 Bits, check-sum = 8 Bits
	Example: MCU has received 40 bits data from AM2302 as
	0000 0010 1000 1100 0000 0001 0101 1111 1110 1110
	16 bits RH data + 16 bits T data + check sum

	1) we convert 16 bits RH data from binary system to decimal system, 0000 0010 1000 1100 → 652
	Binary system Decimal system: RH=652/10=65.2%RH
	2) we convert 16 bits T data from binary system to decimal system, 0000 0001 0101 1111 → 351
	Binary system Decimal system: T=351/10=35.1°C
	When highest bit of temperature is 1, it means the temperature is below 0 degree Celsius.
	Example: 1000 0000 0110 0101, T= minus 10.1°C: 16 bits T data
	3) Check Sum=0000 0010+1000 1100+0000 0001+0101 1111=1110 1110 Check-sum=the last 8 bits of Sum=11101110

	Signal & Timings:
	The interval of whole process must be beyond 2 seconds.

	To request data from DHT:
	1) Sent low pulse for > 1~10 ms (MILI SEC)
	2) Sent high pulse for > 20~40 us (Micros).
	3) When DHT detects the start signal, it will pull low the bus 80us as response signal,
	   then the DHT pulls up 80us for preparation to send data.
	4) When DHT is sending data to MCU, every bit's transmission begin with low-voltage-level that last 50us,
	   the following high-voltage-level signal's length decide the bit is "1" or "0".
		0: 26~28 us
		1: 70 us
;----------------------------------------------------------------------------*/

const int DhtBytes=5; // to complete 40 = 5*8 Bits

int Dht22::readDht()
{
    int uSec = 0;
    uint8_t dhtData[DhtBytes];
    uint8_t byteIndex = 0;
    uint8_t bitIndex = 7;

    for (int i = 0; i < DhtBytes; i++)
        dhtData[i] = 0;

    // == Send start signal to DHT sensor ===========
    gpio_set_direction(_pin, GPIO_MODE_OUTPUT);
    // pull down for 3 ms for a smooth and nice wake up
    gpio_set_level(_pin, 0);
    ets_delay_us(3000);
    // pull up for 25 us for a gentile asking for data
    gpio_set_level(_pin, 1);
    ets_delay_us(25);
    gpio_set_direction(_pin, GPIO_MODE_INPUT); // change to input mode

    // == DHT will keep the line low for 80 us and then high for 80us ====
    uSec = getSignalLevel(85, 0);
    ESP_LOGD(TAG, "Response = %d", uSec);
    if (uSec < 0) return DHT_TIMEOUT_ERROR;
    // -- 80us up ------------------------
    uSec = getSignalLevel(85, 1);
    ESP_LOGD(TAG, "Response = %d", uSec);
    if (uSec < 0) return DHT_TIMEOUT_ERROR;

    // == No errors, read the 40 data bits ================
    for (int k = 0; k < 40; k++)
    {
        // -- starts new data transmission with >50us low signal
        uSec = getSignalLevel(56, 0);
        if (uSec < 0)  return DHT_TIMEOUT_ERROR;
        // -- check to see if after >70us rx data is a 0 or a 1
        uSec = getSignalLevel(80, 1); //! 75
        if (uSec < 0) return DHT_TIMEOUT_ERROR;
        if (uSec > 40)  // 0 steht schon im Array ==> nur 1 muss gesetzt werden
        {
            dhtData[byteIndex] |= (1 << bitIndex);
        }
        // index to next byte
        if (bitIndex == 0)
        {
            bitIndex = 7;
            byteIndex++;
        }
        else
            bitIndex--;
    }
    // == get humidity from Data[0] and Data[1] ==========================
    _humidity = dhtData[0];
    _humidity *= 0x100; // >> 8
    _humidity += dhtData[1];
    _humidity /= 10; // get the decimal
    // == get temp from Data[2] and Data[3]
    _temperature = dhtData[2] & 0x7F;
    _temperature *= 0x100; // >> 8
    _temperature += dhtData[3];
    _temperature /= 10;
    if (dhtData[2] & 0x80) // negative temp, brrr it's freezing
        _temperature *= -1;
    // == verify if checksum is ok ===========================================
    // Checksum is the sum of Data 8 bits masked out 0xFF
    if (dhtData[4] == ((dhtData[0] + dhtData[1] + dhtData[2] + dhtData[3]) & 0xFF))
        return DHT_OK;
    else
        return DHT_CHECKSUM_ERROR;
}