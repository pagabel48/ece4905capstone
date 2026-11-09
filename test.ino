void test_init() {

  // flash the led
  digitalWrite(led_pin, HIGH);
  digitalWrite(buzzer_pin, HIGH);
  delay(1000);
  digitalWrite(led_pin, LOW);
  digitalWrite(buzzer_pin, LOW);
  delay(1000);

  // test ph pumps
  dispense(ph_pump_pin, pump_time);
  dispense(nutrient_pump_pin, pump_time);
  
  // test the ph and ec sensors
  Serial.println(read_ph());
  Serial.println(read_ec());

  flash();
}