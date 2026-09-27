# Communication

## UART

UART is used as the local controller-to-controller event path.

The ESP32-S3 sends protection annunciation/event strings such as:

- `ANN_FDR1_Z1`
- `ANN_FDR1_Z2`
- `ANN_FDR1_Z3`
- `ANN_FDR1_OC`
- `ANN_PTR1_DIFF`
- `ANN_PTR1_HVOC`
- `ANN_PTR1_LVOC`
- `ANN_PTR2_DIFF`
- `ANN_PTR2_HVOC`
- `ANN_PTR2_LVOC`
- `ANN_33FDR1_OC`
- `ANN_33FDR2_OC`

The communication controller converts events into MQTT publications.

## MQTT

The ESP32-DEV firmware uses:

- Wi-Fi
- `WiFiClientSecure`
- `PubSubClient`
- MQTT port `8883`

The supplied firmware publishes protection events under structured topics for distance, overcurrent, differential, transformer overcurrent and earth fault.

It also subscribes to a remote event namespace in the supplied source.

## Blynk

The ESP32-S3 firmware uses Blynk IoT for:

- Measurement display
- Protection status
- Breaker control
- Breaker/status indication
- Demonstration supervisory control

## Security note

The supplied prototype code uses `espClient.setInsecure()` for the MQTT TLS client. This is explicitly identified in the source as a prototype configuration. Production deployment would require proper CA/server certificate validation and a reviewed credential-management approach.
