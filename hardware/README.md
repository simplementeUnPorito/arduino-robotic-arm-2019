# Hardware

Component list as reported in the 2019 project report (budget table and circuit description). Prices are omitted.

| Qty | Component | Role |
|-----|-----------|------|
| 2 | Arduino Mega (mini form factor, ATmega2560-16AU) | Master (sensor side) and slave (arm side) controllers |
| 2 | HM-10 Bluetooth module | Wireless link between master and slave |
| 3 chips listed | MPU-6050 IMU | Accelerometer + gyroscope for arm orientation, fused with a complementary filter (0.98 gyro + 0.02 accelerometer) |
| 8 | Flex ("resistive") sensors, 4.5 in | Finger/joint bend on the glove |
| 8 | MG995 servos (4 "speed" + 4 "angle" versions) | Arm joints |
| 5 | Servos listed as "5690" | Smaller joints/fingers |
| 1 | Character LCD with I2C backpack, DFPlayer Mini MP3 module | Status messages and audio feedback for the operator |
| 1 | 5 V / 10 A switching supply | Powers the servos (the report cites 12 servos on the slave circuit) |
| 1 | 3 A / 5 V battery-charge module | Portable power |

## Construction
- Slave circuit mounted on a metal robotic arm; master circuit mounted on a sleeve and glove worn by the operator, with sensors placed on the user's joints.
- Servos take 4.8-6 V and a 50 Hz pulse train, driven through the `Servo` library with `writeMicroseconds()`.
- Early prototypes used a wooden structure (see `docs/img/build-wood-*.jpg`).
- The report notes the circuits were tested on the arm prototype: the arm replicated the user's movements and the wireless link worked; only the calibration of movements was still pending.
