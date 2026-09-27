# Prototype Pin Mapping

The following table summarizes the principal GPIO assignments visible in the supplied ESP32-S3 firmware.

| Function | Symbol | GPIO |
|---|---|---:|
| 132 kV bus voltage | `V132_BUS` | `1` |
| 33 kV bus voltage | `V33_BUS` | `2` |
| 132 kV feeder-1 current | `FDR1_I` | `10` |
| 132 kV feeder-2 current | `FDR2_I` | `11` |
| PTR-1 HV current | `HV1_I` | `12` |
| PTR-2 HV current | `HV2_I` | `13` |
| PTR-1 LV current | `LV1_I` | `14` |
| PTR-2 LV current | `LV2_I` | `20` |
| 33 kV feeder-1 current | `F33_1_I` | `8` |
| 33 kV feeder-2 current | `F33_2_I` | `9` |
| Relay outputs | `RELAY1–RELAY8` | `4,5,6,7,15,16,17,18` |

## Protection annunciation outputs

The ESP32-S3 source defines dedicated annunciation outputs for:

- FDR1 Zone 1 / Zone 2 / Zone 3
- FDR1 and FDR2 overcurrent
- PTR1 differential / HV OC / LV OC
- PTR2 differential / HV OC / LV OC
- 33 kV feeder-1 / feeder-2 overcurrent

## Earth-fault controller

The ESP32-DEV source defines dedicated analog inputs for eight earth-fault channels and annunciation outputs for each corresponding event.

> Always verify GPIO conflicts and board-specific pin availability before hardware reuse. The table documents the supplied prototype; it is not a universal ESP32 pin recommendation.
