# 4. Semesterprojekt – Elektrisches Modellauto (Software)

Arduino-Software für das Modellauto (RWTH Aachen, Institutsprojekt 4. Semester).
Das Auto folgt einem stromdurchflossenen Leiter: zwei Induktionssensoren messen
das Feld, ein **PID-Regler steuert die Lenkung**, ein zweiter **PID-Regler hält
über den Hall-Sensor die Geschwindigkeit** konstant.

Board: **Arduino UNO** (ATmega328P).

---

## 1. Projektstruktur

```
4SP_Auto/
├── 4SP_Auto.ino     Hauptprogramm: setup(), loop(), Regelung, Interrupt
├── config.h         >>> ALLE einstellbaren Werte (Pins, Kalibrierung, Regler)
├── motor.hpp/.cpp   Antriebsmotor über DC/DC-Wandler (PWM)
├── servo.hpp/.cpp   Lenkservo
├── sensor.hpp/.cpp  Induktionssensoren + gleitender Mittelwert
├── pid.hpp/.cpp     PID-Regler (Skript Kapitel 6, Gl. 9)
├── speed_sens.hpp   Geschwindigkeitssensor (Hall, per Interrupt)
│   speed_sense.cpp
├── system.hpp       globale Einstellungen
└── uart.hpp/.cpp    Serielle Befehle (Test & Bedienung)
```

> Fast alles, was ihr an **euer** Auto anpassen müsst, steht in **`config.h`**.

---

## 2. Verwendete Pins (Standard – in `config.h` änderbar)

| Signal                         | Konstante            | Pin  | Typ                       |
|--------------------------------|----------------------|------|---------------------------|
| Lenkservo                      | `PIN_SERVO`          | `9`  | PWM (Servo-Bibliothek)    |
| Motorsteuersignal → DC/DC      | `PIN_MOTOR`          | `5`  | PWM                       |
| Sensor links (Schwingkreis)    | `PIN_SENSOR_LEFT`    | `A0` | Analog-Eingang            |
| Sensor rechts (Schwingkreis)   | `PIN_SENSOR_RIGHT`   | `A1` | Analog-Eingang            |
| Geschwindigkeitssensor (Hall)  | `PIN_SPEED_SENSOR`   | `2`  | externer Interrupt        |

> **Wichtig:** Das sind sinnvolle Standardwerte. Schaut auf euer Auto und passt
> die Pins in `config.h` so an, wie ihr wirklich verdrahtet habt.
> Der Servo darf nicht auf denselben Timer wie das Motor-PWM (Pin 9/10 sind für
> den Servo reserviert), Interrupts gehen beim UNO nur auf Pin 2 oder 3.

---

## 3. Einrichtung (Arduino IDE)

1. **Arduino IDE** öffnen (bereits installiert unter `/Applications/Arduino IDE.app`).
2. Ordner `4SP_Auto/` öffnen: `Datei → Öffnen…` → `4SP_Auto/4SP_Auto.ino`.
   (Die `.hpp/.cpp`-Dateien erscheinen automatisch als Tabs.)
3. Board wählen: **Werkzeuge → Board → Arduino AVR Boards → Arduino UNO**.
4. Die **`Servo`**-Bibliothek ist bei der IDE dabei; falls nicht:
   `Werkzeuge → Bibliotheken verwalten…` → „Servo" installieren.
5. Port wählen (`Werkzeuge → Port`) und mit dem **Upload-Pfeil** hochladen.

### Alternativ über die Kommandozeile (arduino-cli)
```bash
arduino-cli compile --fqbn arduino:avr:uno 4SP_Auto      # nur kompilieren
arduino-cli upload  --fqbn arduino:avr:uno -p <PORT> 4SP_Auto
```

---

## 4. Bedienung über den Serial Monitor

Serial Monitor öffnen, **115200 Baud**, Zeilenende auf **„No Line Ending"**.
Jeder Befehl endet mit `;`, der Wert kommt direkt hinter den Befehl.

| Befehl                | Wirkung                                                        |
|-----------------------|----------------------------------------------------------------|
| `help;`               | Hilfe / Befehlsübersicht anzeigen                              |
| `setServoPos<n>;`     | Servo testen, `n = -100..100` (nur im **Testmodus**)          |
| `setMotorSpeed<n>;`   | Motor testen, `n = 0..255` (nur im **Testmodus**)             |
| `toggleAnalogPlot;`   | Sensor-Rohwerte für den **Serial Plotter** an/aus             |
| `startDrive;`         | **Autonomes Fahren starten**                                   |
| `stopDrive;`          | Autonomes Fahren stoppen → zurück in den Testmodus            |

Beim Einschalten startet das Auto **im Testmodus** (fährt also nicht von selbst
los). Erst `startDrive;` aktiviert die Regelung.

---

## 5. Einstellen / Kalibrieren (Reihenfolge)

1. **Servo:** `toggleAnalogPlot` aus, im Testmodus `setServoPos-100;`, `setServoPos0;`,
   `setServoPos100;` senden und `SERVO_FULL_LEFT`, `SERVO_CENTER`, `SERVO_FULL_RIGHT`
   in `config.h` so einstellen, dass die Räder ganz links / gerade / ganz rechts stehen.
2. **Motor:** `setMotorSpeed40;` usw. – kleinsten Wert finden, bei dem der Motor
   sicher anläuft → `MOTOR_MIN_SPEED`. Höchsttempo → `MOTOR_MAX_SPEED`.
3. **Sensoren:** `toggleAnalogPlot;` einschalten, Auto über den Leiter halten und im
   **Serial Plotter** die Werte beobachten. Den Schwellwert `OFF_TRACK_THRESHOLD`
   knapp über den Wert setzen, den die Sensoren OHNE Leiter anzeigen.
4. **Lenkung (PID):** `startDrive;`, dann `STEER_KP` erhöhen bis das Auto dem Leiter
   folgt; `STEER_KD` gegen Schwingen; `STEER_KI` meist 0.
   Lenkt es falschherum → `STEER_SIGN` auf `-1`.
5. **Geschwindigkeit (PID):** `TARGET_SPEED` klein starten, `SPEED_KP`/`SPEED_KI`
   so einstellen, dass das Tempo konstant bleibt.

### Faustregeln zum Reglertuning (aus dem Skript, Tab. 6.4)

| Parameter erhöhen | Anstiegszeit | Überschwingen | Regelabweichung |
|-------------------|--------------|---------------|-----------------|
| `Kp`              | kleiner      | größer        | kleiner         |
| `Ki`              | kleiner      | größer        | verschwindet    |
| `Kd`              | wenig        | kleiner       | keine Änderung  |

Vorgehen: erst `Kp` (Anstieg), dann `Kd` (Überschwingen/Einschwingen), dann `Ki`
(bleibende Abweichung).

---

## 6. Wie die Regelung funktioniert (Kurz)

- **`loop()`** läuft in einem festen Takt `CONTROL_PERIOD_MS` (= Abtastzeit *T*).
- **Lenkung:** `error = links − rechts`, Sollwert `0` (Leiter mittig). Der PID
  gibt einen Lenkbefehl `−100..+100` aus → `servo_set_position()`.
- **Off-Track:** sehen beide Sensoren nichts (< `OFF_TRACK_THRESHOLD`), hält das
  Auto die letzte Lenkrichtung und fährt zum Leiter zurück.
- **Geschwindigkeit:** der Hall-Sensor löst pro Umdrehung einen **Interrupt** aus;
  aus dem zeitlichen Abstand wird die Geschwindigkeit berechnet. Der PID stellt
  daraus das Motor-PWM → konstante Geschwindigkeit trotz sinkender Akkuspannung.
- **PID:** diskrete Form aus dem Skript, Gl. 9:
  `u(k) = Kp·e(k) + Ki·T·Σe + Kd·(e(k)−e(k−1))/T`.
