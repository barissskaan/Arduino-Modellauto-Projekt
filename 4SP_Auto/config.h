/////////////////////////////////////////////////////////////////////////////
//  config.h  -  Zentrale Konfiguration des Modellautos
//  ------------------------------------------------------------------------
//  ==> HIER stehen ALLE Werte, die ihr eventuell an euer echtes Auto
//      anpassen muesst (Pins, Kalibrierung, Reglerparameter).
//  ==> Wenn ihr etwas anders verdrahtet oder eingebaut habt, aendert ihr
//      NUR diese Datei - sonst muss der restliche Code nicht angefasst werden.
//
//  This header collects every value you may have to adapt to your real car
//  (pin assignment, calibration, controller gains). If your wiring differs,
//  you only edit THIS file.
/////////////////////////////////////////////////////////////////////////////
#pragma once

// ===========================================================================
//  1) PIN-BELEGUNG  (Board: Arduino UNO / ATmega328P)
// ===========================================================================
//  Lenkservo  -> PWM-faehiger Pin. Uno: 3, 5, 6, 9, 10, 11
#define PIN_SERVO            9
//  Motorsteuersignal (PWM) -> Eingang IN des DC/DC-Wandlers (Halbbruecke)
//  Achtung: NICHT Pin 9/10 benutzen, die belegt die Servo-Bibliothek.
#define PIN_MOTOR            5
//  Linker  Sensorverstaerker (Schwingkreis)  -> Analog-Eingang
#define PIN_SENSOR_LEFT      A0
//  Rechter Sensorverstaerker (Schwingkreis)  -> Analog-Eingang
#define PIN_SENSOR_RIGHT     A1
//  Geschwindigkeitssensor (Hall DRV5011) -> externer Interrupt-Pin. Uno: 2 oder 3
#define PIN_SPEED_SENSOR     2

// ===========================================================================
//  2) LENKSERVO  -  Kalibrierung (Werte in Mikrosekunden, typ. 1000..2000)
// ===========================================================================
//  Am realen Servo einmessen: im Serial Monitor z.B. "setServoPos-100;" (links),
//  "setServoPos0;" (Mitte), "setServoPos100;" (rechts) senden und die drei
//  Werte hier so einstellen, dass die Raeder genau ganz links / mittig / ganz
//  rechts stehen. (Manueller Modus, siehe README.)
#define SERVO_FULL_LEFT      1200   // Vollausschlag links
#define SERVO_CENTER         1500   // Geradeaus
#define SERVO_FULL_RIGHT     1800   // Vollausschlag rechts

// ===========================================================================
//  3) ANTRIEBSMOTOR  -  PWM-Tastgrad 0..255 (0 = aus)
// ===========================================================================
#define MOTOR_MIN_SPEED      40     // ab diesem PWM-Wert dreht der Motor (Anfahren)
#define MOTOR_MAX_SPEED      180    // obere Begrenzung (Sicherheit / max. Tempo)

// ===========================================================================
//  4) LENK-REGLER (PID)  -  haelt das Auto ueber dem Leiter/Draht
//     Stellgroesse = Lenkbefehl -100 (ganz links) .. +100 (ganz rechts)
// ===========================================================================
#define STEER_KP             0.12   // P-Anteil: staerker = schnelleres Lenken
#define STEER_KI             0.0    // I-Anteil: gegen bleibenden Versatz (meist 0)
#define STEER_KD             0.06   // D-Anteil: daempft/beruhigt das Lenken
#define STEER_ANTIWINDUP     150.0  // Begrenzung des Integrators
//  Vorzeichen der Lenkung. Lenkt das Auto in die FALSCHE Richtung: auf -1 setzen.
#define STEER_SIGN           (+1)
//  Mittelpunkt-Abgleich: der Diff-Wert (Left-Right), den die Sensoren zeigen,
//  wenn der Draht GENAU mittig unter dem Auto liegt. Im Serial Monitor ablesen
//  und hier eintragen. Der Lenkregler nimmt diesen Wert als "geradeaus".
//  (Gemessen: Left~362 / Right~378 -> Diff ~ -15)
#define STEER_CENTER_OFFSET  (-15)

// ===========================================================================
//  5) GESCHWINDIGKEITS-REGLER (PID)  -  haelt konstante Geschwindigkeit
//     Stellgroesse = Motor-PWM 0..255
// ===========================================================================
#define SPEED_KP             4.0
#define SPEED_KI             0.8
#define SPEED_KD             0.0
#define SPEED_ANTIWINDUP     255.0  // Integrator bis max. PWM

// ===========================================================================
//  6) SYSTEM / FAHRVERHALTEN
// ===========================================================================
//  Sollgeschwindigkeit fuer den Speed-Regler, Einheit: Impulse/Sekunde
//  (= Umdrehungen/s, siehe SPEED_PULSE_SCALE). Klein anfangen, dann erhoehen.
#define TARGET_SPEED         20
//  Schwelle "vom Kurs abgekommen": liegen BEIDE Sensorwerte darunter, sieht das
//  Auto den Leiter nicht mehr und haelt die zuletzt gefahrene Lenkrichtung.
#define OFF_TRACK_THRESHOLD  60
//  true = Sensorwerte (Left/Right/Diff) staendig ausgeben - zum Einstellen /
//  Kalibrieren im Serial Monitor. Fuers normale Fahren wieder auf false setzen.
#define PLOT_ANALOG          true

// ===========================================================================
//  7) REGLER-TAKT & GESCHWINDIGKEITSMESSUNG
// ===========================================================================
//  Abtastperiode T des Reglers in ms (fester loop-Takt). Konstant halten,
//  denn I- und D-Anteil haengen von T ab (siehe Skript, Gl. 9).
#define CONTROL_PERIOD_MS    10UL
//  Flanke des Hall-Sensors, an der ein Impuls gezaehlt wird: RISING/FALLING/CHANGE
#define SPEED_SENSOR_EDGE    RISING
//  Umrechnung gemessene Periode -> Geschwindigkeit:  speed = SCALE / dt_in_us
//  1e6 => Ergebnis in Impulsen/Sekunde (bei 1 Magnet = 1 Impuls je Umdrehung).
#define SPEED_PULSE_SCALE    1000000.0
//  Kommt laenger als diese Zeit kein Impuls, gilt: Auto steht (Geschw. = 0).
#define SPEED_TIMEOUT_US     300000UL   // 0,3 s
