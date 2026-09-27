# Source-Level Code Review Notes

This document records observations from the supplied prototype firmware. It does not replace a formal protection-engineering review.

## Positive implementation features

- Clear separation between measurement/protection and communication/earth-fault functions.
- Explicit GPIO definitions.
- Event flags reduce duplicate event transmission.
- UART event strings are easy to trace during prototype testing.
- MQTT topics are organized by protection function.
- Blynk virtual pins are used for supervisory visualization and prototype breaker control.
- The source includes explicit safety comments around prototype relay control.

## Items requiring engineering review before any field use

### MQTT TLS validation

The communication firmware calls `espClient.setInsecure()`. The source itself notes that CA validation should replace this for production.

### Protection calculations

The source uses simplified ADC-to-engineering-unit mapping and demonstration calculations. For example, the power calculation is explicitly described in the source as a simplified demonstration rather than the practical three-phase power equation.

### Timing implementation

Distance Zone 2 and Zone 3 use blocking `delay()` calls in the protection logic. This is acceptable for a small demonstration but requires redesign for deterministic, multitask protection firmware.

### Breaker control

The Blynk callbacks directly operate relay outputs. The source itself recommends interlocking, SCADA permissions and local/remote selection for practical systems.

### Prototype settings

Pickup values, impedance thresholds, scaling constants and relay mappings are prototype values. They require protection-study validation, CT/VT scaling validation, coordination review and hardware testing before any real equipment interface.

## Credential handling

The supplied code uses placeholders for Wi-Fi, Blynk and MQTT credentials. Real credentials should never be committed to a public repository.
