#include "DHT.h"

#define DHTPIN_INSIDE 2
#define DHTPIN_OUTSIDE 3
#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define DEBUG 1

#define SENSOR_READ_PERIOD 30000  // Time between every measuring in ms. (30000ms is 0.5 min)
#define UPLOAD_PERIOD 900000      // Time between every uploading in ms. (900000ms is 15 min)
#define EXTRA_SPACE_IN_QUEUE 4    // X times more places in the queue

DHT sensorInside(DHTPIN_INSIDE, DHTTYPE);
DHT sensorOutside(DHTPIN_OUTSIDE, DHTTYPE);

int queueSize = 0;
int itemsInQueue = 0;
struct climateData **climateDataQueue; 

struct sensorData {
  bool hasData;
  float humidity;
  float temperature;
};

struct climateData {
  long timestamp; //Since the Arduino started, overflows every ~50 days.
  struct sensorData *inside; 
  struct sensorData *outside; 
};

void setup() {
  Serial.begin(9600); 
  initClimateDataQueue();

  sensorInside.begin();
  sensorOutside.begin();
}

void loop() {
  newDataPoint();  
  delay(SENSOR_READ_PERIOD);
}

void newDataPoint(){
  struct climateData *data = readClimateData();
  #if (DEBUG==1)
    logClimateData(data);
  #endif
  
  climateDataQueue[itemsInQueue] = data; //TODO do some more fazy cyclic
  
  itemsInQueue++;
  
  if(itemsInQueue >= queueSize){
    #if (DEBUG==1)
      Serial.println("The queue is full, need to send the data.");
      sendData();
    #endif
  }
}

sensorData *readSensor(DHT dht){
  struct sensorData *data = (struct sensorData*) malloc(sizeof(struct sensorData));
  
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
  struct climateData *data = (struct climateData*)malloc(sizeof(struct climateData));
  
  data->timestamp = millis();
  data->inside = readSensor(sensorInside);
  data->outside = readSensor(sensorOutside);
  
  #if (DEBUG==1)
    if(!data->inside->hasData){
      Serial.print("Error, reading from inside sensor! At: ");
      Serial.println(data->timestamp);
    }
    if(!data->outside->hasData){
      Serial.print("Error, reading from outside sensor! At: ");
      Serial.println(data->timestamp);
    }
  #endif
  
  return data;
}

void logClimateData(climateData *data){  
  Serial.println("###############");
  Serial.println("----Time----");
  Serial.print(data->timestamp);
  Serial.println(" ms");
  Serial.println("----Inside----");
  logSensorData(data->inside);
  Serial.println("----Outside----");
  logSensorData(data->outside);
  Serial.println("###############");
}

void logSensorData(sensorData *data){
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

void initClimateDataQueue(){
  queueSize = (1+((UPLOAD_PERIOD-1)/SENSOR_READ_PERIOD)) * EXTRA_SPACE_IN_QUEUE; //Ceil
  climateDataQueue = (struct climateData**) malloc(sizeof(struct climateData)*queueSize);
  for(int i = 0;i < queueSize;i++){
    climateDataQueue[i] = 0;
  }
}

void sendData(){
  //TODO implement 
  #if (DEBUG==1)
    Serial.println("Sending data"); 
  #endif
}

