# 132/33 kV Digital Substation Automation Prototype

**Developed by Narala Heshma Sree**  
**B.Tech – Electrical and Electronics Engineering**

A prototype digital-substation automation platform built around ESP32 controllers, electrical measurement inputs, protection logic, relay-based breaker control, UART inter-controller communication, MQTT event communication and a Blynk IoT supervisory dashboard.

> **Project scope:** This repository presents the supplied prototype firmware and hardware demonstration as one standalone **132/33 kV Digital Substation Automation** project. The source code and photographs are educational/prototype material and are not an operational protection or SCADA system.

## 1. What the prototype demonstrates

- 132 kV and 33 kV bus voltage measurement
- 132 kV feeder current measurement
- Power-transformer HV/LV current measurement
- 33 kV feeder current measurement
- Approximate feeder/transformer power calculation for demonstration
- Distance protection with Zones 1, 2 and 3
- Feeder overcurrent protection
- Transformer differential protection
- Transformer HV/LV overcurrent protection
- Earth-fault protection
- Breaker relay control and breaker-status indication
- Protection annunciation and mimic indications
- UART communication between ESP32 controllers
- MQTT event communication using HiveMQ Cloud
- Blynk IoT dashboard for measurements, status and supervisory control

## 2. High-level architecture

```text
                    132/33 kV DIGITAL SUBSTATION
                              PROTOTYPE
                                  │
                ┌─────────────────┴─────────────────┐
                │                                   │
        ESP32-S3 Controller                  ESP32-DEV Controller
        ───────────────────                  ────────────────────
        • Measurements                       • Earth-fault inputs
        • Protection logic                   • Annunciation
        • Breaker relay control              • UART event gateway
        • Breaker/mimic status               • MQTT communication
        • Blynk supervision                  • Protection event publish
                │                                   │
                └────────────── UART ────────────────┘
                                                    │
                                                    ▼
                                              HiveMQ Cloud
                                                    │
                                                    ▼
                                             MQTT event layer
```

The supplied ESP32-S3 firmware uses Blynk IoT for supervisory data and breaker commands. The supplied ESP32-DEV firmware provides earth-fault processing, UART event transfer and MQTT messaging.

## 3. Protection functions

### Distance protection

The controller calculates an approximate apparent impedance using:

```text
Z = V / I
```

The supplied firmware implements three protection zones for the 132 kV feeders:

- Zone 1 — instantaneous operation
- Zone 2 — time-delayed operation
- Zone 3 — remote-backup operation

### Overcurrent protection

Overcurrent pickup logic is implemented for:

- 132 kV feeder-1
- 132 kV feeder-2
- PTR-1 HV
- PTR-1 LV
- PTR-2 HV
- PTR-2 LV
- 33 kV feeder-1
- 33 kV feeder-2

### Differential protection

Transformer differential event handling is implemented for:

- PTR-1
- PTR-2

### Earth-fault protection

The ESP32-DEV controller reads dedicated analog earth-fault inputs for:

- 132 kV feeder-1
- 132 kV feeder-2
- PTR-1 HV
- PTR-1 LV
- PTR-2 HV
- PTR-2 LV
- 33 kV feeder-1
- 33 kV feeder-2

Detected events are annunciated, transferred over UART and published through MQTT.

## 4. Measurements and dashboard

The ESP32-S3 firmware acquires:

- 132 kV bus voltage
- 33 kV bus voltage
- 132 kV feeder currents
- Transformer HV currents
- Transformer LV currents
- 33 kV feeder currents

It then sends the values to Blynk. The prototype dashboard displays electrical measurements, calculated power, breaker controls, protection indications and transformer temperature/status fields used by the demonstration.

## 5. Breaker control

Eight relay outputs are used by the supplied controller for demonstration breaker operation.

The firmware uses an **active-low relay convention**:

```text
HIGH → relay OFF
LOW  → relay ON
```

The Blynk virtual-pin handlers provide demonstration control for the feeder and transformer breaker relay outputs.

## 6. Communication

### UART

The two ESP32 controllers exchange protection events over UART.

Typical event messages include:

```text
ANN_FDR1_Z1
ANN_FDR1_Z2
ANN_FDR1_Z3
ANN_FDR1_OC
ANN_PTR1_DIFF
ANN_PTR1_HVOC
ANN_PTR1_LVOC
ANN_PTR2_DIFF
ANN_PTR2_HVOC
ANN_PTR2_LVOC
ANN_33FDR1_OC
ANN_33FDR2_OC
```

Earth-fault events are also transferred through UART.

### MQTT

The ESP32-DEV firmware uses `PubSubClient` and a secure MQTT port to communicate with HiveMQ Cloud.

The supplied code uses MQTT topics for distance, overcurrent, differential, transformer overcurrent and earth-fault events.

## 7. Hardware prototype

The physical prototype contains:

- ESP32 development boards
- Relay modules
- Indicator LEDs
- Analog input/sensor interfaces
- Breaker/mimic controls
- Protection indication points
- Extensive point-to-point wiring
- Transparent enclosure with front-panel labels

The prototype photographs in `images/` show the physical implementation and Blynk supervisory interface.

## 8. Software

### ESP32-S3 controller

- Arduino framework
- ESP32 Arduino core
- Wi-Fi
- Blynk IoT
- ADC measurement
- Timer-based measurement/protection processing
- UART communication
- Relay outputs

### ESP32-DEV controller

- Arduino framework
- Wi-Fi
- WiFiClientSecure
- PubSubClient
- MQTT over TLS port 8883
- UART communication
- Earth-fault analog inputs
- Annunciation outputs

## 9. Repository contents

```text
132_33kV_Digital_Substation_Automation_Prototype/
│
├── README.md
├── LICENSE
│
├── firmware/
│   ├── Digital_Substation_ESP32_S3.ino
│   └── Digital_Substation_ESP32_DEV.ino
│
├── docs/
│   ├── SYSTEM_ARCHITECTURE.md
│   ├── PROTECTION_FUNCTIONS.md
│   ├── COMMUNICATION.md
│   ├── HARDWARE_AND_DASHBOARD.md
│   ├── PIN_MAPPING.md
│   └── CODE_REVIEW.md
│
├── images/
│   └── prototype and dashboard photographs
│
└── presentation/
    ├── Digital_Substation_Automation_Prototype.pptx
    └── Digital_Substation_Automation_Prototype.pdf
```

## 10. Build requirements

Install:

- ESP32 Arduino core
- Blynk library
- PubSubClient library

Configure before use:

- Blynk Template ID / Name / Auth Token
- Wi-Fi SSID/password
- MQTT server/username/password

Never commit real credentials to GitHub.

## 11. Important engineering disclaimer

This is an **educational prototype**. It must not be connected directly to energized APTRANSCO equipment and must not be treated as a replacement for certified protection relays, interlocking, RTU/SAS or an operational SCADA/control system.

The firmware contains relay-control outputs. Testing should be performed only with isolated low-voltage hardware and appropriate supervision.

## 12. Author

**Narala Heshma Sree**  
B.Tech – Electrical and Electronics Engineering

