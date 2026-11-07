# Firmware

Firmware transcribed verbatim from the code listings in the original 2019 project report. Comments are the original Spanish. No logic was modified.

| Folder | Board | What it does |
|--------|-------|--------------|
| `wired-record-playback/` | Arduino (Mega, Uno, Nano class) | 4 servos on pins 3, 5, 10, 11 driven by 4 potentiometers (A0-A3). One button (pin 6) records a pose; a double press starts replay; a switch (pin 4) pauses. |
| `wired-imu-glove/` | Arduino Mega | Final wired version: 11 servos (pins 3-13), analog inputs A0-A6 and 2 MPU-6050 IMUs over I2C, same record/replay logic with `delay()` removed almost everywhere. |
| `wireless-transmitter/` | Arduino Mega | Master: reads sensors, computes positions and sends them over an HM-10 Bluetooth link (AltSoftSerial). |
| `wireless-receiver/` | Arduino on the arm | Slave: parses the Bluetooth string, rejects malformed data and drives the servos (ServoTimer2). |

Libraries: `Servo`, `Wire`, `AltSoftSerial`, `ServoTimer2` (third-party, not included).

## Build status

Build status (verified 2026-11-06 with PlatformIO, atmelavr toolchain 7.3.0, Servo 1.3.0, AltSoftSerial 1.4.0, ServoTimer2 alpha):
- `wired-record-playback/` (megaatmega2560): compiles.
- `wired-imu-glove/` (megaatmega2560): compiles.
- `wireless-receiver/` (uno): compiles.
- `wireless-transmitter/` (megaatmega2560): does not compile as archived. The listing is truncated: the file ends inside the main loop with 2 closing braces missing (41 `{` vs 39 `}`), error "expected '}' at end of input" at line 567.

Estado de compilacion (verificado 2026-11-06 con PlatformIO, toolchain atmelavr 7.3.0, Servo 1.3.0, AltSoftSerial 1.4.0, ServoTimer2 alpha):
- `wired-record-playback/` (megaatmega2560): compila.
- `wired-imu-glove/` (megaatmega2560): compila.
- `wireless-receiver/` (uno): compila.
- `wireless-transmitter/` (megaatmega2560): no compila tal como esta archivado. El listado esta truncado: el archivo termina dentro del bucle principal y faltan 2 llaves de cierre (41 `{` frente a 39 `}`), error "expected '}' at end of input" en la linea 567.
