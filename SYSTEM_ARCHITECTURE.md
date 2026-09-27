# System Architecture

## Objective

The prototype demonstrates a compact digital-substation automation concept in which measurement, protection, breaker control, annunciation and IoT supervision are distributed between ESP32 controllers.

## Controller roles

### ESP32-S3 controller

- Acquires bus and feeder/transformer currents.
- Calculates approximate power values.
- Executes distance, overcurrent and transformer protection logic present in the supplied firmware.
- Drives relay outputs used for prototype breaker operation.
- Updates the Blynk dashboard.
- Exchanges protection events with the second controller through UART.

### ESP32-DEV controller

- Acquires dedicated earth-fault analog inputs.
- Operates earth-fault annunciation outputs.
- Transfers earth-fault events over UART.
- Publishes protection events through MQTT.
- Receives MQTT events and exposes the event gateway function.

## Functional chain

```text
Sensors / simulated electrical quantities
              ↓
        ESP32-S3 ADC
              ↓
 Measurement conversion
              ↓
 Protection algorithms ─────→ Relay outputs
              │
              ├─────────────→ LED annunciation
              │
              ├─────────────→ Blynk dashboard
              │
              └── UART ──→ ESP32-DEV
                              │
                              ├── Earth-fault processing
                              ├── Annunciation
                              └── MQTT → HiveMQ Cloud
```

The photographs demonstrate the physical mimic/prototype and the Blynk supervisory interface.
