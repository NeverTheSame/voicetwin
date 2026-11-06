// Light sensor demo: reads a BH1750 light sensor for Python and blinks the built-in LED
// at a speed that depends on the state sent from Python.

#include "Arduino_RouterBridge.h"
#include <Wire.h>

// BH1750 can sit on the header I2C pins (Wire) or the Qwiic connector (Wire1),
// at address 0x23 (ADDR low/floating) or 0x5C (ADDR high). All four are probed.
const uint8_t BH1750_ADDRS[] = {0x23, 0x5C};
const uint8_t BH1750_CONT_HIGH_RES = 0x10;

TwoWire* sensorBus = nullptr;
uint8_t sensorAddr = 0;

volatile unsigned long blinkInterval = 1000;  // ms, starts as "idle"
unsigned long lastToggle = 0;
bool ledOn = false;

bool tryBus(TwoWire* bus) {
    for (uint8_t addr : BH1750_ADDRS) {
        bus->beginTransmission(addr);
        bus->write(BH1750_CONT_HIGH_RES);
        if (bus->endTransmission() == 0) {
            sensorBus = bus;
            sensorAddr = addr;
            return true;
        }
    }
    return false;
}

bool findSensor() {
    Wire.begin();
    if (tryBus(&Wire)) return true;
    Wire1.begin();
    return tryBus(&Wire1);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);  // HIGH means LED is OFF

    Bridge.begin();
    Bridge.provide("set_state", set_state);  // Called from Python with the state name
    Bridge.provide("read_lux", read_lux);    // Called from Python, returns lux or -1

    findSensor();
}

void loop() {
    // Non-blocking blink: no delay(), so Bridge calls keep being handled.
    unsigned long now = millis();
    if (now - lastToggle >= blinkInterval) {
        lastToggle = now;
        ledOn = !ledOn;
        digitalWrite(LED_BUILTIN, ledOn ? LOW : HIGH);  // LOW means LED is ON
    }
}

float read_lux() {
    if (sensorBus == nullptr && !findSensor()) {
        return -1;  // no sensor found; retried on every call so it can be plugged in later
    }
    if (sensorBus->requestFrom(sensorAddr, (uint8_t)2) != 2) {
        sensorBus = nullptr;  // sensor vanished, search again next time
        return -1;
    }
    uint16_t raw = (sensorBus->read() << 8) | sensorBus->read();
    return raw / 1.2f;
}

void set_state(String state) {
    if (state == "idle") {
        blinkInterval = 1000;
    } else if (state == "listening") {
        blinkInterval = 300;
    } else if (state == "speaking") {
        blinkInterval = 100;
    } else if (state == "night") {
        blinkInterval = 2000;
    }
}
