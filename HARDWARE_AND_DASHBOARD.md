# Hardware and Dashboard

## Physical prototype

The prototype is assembled inside a transparent enclosure with a labeled front panel representing:

- 132/33 kV substation
- 132 kV feeder-1 and feeder-2
- 132 kV bus
- Transformer HV/LV sections
- 33 kV bus
- 33 kV feeder-1 and feeder-2
- CBO/CBC breaker indications
- Zone 1/2/3
- Differential, overcurrent and earth-fault indications

The internal photograph shows ESP32 controller hardware, relay modules, terminal/interconnect wiring and the indicator/control circuitry.

## Supervisory dashboard

The Blynk screenshots demonstrate:

- Current
- Voltage
- Power
- Breaker controls/status
- Protection indications
- Transformer HV/LV values
- Transformer temperature/status fields
- 132 kV and 33 kV bus visualization

The dashboard is a prototype supervisory interface and is not an operational SCADA/HMI replacement.

## Demonstration value

The combination of physical mimic hardware and digital dashboard makes it possible to demonstrate a protection event from input condition through:

```text
Input → Protection logic → Relay/annunciation
      → UART event → MQTT event → Dashboard/status
```
