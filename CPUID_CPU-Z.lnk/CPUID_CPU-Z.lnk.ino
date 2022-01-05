const int SLEEP_TIME = 5e6;

void setup() {
  Serial.begin(115200);
  Serial.println("I woke up");
  delay(1000);
  Serial.println("I woke up");
  delay(1000);Serial.println("I woke up");
  delay(1000);Serial.println("I woke up");
  delay(1000);Serial.println("I woke up");
  delay(1000);Serial.println("I woke up");
  delay(1000);Serial.println("I woke up");
  delay(1000);Serial.println("I woke up");
  delay(1000);
  esp_sleep_enable_timer_wakeup(SLEEP_TIME);
  delay(1000);

  Serial.println("I am going to Sleep");
  esp_deep_sleep_start();

}

void loop() {
  // put your main code here, to run repeatedly:

}
