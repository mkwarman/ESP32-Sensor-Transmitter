#include <WiFi.h>
#include <HTTPClient.h>
#include <DHTesp.h>

#define DHTPIN 15     // what digital pin we're connected to
#define BATTERY_PIN 35
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  20        /* Time ESP32 will go to sleep (in seconds) */
#define LED_PIN 13
 
const char* ssid = "SSID";
const char* password =  "PASSWORD";

RTC_DATA_ATTR int failureCount = 0;

DHTesp dht;
/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  WiFi.begin(ssid, password);
  dht.setup(DHTPIN, DHTesp::DHT22); // Initialize temperature sensor
  //Serial.begin(115200);
  float voltage = analogRead(BATTERY_PIN);
  int connection_attempts = 0;

  while (connection_attempts < 20) {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    connection_attempts++;
    delay(500);
    //Serial.println("Connecting to WiFi..");
  }

  //Serial.println("Connected to the WiFi network");
  HTTPClient http;   

  http.setTimeout(1); // We dont care about the response
  http.begin("http://192.168.1.194/update");  //Specify destination for HTTP request
  http.addHeader("Content-Type", "text/plain");             //Specify content-type header

  TempAndHumidity lastValues = dht.getTempAndHumidity();

  if (isnan(lastValues.temperature) || isnan(lastValues.humidity)) {
    //Serial.println("Failed to read from DHT sensor!");
    // This should be removed prior to production. The RPi can do this.
    ++failureCount;
  }
  /*
  else {
    Serial.print("Humidity: ");
    //Serial.print(h);
    Serial.print(String(lastValues.humidity,0));
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    //Serial.print(t);
    Serial.print(String(lastValues.temperature,0));
    Serial.print(" *C ");
    Serial.print(" --- Battery: ");
    Serial.print(voltage);
  }
  
  Serial.println("About to send request");
  */
  
  int httpResponseCode = http.POST("Humidity: "
  + String(lastValues.humidity,0)
  + "% --- Temperature: "
  + String(lastValues.temperature,0)
  + "*C --- Battery: "
  + String(voltage)
  + " --- Failures: "
  + String(failureCount));   //Send the actual POST request

  //Serial.println("Request completed");
  
  http.end();  //Free resources

  //Serial.println("failure count: " + String(failureCount));
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start(); // Deep sleep
}

void loop(){
  //This is not going to be called
}
