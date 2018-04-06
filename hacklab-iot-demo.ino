#include <DHT.h>

#define DHTPIN 8
#define DHTTYPE DHT11

DHT climate(DHTPIN, DHTTYPE);

void setup()
{
    Serial.begin(115200);
    climate.begin();
    Serial.println("\n\n\t\t\tHACKLAB IOT DEMO\n");
}

void loop()
{
    int temp = climate.readTemperature(),
        hum = climate.readHumidity();

    if (isnan(temp) || isnan(hum))
    {
        Serial.println("Cannot read temperature or humidity");
        return;
    }
    Serial.print("Temperature:  ");
    Serial.print(temp);
    Serial.print("\t\tHumidity:  ");
    Serial.println(hum);
}