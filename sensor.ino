#include "DHT.h"

#define DHTPIN_INSIDE 2
#define DHTPIN_OUTSIDE 3
#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DEBUG 1

DHT sensorInside(DHTPIN_INSIDE, DHTTYPE);
DHT sensorOutside(DHTPIN_OUTSIDE, DHTTYPE);

struct sensorData {
  bool hasData;
  float humidity;
  float temperature;
};

struct climateData {
  struct sensorData *inside; 
  struct sensorData *outside; 
};

void setup() {
  Serial.begin(9600); 

  sensorInside.begin();
  sensorOutside.begin();
}

void loop() {
  climateData *data = readClimateData();
  printClimateData(data);
  delay(10000);
}

sensorData *readSensor(DHT dht){
  sensorData *data = (sensorData*) malloc(sizeof(sensorData));
  
  data->humidity = dht.readHumidity();
  data->temperature = dht.readTemperature();

  if(isnan(data->humidity) || isnan(data->temperature)){
    data->hasData = false;
    return data;
  } else {
    data->hasData = true; 
    return data; 
  }
}

climateData *readClimateData(){
  climateData *data = (climateData*)malloc(sizeof(climateData));

  data->inside = readSensor(sensorInside);
  data->outside = readSensor(sensorOutside);
  
  #if (DEBUG==1)
    if(!data->inside->hasData){
      Serial.println("Error, reading from inside sensor!");
    }
    if(!data->outside->hasData){
      Serial.println("Error, reading from outside sensor!");
    }
  #endif
  
  return data;
}

void printClimateData(climateData *data){  
  Serial.println("----Inside----");
  printSensorData(data->inside);
  Serial.println("----Outside----");
  printSensorData(data->outside);
  Serial.println("###############");
}

void printSensorData(sensorData *data){
  if(data->hasData){
    Serial.print("Humidity: "); 
    Serial.print(data->humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
    Serial.print(data->temperature);
    Serial.println(" *C");
  } else {
    Serial.println("No data!");
  }
}

