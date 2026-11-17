
/*

this script satisfies tests 3.3, 3.4, 3.6

tests 3.1, 3.2, 3.5 must be tested during normal operation

*/
void test_init() {

  // demonstrate led flashing while setting up the rtc
  // low liquid level failure condition
  // test 3.6
  print_datetime(rtc.now());

  start_led_flash(300);
  set_rtc_time();
  stop_led_flash();

  print_datetime(rtc.now());

  // hold led on and activate buzzer, no flow failure condition
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);

  delay (1000);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // test ph pumps
  dispense_liquid(PH_UP_PUMP_PIN, PH_UP_PUMP_TIME);
  dispense_liquid(PH_DOWN_PUMP_PIN, PH_DOWN_PUMP_TIME);
  dispense_liquid(NUTRIENT_PUMP_PIN, NUTRIENT_PUMP_TIME);
  
  // test the ph and ec sensors
  Serial.print("PH: ");
  Serial.println(ph);
  Serial.print("EC: ");
  Serial.println(ec);

  // toggle the pump and light off and on, 2 seconds
  digitalWrite(LIGHT_CTRL_PIN, LOW);
  digitalWrite(CIRCULATION_CTRL_PIN, LOW);

  delay(2000);

  digitalWrite(LIGHT_CTRL_PIN, HIGH);
  digitalWrite(CIRCULATION_CTRL_PIN, HIGH);

  // toggle main light on and off 1 minute intervals, test 3.3
  // buzz and turn on light when ec measurement is within range, test 3.4
  while (1) {
    int minute = rtc.now().minute();

    // light is on when minute is odd (ex 5:31, 7:53, not 7:52 or 5:30)
    digitalWrite(LIGHT_CTRL_PIN, minute % 2);

    // comment out for prod
    Serial.print("PH: ");
    Serial.println(ph);
    Serial.print("EC: ");
    Serial.println(ec);

    // buzz and turn on led if ec is within threshold, else stay silent
    if (EC_SETPOINT - EC_THRESHOLD < ec < EC_SETPOINT + EC_THRESHOLD) {
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
    }

    delay (20);
  }
}

void print_datetime(DateTime dt) {
    Serial.print(dt.year());
    Serial.print("/");
    Serial.print(dt.month());
    Serial.print("/");
    Serial.print(dt.day());

    Serial.print(" ");

    Serial.print(dt.hour());
    Serial.print(":");
    Serial.print(dt.minute());
    Serial.print(":");
    Serial.println(dt.second());
}