
/* 
  check each condition

  //check ph concentrate level
  //check ec concentrate level
  //check basic solution concentrate level

  //check reservoir level

  return true if at least one liquid level is too low, else return false
*/
bool liquid_levels_low() {

  int ph_up_state = digitalRead(PH_UP_LEVEL_PIN);
  int ph_down_state = digitalRead(PH_DOWN_LEVEL_PIN);
  int nutrient_state = digitalRead(NUTRIENT_LEVEL_PIN);
  int reservoir_state = digitalRead(RESERVOIR_LEVEL_PIN);

  // due to normally open and pullup resistor, switch closed means low
  // returns true if at least one state is LOW
  return !(HIGH == ph_up_state == ph_down_state == nutrient_state == reservoir_state);
}

void dispense_liquid(int pin, int time) {
  digitalWrite(pin, HIGH);
  delay(time);
  digitalWrite(pin, LOW);
}

void read_ph () {
  float ph_conv = 0.41412f;

  float voltage = analogRead(PH_SENSOR_PIN) * (VREF / 1024.0f); // volts / adc resolution

  ph = (7 - voltage / ph_conv) * FILTER_PER + (1 - FILTER_PER) * ph;
}

void read_ec() {
  // normally divided by "compensation coeffient", but assuming 25c we don't need this
  float voltage = analogRead(EC_SENSOR_PIN) * (VREF / 1024.0f);

  // compensation
  float n_ec =  (133.42f * voltage * voltage * voltage
             - 255.86f * voltage * voltage
             + 857.39f * voltage) * 0.5f;

  // apply filtering
  ec = n_ec * FILTER_PER + (1 - FILTER_PER) * ec;
}

bool read_sensors(repeating_timer_t *rt){
  read_ph();
  read_ec();

  return true;
}

void start_sensors () {
  add_repeating_timer_ms(SENSOR_PERIOD_MS, read_sensors, nullptr, &sensor_timer);
}

bool toggle_led(repeating_timer_t *rt) {
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));

  return true;
}

void start_led_flash (int period_ms) {
  // 500 ms on time, period of 1 second
  add_repeating_timer_ms(period_ms, toggle_led, nullptr, &led_timer);
}

void stop_led_flash () {
  // 500 ms on time, period of 1 second
  cancel_repeating_timer(&led_timer);
}

void flow_sensor_pulse() {
  waterflow++;
}

void button_reset() {
  rp2040.reboot();
}

// wait 3 seconds to read time over usb serial and set rtc, if not received keep time as normal
void set_rtc_time() {
  // Give the computer 5 seconds to send the time
    unsigned long start = millis();

    while (millis() - start < 5000) {
        if (Serial.available()) {
            String timestamp = Serial.readStringUntil('\n');

            int year, month, day;
            int hour, minute, second;

            if (sscanf(timestamp.c_str(),
                "%d-%d-%d %d:%d:%d",
                &year, &month, &day,
                &hour, &minute, &second) == 6) {

                rtc.adjust(DateTime(
                    year, month, day,
                    hour, minute, second
                ));

                Serial.println("RTC synchronized");
                break;
            }
        }
    }
}
