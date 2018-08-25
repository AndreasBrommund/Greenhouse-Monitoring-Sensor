#include "DHT.h"

#define DHTPIN_INSIDE 2
#define DHTPIN_OUTSIDE 3
#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define DEBUG 1

#define SENSOR_READ_PERIOD 30000  // Time between every measuring in ms. (30000ms is 0.5 min)
#define UPLOAD_PERIOD 900000      // Time between every uploading in ms. (900000ms is 15 min)
#define EXTRA_SPACE_IN_BUFFER 4    // X times more places in the queue

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


int bufferSize = 0;
int firstIndexBuffer = 0; //[0..(bufferSize-1)]
int itemsInBuffer = 0;    //[0..bufferSize]
struct climateData **climateDataBuffer;

DHT sensorOutside(DHTPIN_OUTSIDE, DHTTYPE);
DHT sensorInside(DHTPIN_INSIDE, DHTTYPE);

/***************************
           Code
 ***************************/

void setup() {
  Serial.begin(9600);
  initClimateDataBuffer();

#if (DEBUG==1)
  logBuffer();
#endif

  sensorInside.begin();
  sensorOutside.begin();
}

void loop() {


  newDataPoint();

#if (DEBUG==1)
  logBuffer();
#endif

  delay(SENSOR_READ_PERIOD);
}

void newDataPoint() {

  struct climateData * data = getNextSlot_Buffer();
  if (data == NULL) {
    //TODO the buffer is full
    return;
  }

  readClimateData(data);

#if (DEBUG==1)
  logClimateData(data);
#endif
}

void sendData() {
  //TODO implement
#if (DEBUG==1)
  Serial.println("Sending data");
#endif
}


/***************************
        Buffer
 ***************************/
void initSensorData(struct sensorData* data) {
  data->hasData = false;
  data->humidity = 0.0f;
  data->temperature = 0.0f;
}

void initClimateData(struct climateData* data) {
  data->timestamp = 0;
  initSensorData(data->inside);
  initSensorData(data->outside);
}

void initClimateDataBuffer() {
  bufferSize = (1 + ((UPLOAD_PERIOD - 1) / SENSOR_READ_PERIOD)) * EXTRA_SPACE_IN_BUFFER; //Ceil
  itemsInBuffer = 0;
  firstIndexBuffer = 0;

  climateDataBuffer = (struct climateData**) malloc(sizeof(struct climateData) * bufferSize);

  for (int i = 0; i < bufferSize; i++) {
    struct climateData * data = (struct climateData*) malloc(sizeof(struct climateData));
    data->inside = (struct sensorData*) malloc(sizeof(struct sensorData));
    data->outside = (struct sensorData*) malloc(sizeof(struct sensorData));

    initClimateData(data);

    climateDataBuffer[i] = data;
  }
}

climateData * getNextSlot_Buffer() {
  if (itemsInBuffer >= bufferSize) {
#if (DEBUG==1)
    Serial.println("The buffer is full");
#endif
    return NULL;
  }

  int index = (firstIndexBuffer + itemsInBuffer) % bufferSize;
  itemsInBuffer++;
  return climateDataBuffer[index];
}

void removeOldest_Buffer() {
  if (itemsInBuffer <= 0) {
#if (DEBUG==1)
    Serial.println("Remove from empty buffer");
#endif
    return;
  }

  initClimateData(climateDataBuffer[firstIndexBuffer]);

  firstIndexBuffer = (firstIndexBuffer + 1) % bufferSize;
  itemsInBuffer--;
}

climateData * getOldest_Buffer() {
  if (itemsInBuffer <= 0) {
#if (DEBUG==1)
    Serial.println("The buffer is empty");
#endif
    return NULL;
  }
  return climateDataBuffer[firstIndexBuffer];
}
/***************************
        Read sensors
 ***************************/

void readSensor(DHT dht, struct sensorData * data) {
  data->humidity = dht.readHumidity();
  data->temperature = dht.readTemperature();

  if (isnan(data->humidity) || isnan(data->temperature)) {
    data->hasData = false;
  } else {
    data->hasData = true;
  }
}

void readClimateData(struct climateData * data) {

  data->timestamp = millis();
  readSensor(sensorInside, data->inside);
  readSensor(sensorOutside, data->outside);

#if (DEBUG==1)
  if (!data->inside->hasData) {
    Serial.print("Error, reading from inside sensor! At: ");
    Serial.println(data->timestamp);
  }
  if (!data->outside->hasData) {
    Serial.print("Error, reading from outside sensor! At: ");
    Serial.println(data->timestamp);
  }
#endif
}

/***************************
           Log
 ***************************/

void logBuffer() {
  Serial.println("%%%%%%%%%%%%%%%");
  Serial.println("%%%%%%%%%%%%%%%");
  Serial.print("Buffer size: ");
  Serial.println(bufferSize);
  Serial.print("Items in buffer: ");
  Serial.println(itemsInBuffer);
  for (int i = 0; i < bufferSize; i++) {

    if (i == firstIndexBuffer) {
      Serial.print("->");
    }
    Serial.print("Index: ");
    Serial.println(i);
    logClimateData(climateDataBuffer[i]);
  }
  Serial.println("%%%%%%%%%%%%%%%");
  Serial.println("%%%%%%%%%%%%%%%");
}

void logClimateData(climateData *data) {
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

void logSensorData(sensorData *data) {
  if (data->hasData) {
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

