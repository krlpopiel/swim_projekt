# 🚗 STM32-Based Vehicle

Semester project for **Embedded Systems and Microcontrollers** Authors: Adrian Popielarczyk, Zuzanna Orzechowska

Student IDs: 21295, 21284

Start date: May 3, 2025

The repository contains code, documentation, and project materials.

---

## 📌 Project Description

The robot can move in two modes – **automatic** (follows any black trace on a white background - line follower) and **manual** (movement is controlled via a remote control and an infrared module). The robot features an LCD screen capable of displaying text and faces – its expression changes along with the driving direction in automatic mode. It performs these tasks using external buttons.

The robot has one pair of axles. The axle is not steerable (fixed), and traction/drive is applied to the rear axle. Steering resembles the movement scheme of crawler vehicles like a tank (skid-steering), with the difference that wheels are used instead of tracks. The robot has a single power source – for both the motors and the STM32 board – consisting of six 1.6V batteries. During program initialization (robot startup), a lively melody is played via a passive buzzer.

---

## 🛠️ Technologies and Tools

* **Microcontroller:** STM32 NUCLEO F411RE
* **IDE:** STM32CubeIDE
* **Programming:** C (HAL / LL)
* **Sensors:**
* IR Receiver Sensor 38 kHz
* Optical Sensors (IR) TCRT5000


* **Power Supply:** 6x 1.6V Batteries
* **Motor Driver:** L298n
* **Communication:** UART (Infrared module)

---

## ⚙️ Functionalities

* ✅ PWM-controlled drive using Timers
* ✅ Ultrasonic sensor support (distance measurement)
* ✅ Reading IR sensor values (line/obstacles) using ADC
* ✅ Melody playback during program initialization
* ✅ Movement control via UART (Remote with IR module)
* ✅ Battery powered – full autonomy
* ✅ LCD display showing facial expressions - nose and eyes

---

## 📁 Repository Structure

Three branches showing project progress. New releases were created along with new code content (containing a zip package). Currently, the latest is **release v3.1** – complete manual and automatic driving functionality, melody playback, and LCD display.

---

## 🧪 Test Scenarios

* [x] Reaction to white/black line (IR)
* [x] Communication via infrared module
* [x] Battery power test
* [x] Real-time movement control

---

## 📸 Demo and Photos

* Photos of the vehicle as well as recordings are located in the folder:
[https://mega.nz/fm/jtxTxCZR](https://mega.nz/fm/jtxTxCZR)

---

## 📄 Documentation

Full project documentation is located in the folder:
[https://mega.nz/fm/jtxTxCZR](https://mega.nz/fm/jtxTxCZR)
including:

* Final Report (PDF)
* Milestone Reports

---

## 📅 Work Schedule

* **May 03 - May 13:** designing basic sequences, robot assembly, construction report, 3D robot project, mechanical engineering
* **May 14 - May 27:** programming additional sequences, programming basic sequence, construction report
* **May 28 - June 03:** adding additional functionalities - passive buzzer and LCD display
* **June 10:** final presentation of the robot

---

## 🧠 Conclusions

The robot flawlessly traverses the route along the line and can be efficiently operated via the remote control with the infrared module. Additionally, adding the LCD display, which shows the appropriate face corresponding to the robot's driving direction in automatic mode, proved to be a success. The passive buzzer signals that the entire program is working, and the robot properly initializes the program and begins driving.

---

## 📬 Contact

If you have any questions:

* Email: 21295@student.ans-elblag.pl, 21284@student.ans-elblag.pl
* GitHub: krlpopiel, zuzanna-orzechowska

---

**License:** MIT
