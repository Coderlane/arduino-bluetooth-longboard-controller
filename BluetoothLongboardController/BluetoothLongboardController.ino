#include <HardwareBluetoothRN42.h>
#include <HardwareSerial.h>
#include <Throttle.h>


// Must be analog pin
const int throttle_pin = 18; // A0

const int status_pin = 2;
const int status_led_pin = 3;

long prev_millis = 0;
long interval_millis = 1000;
int status_led_state = LOW;
int recent_disconnect = 0;

Throttle throttle(throttle_pin);
HardwareBluetoothRN42 bluetooth((HardwareSerial&) Serial, status_pin, 0, "BlueRemote", "3145");

void connection_up();
void connection_lost();
void connection_down();

void
setup()
{
  bluetooth.setup();
  pinMode(status_led_pin, OUTPUT);
}

void
loop ()
{
  long cur_millis = millis();
  if (cur_millis - prev_millis > interval_millis) {
    prev_millis = cur_millis;

    if (bluetooth.isConnected()) {
      connection_up();
    } else {
      if (recent_disconnect < 10) {
        connection_lost();
      } else {
        connection_down();
      }
    }
  }
}


inline void connection_up()
{
  // Connection is up.

  recent_disconnect = 0;
  interval_millis = 1000;

  int percent = throttle.read();
  // Send the data
  bluetooth.println(percent);

  if (status_led_state != HIGH) {
    // Turn on LED
    digitalWrite(status_led_pin, HIGH);
    status_led_state = HIGH;
  }
}

inline void connection_lost()
{
  // Connection recently lost.
  // Attempt to resend quickly :)

  interval_millis = 500;
  recent_disconnect++;

  // Blink LED
  if (status_led_state == LOW) {
    digitalWrite(status_led_pin, HIGH);
    status_led_state = HIGH;
  }  else {
    digitalWrite(status_led_pin, LOW);
    status_led_state = LOW;
  }
}

inline void connection_down()
{
  // Connection has been lost for a while.

  interval_millis = 2000;
  if (status_led_state != LOW) {
    // Turn off LED
    digitalWrite(status_led_pin, LOW);
    status_led_state = LOW;
  }
}

