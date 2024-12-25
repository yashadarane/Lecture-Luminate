#include <Wire.h>
#include <RTClib.h>

#define PIR_PIN 2         // Pin connected to the PIR sensor
#define LED_PIN 13        // Pin connected to the LED

RTC_DS3231 rtc;          // Create an instance of the RTC object

// Define the on/off times for the lights for each day of the week (24-hour format)
// Format: {onHour, onMinute, offHour, offMinute}
int dailyTimes[7][4] = {
  {8, 15, 12, 30},  // Monday
  {8, 15, 12, 30},  // Tuesday
  {10, 30, 12, 30}, // Wednesday
  {8, 15, 12, 30},  // Thursday
  {8, 15, 12, 30},  // Friday
  // Repeat for the remaining days of the week if needed
};

unsigned long lastMotionTime = 0;  // Variable to store the time when motion was last detected
unsigned long debounceDelay = 10000;  // Debounce delay in milliseconds (adjust as needed)
unsigned long debugPrintDelay = 1000;  // Delay between debug print statements in milliseconds

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Initialize the serial communication
  Serial.begin(9600);

  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(2024, 4, 24, 12, 28, 0));
  }
}

void loop() {
  // Read the current time from the RTC
  DateTime now = rtc.now();

  // Get the current day of the week (Sunday = 1, Monday = 2, ..., Saturday = 7)
  int currentDay = now.dayOfTheWeek();

  // Check if it's time to turn the lights on/off for the current day
  int onHour = dailyTimes[currentDay - 1][0];
  int onMinute = dailyTimes[currentDay - 1][1];
  int offHour = dailyTimes[currentDay - 1][2];
  int offMinute = dailyTimes[currentDay - 1][3];

  int currentHour = now.hour();
  int currentMinute = now.minute();

  // Debugging output
  Serial.print("Current day: ");
  Serial.println(currentDay);
  delay(debugPrintDelay);
  Serial.print("Current time: ");
  Serial.print(currentHour);
  Serial.print(":");
  Serial.println(currentMinute);
  delay(debugPrintDelay);
  Serial.print("On time: ");
  Serial.print(onHour);
  Serial.print(":");
  Serial.println(onMinute);
  delay(debugPrintDelay);
  Serial.print("Off time: ");
  Serial.print(offHour);
  Serial.print(":");
  Serial.println(offMinute);
  delay(debugPrintDelay);

  // Check if it's time to turn the lights on/off
  if ((currentHour == onHour && currentMinute >= onMinute) ||
      (currentHour == offHour && currentMinute <= offMinute) ||
      (currentHour > onHour && currentHour < offHour)) {
    // Within the specified time range, turn on the lights
    turnOnLights();
    // Stop motion sensing
    lastMotionTime = millis();
  } else {
    // Outside the specified time range, turn off the lights
    turnOffLights();
    // Resume motion sensing
    if (digitalRead(PIR_PIN) == HIGH) {
      // Motion detected, check if enough time has passed since last detection
      if (millis() - lastMotionTime >= debounceDelay) {
        // Sufficient debounce delay has elapsed, turn on the lights
        turnOnLights();
        delay(10000);
        // Update the last motion detection time
        lastMotionTime = millis();
      }
    }
  }
}

void turnOnLights() {
  digitalWrite(LED_PIN, HIGH);  //  Turn on the lights
  Serial.println("Lights turned on");
}

void turnOffLights() {
  digitalWrite(LED_PIN, LOW);   // Turn off the lights
  Serial.println("Lights turned off");
}
