<p align="center"><img src="docs/img/banner.svg" width="100%" alt="Arduino Robotic Arm (2019)"></p>

<p align="center"><img src="https://img.shields.io/badge/Arduino-Mega-00979d"> <img src="https://img.shields.io/badge/Servos-MG995-informational"> <img src="https://img.shields.io/badge/MPU--6050-I2C-informational"> <img src="https://img.shields.io/badge/Bluetooth-HM--10-0082fc"> <img src="https://img.shields.io/badge/C%2B%2B-Arduino-555"> <img src="https://img.shields.io/badge/License-MIT-lightgrey"></p>
<p align="center"><a href="#english">English</a> · <a href="#español">Español</a></p>

## English

### Overview
My first real robotics build: an Arduino-controlled robotic arm, built in 2019 during technical high school by a team of five. It started as a wired arm with four potentiometers that could record and replay a motion, and grew into a wireless master/slave system: a sensor sleeve and glove (flex sensors and MPU-6050 IMUs) streams joint positions over Bluetooth to a second Arduino that drives the servos of a metal arm. It is the direct precedent of my later [PSoC 5LP teach-and-replay arm](https://github.com/simplementeUnPorito/psoc-embedded-projects).

> Firmware transcribed verbatim from the code listings in the original 2019 project report. The photos, figures and video come from that report and its presentation. The sketches were not re-run or re-compiled for this repository.

<p align="center"><img src="docs/img/build-arm-segments.jpg" width="560" alt="Metal arm segments laid out with servos and gripper"><br><sub>Metal arm segments with MG995-class servos and gripper (2019 build)</sub></p>

### Highlights
- Two generations: wired (4 servos, 4 potentiometers, one button) and wireless (up to 11 servos, 6-7 analog sensors, 2 IMUs, HM-10 Bluetooth).
- Record and replay: poses are stored in a matrix; during replay the largest joint travel sets the number of micro-steps so that all servos arrive together, with ramped timing at the start and end of each move.
- IMU angles from accelerometer and gyroscope combined with a complementary filter (0.98 / 0.02), chosen in the report over a Kalman filter because of the Arduino's processing limits.
- The slave validates the Bluetooth string (incomplete or out-of-range values are discarded) before moving the servos.
- The final wired sketch removes almost every `delay()` to fix sluggish behaviour of the first version and needs an Arduino Mega.
- Per the report, the arm replicated the operator's movements over the wireless link; motion calibration was still pending.

### How it worked
- **Controller:** an Arduino on each side (sensor master and arm slave); the Mega for the larger versions.
- **Actuators:** hobby servos driven with the `Servo` library (`writeMicroseconds`, 50 Hz pulse train); MG995 for the main joints.
- **Input:** version 1 used potentiometers as joint knobs; later versions used flex sensors and MPU-6050 IMUs worn on the operator's arm and hand.
- **Modes:** a button toggles between manual (arm follows the sensors, updated every 25 ms) and automatic (replay of the recorded matrix); a switch pauses.
- **Power:** a 5 V / 10 A switching supply for the servos.

### System architecture
Control flow as drawn in the report (original diagrams in `docs/figures/`).

```mermaid
flowchart LR
  subgraph Master["Master: sleeve + glove"]
    S[Flex sensors + MPU-6050] --> M[Map positions] --> TX[Send over HM-10]
    M --> B{Button?}
    B -->|yes| REC[Store pose in matrix]
    REC --> PLAY[Replay: read pose, compute distances and micro-steps, send 1 micro-step]
  end
  TX -. Bluetooth .-> RX
  PLAY -. Bluetooth .-> RX
  subgraph Slave["Slave: arm"]
    RX[Read string] --> V{Read error?}
    V -->|no| SP[Split into joint positions] --> SV[Move servos]
    V -->|yes| RX
  end
```

<p align="center"><img src="docs/figures/flowchart-wireless-master-slave.png" width="640" alt="Report flowchart of the wireless master and slave programs"><br><sub>Report flowchart: wireless master and slave programs.</sub></p>
<p align="center"><img src="docs/figures/flowchart-wired-manual-auto.png" width="640" alt="Report flowchart of manual and automatic modes"><br><sub>Report flowchart: manual (learning) and automatic (replay) modes.</sub></p>

### Hardware
| Component | Role |
|-----------|------|
| Arduino Mega x2 | Master and slave controllers |
| HM-10 Bluetooth x2 | Wireless link |
| MPU-6050 IMU | Arm orientation |
| Flex sensors / potentiometers | Joint input |
| MG995 and smaller servos | Arm joints |
| 5 V / 10 A supply | Servo power |

Full list in [`hardware/`](hardware/README.md).

### Photos
<p align="center"><img src="docs/img/build-arm-segments.jpg" width="360" alt="Metal arm segments laid out with servos and gripper"> <img src="docs/img/build-metal-base-servo.jpg" width="360" alt="Laser-cut metal base frame with servo"></p>
<p align="center"><img src="docs/img/build-metal-frame.jpg" width="360" alt="Metal frame with servo and wiring"> <img src="docs/img/build-wood-prototype.jpg" width="200" alt="Wooden prototype of the arm"> <img src="docs/img/build-wood-joint.jpg" width="200" alt="Wooden wrist/hand joint prototype"></p>

<p align="center"><img src="docs/img/video-system-blocks.jpg" width="720" alt="System blocks slide: control, inputs, outputs, peripherals, communication, movement"><br><sub>Presentation slide: control, information inputs and outputs, peripherals, communication, movement.</sub></p>

### Repository layout
```
firmware/      4 Arduino sketches (wired record/replay, wired IMU, wireless master and slave)
hardware/      component list and construction notes
docs/img/      photos, video stills, GIF, banner
docs/figures/  flowcharts from the report
docs/video/    demo.mp4 (720p re-encode of the 2019 presentation, 1:21)
```

### Getting started
1. Arduino IDE 1.8 or later; install `AltSoftSerial` and `ServoTimer2` (`Servo` and `Wire` are bundled).
2. Open a sketch from `firmware/`. The IMU sketches target an Arduino Mega; AltSoftSerial fixes the Bluetooth pins (the transmitter comments mention 46 RX / 48 TX on a Mega).
3. Configure the HM-10 modules with AT commands (`AT+ROLE`, `AT+CON`, ...), as described in the report, before using the wireless pair.

### What came next
The record-and-replay idea (store poses, replay them smoothly) was rebuilt on a PSoC 5LP with potentiometer knobs, keypad, LCD and EEPROM storage: [psoc-embedded-projects](https://github.com/simplementeUnPorito/psoc-embedded-projects).

### Team & credits
Developed in 2019 by Elías Álvarez, Óscar Araujo, Sara Ramírez, Iván Cristaldo and Adolfo Villalba. Built in 2019 during technical high school.

### License
MIT, see [LICENSE](LICENSE). Report figures and photos are shared for reference.

## Español

### Resumen
Mi primer proyecto real de robótica: un brazo robótico controlado con Arduino, construido en 2019 durante el bachillerato técnico por un equipo de cinco personas. Comenzó como un brazo cableado con cuatro potenciómetros capaz de grabar y repetir un movimiento, y evolucionó a un sistema inalámbrico maestro/esclavo: una manga y un guante con sensores (sensores flexibles e IMU MPU-6050) envían las posiciones por Bluetooth a un segundo Arduino que mueve los servos de un brazo metálico. Es el antecedente directo de mi [brazo PSoC 5LP de enseñar y repetir](https://github.com/simplementeUnPorito/psoc-embedded-projects).

> Firmware transcrito textualmente de los listados de código del informe original del proyecto de 2019. Las fotos, figuras y el video provienen de ese informe y su presentación. Los sketches no se volvieron a ejecutar ni compilar para este repositorio.

### Puntos destacados
- Dos generaciones: cableada (4 servos, 4 potenciómetros, un botón) e inalámbrica (hasta 11 servos, 6-7 sensores analógicos, 2 IMU, Bluetooth HM-10).
- Grabar y repetir: las poses se guardan en una matriz; en la repetición el mayor recorrido define la cantidad de micropasos para que todos los servos lleguen a la vez, con tiempos escalonados al inicio y al final.
- Ángulos de las IMU con filtro complementario (0,98 / 0,02), elegido en el informe frente al filtro de Kalman por la capacidad de procesamiento del Arduino.
- El esclavo valida la cadena Bluetooth (descarta datos incompletos o fuera de rango) antes de mover los servos.
- El sketch cableado final elimina casi todos los `delay()` y requiere un Arduino Mega.
- Según el informe, el brazo replicó los movimientos del operador por el enlace inalámbrico; faltaba calibrar los movimientos.

### Cómo funcionaba
- **Controlador:** un Arduino a cada lado (maestro con sensores y esclavo en el brazo); Mega en las versiones mayores.
- **Actuadores:** servos controlados con la librería `Servo` (`writeMicroseconds`, 50 Hz); MG995 en las articulaciones principales.
- **Entradas:** la versión 1 usaba potenciómetros; las siguientes, sensores flexibles e IMU MPU-6050 sobre el brazo y la mano del operador.
- **Modos:** un botón alterna entre manual (el brazo sigue los sensores, cada 25 ms) y automático (repetición de la matriz grabada); un interruptor pausa.
- **Alimentación:** fuente conmutada de 5 V / 10 A para los servos.

### Arquitectura
Ver el diagrama de flujo en la sección en inglés y las figuras originales del informe en `docs/figures/`.

### Hardware
Lista completa en [`hardware/`](hardware/README.md) (en inglés): 2 Arduino Mega, 2 módulos HM-10, MPU-6050, sensores flexibles, servos MG995 y menores, fuente de 5 V / 10 A.

### Estructura del repositorio
`firmware/` (4 sketches), `hardware/`, `docs/img/` (fotos y cuadros del video), `docs/figures/` (diagramas del informe), `docs/video/demo.mp4` (re-codificación a 720p de la presentación de 2019).

### Cómo empezar
Instalar `AltSoftSerial` y `ServoTimer2` en el IDE de Arduino, abrir un sketch de `firmware/` (Arduino Mega para las versiones con IMU) y configurar los HM-10 con comandos AT antes de usar el par inalámbrico.

### Qué vino después
La idea de grabar y repetir poses se reconstruyó sobre un PSoC 5LP: [psoc-embedded-projects](https://github.com/simplementeUnPorito/psoc-embedded-projects).

### Equipo y créditos
Desarrollado en 2019 por Elías Álvarez, Óscar Araujo, Sara Ramírez, Iván Cristaldo y Adolfo Villalba. Construido en 2019 durante el bachillerato técnico.

### Licencia
MIT, ver [LICENSE](LICENSE). Las figuras y fotos del informe se comparten como referencia.
