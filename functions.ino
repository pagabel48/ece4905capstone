void dispense(int pin, int time) {
  digitalWrite(pin, HIGH);
  delay(time);
  digitalWrite(pin, LOW);
}

float read_ph () {
  float ph_conv = 0.41412f;

  float voltage = analogRead(ph_sensor_pin) * (5.0f / 1023.0f); // volts / adc resolution

  // arduino can only read positive voltages so reading can't be higher than 7 ph
  return 7 - voltage / ph_conv;
}

float read_ec() {
  float voltage = analogRead(tds_sensor_pin) * (5.0f / 1023.0f);

  // compensation
  return (133.42f * voltage * voltage * voltage
             - 255.86f * voltage * voltage
             + 857.39f * voltage) * 0.5f;
}

void flash() {
  while (1){
    digitalWrite(led_pin, HIGH);
    delay(1000);
    digitalWrite(led_pin, LOW);
    delay(1000);
  }
}

void pulse() {
  waterflow += 1.0 / 5880.0;
}
