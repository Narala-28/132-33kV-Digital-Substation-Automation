# Protection Functions

The following descriptions are based on the supplied firmware.

## 1. Distance protection

The firmware calculates approximate apparent impedance using `Z = V / I`.

The supplied distance logic includes:

| Function | Demonstration behaviour |
|---|---|
| Zone 1 | Instantaneous trip |
| Zone 2 | 1 second delay |
| Zone 3 | 2 second delay / remote backup |

The firmware contains separate distance handling for the two 132 kV feeder paths.

## 2. Feeder overcurrent

The firmware implements pickup-based overcurrent logic for the 132 kV feeder paths and 33 kV feeder paths. When pickup is exceeded, the associated relay output and annunciation are operated and an event is sent to the communication controller.

## 3. Transformer protection

The supplied code includes transformer:

- Differential event handling
- HV overcurrent
- LV overcurrent

for two transformer paths.

## 4. Earth fault

The ESP32-DEV firmware defines eight earth-fault analog channels covering two 132 kV feeders, two sides of each of two transformers, and two 33 kV feeders.

The supplied pickup settings include:

- 132 kV feeder EF: 60
- Transformer HV EF: 16
- Transformer LV EF: 64
- 33 kV feeder EF: 60

These are prototype scaling/pickup values from the supplied code and should not be interpreted as protection settings for field equipment.

## 5. Event handling

Event flags are used to avoid repeatedly publishing the same event while a condition remains active. Reset messages are sent when the corresponding prototype condition clears.
