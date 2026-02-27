# How to upload and run the Laser Harp Arduino firmware (HEX) on an Arduino Mega 2560

This guide explains how to load the precompiled **`.hex`** firmware found in the repository folder **`arduinocode/`** onto an **Arduino Mega 2560**, and how it relates to the Arduino sketch you shared earlier (stepper + laser + LDRs + encoder).

---

## 1) What you need

### Hardware
- **Arduino Mega 2560**
- Stepper driver (for example A4988 / DRV8825 or similar)
- Stepper motor (connected to the driver)
- Laser module (connected to Arduino pin **53** through suitable circuitry if needed)
- 5 light sensors (LDR / photodiodes) wired to analog pins:
  - **A8, A9, A10, A11, A12**
- 5 output LEDs (or connections to a “Daisy”/external system) on digital pins:
  - **17, 18, 19, 20, 21**
- Rotary encoder wired to:
  - **50, 51**
- Power supply suitable for the stepper motor + driver (do **not** power the motor from the Arduino 5V)

### Software (choose one method)
- **Method A (recommended, simplest):** `avrdude`
- **Method B (Windows-friendly):** XLoader (or similar HEX uploader)
- **Method C:** Arduino IDE (only if you have the `.ino` sketch; for HEX you normally use avrdude/XLoader)

---

## 2) Pin mapping 
- Stepper driver:
  - `STEP`  -> **D2**
  - `DIR`   -> **D5**
- Laser:
  - Laser control -> **D53**
- Encoder:
  - `enc1` -> **D50**
  - `enc2` -> **D51**
- Sensors:
  - `ldr1` -> **A8**
  - `ldr2` -> **A9**
  - `ldr3` -> **A10**
  - `ldr4` -> **A11**
  - `ldr5` -> **A12**
- Outputs:
  - `led1` -> **D17**
  - `led2` -> **D18**
  - `led3` -> **D19**
  - `led4` -> **D20**
  - `led5` -> **D21**

Important: our code uses `INPUT_PULLUP` for the LDR pins. That implies our sensor wiring is consistent with this logic (typically you will need a proper divider or conditioning so `analogRead()` produces meaningful values). If the readings look inverted or always high/low, you may need to adjust the sensor circuit or remove the pullup usage and use an external divider.

---

## 3) Locate the HEX file in the repo

In the repository **`PoliTeK/Laser-Harp`**, go to:

- `arduinocode/`  
  and find a file ending with **`.hex`**.

You will upload **that exact `.hex`** to the Arduino Mega 2560.

---

## 4) Upload the HEX to Arduino Mega 2560 (Method A: avrdude)

### 4.1 Install avrdude
- **Windows:** easiest way is to install Arduino IDE (it includes avrdude), or install WinAVR / avrdude package.
- **macOS:** `brew install avrdude`
- **Linux:** `sudo apt install avrdude`

### 4.2 Identify your serial port
- **Windows:** Device Manager → Ports (COM & LPT) → note COM port (e.g. `COM6`)
- **macOS:** typically `/dev/cu.usbmodemXXXX` or `/dev/cu.usbserialXXXX`
- **Linux:** typically `/dev/ttyACM0` or `/dev/ttyUSB0`

### 4.3 Upload command
Arduino Mega 2560 uses:
- MCU: `atmega2560`
- Programmer protocol: `wiring`
- Baud: typically `115200`

Run (edit `PORT` and `FIRMWARE.hex`):

```bash
avrdude -v -p atmega2560 -c wiring -P PORT -b 115200 -D -U flash:w:arduinocode/FIRMWARE.hex:i
```

Examples:

**Windows**
```bash
avrdude -v -p atmega2560 -c wiring -P COM6 -b 115200 -D -U flash:w:arduinocode/laserharp.hex:i
```

**macOS**
```bash
avrdude -v -p atmega2560 -c wiring -P /dev/cu.usbmodem101 -b 115200 -D -U flash:w:arduinocode/laserharp.hex:i
```

**Linux**
```bash
avrdude -v -p atmega2560 -c wiring -P /dev/ttyACM0 -b 115200 -D -U flash:w:arduinocode/laserharp.hex:i
```

If successful, avrdude will report that the flash was written and verified.

---

## 5) Upload the HEX (Method B: XLoader on Windows)

1. Download and open **XLoader**
2. Select the `.hex` file from `arduinocode/`
3. Choose device: **Mega(ATMEGA2560)**
4. Select COM port
5. Click **Upload**

---

## 6) First power-on and verification

### 6.1 Serial Monitor
Your sketch prints the LDR values at **9600 baud**:
- Open Serial Monitor at **9600**
- You should see:
  - `LDRs: v1 v2 v3 v4 v5`

If you see random characters, the baud rate is wrong.

### 6.2 Motion + laser behavior
Expected behavior based on your code:
- The stepper moves **forward** across `corde` segments (currently `corde = 4`)
- At each segment it turns the **laser ON**, reads sensors, sets the LED outputs depending on threshold `k = 100`, then turns the laser OFF
- Then it moves **backward** similarly, turning laser on briefly but without the LED LDR logic in the backward pass

---

## 7) Common problems and fixes

### “avrdude: stk500v2_ReceiveMessage(): timeout”
- Wrong COM/serial port
- Board not recognized / bad cable
- Another program is using the port (close Serial Monitor)
- Wrong baud/protocol (use `-c wiring -b 115200` for Mega2560)

### Stepper moves in the wrong direction
- Swap motor coil wiring or invert direction by flipping `digitalWrite(dirPin, HIGH/LOW)` logic.

### LDR values always high / always low
- Check sensor wiring and ground reference
- `INPUT_PULLUP` on analog pins may not match your circuit expectations
- Confirm that the sensors actually produce an analog voltage into A8–A12

### Laser module doesn’t turn on
- Pin 53 is a digital control signal; most laser modules need proper power and sometimes a transistor/MOSFET driver.
- Do not power a laser module directly from an Arduino pin unless it is explicitly designed for that current and wiring.

---

## 8) If you want to modify the behavior later

- If you only have the `.hex`, you can **upload and run**, but you cannot easily edit logic without the original `.ino/.cpp` sources.
- If you want to change parameters (like `corde`, `k`, timings), edit the Arduino source (the sketch you provided), compile it in Arduino IDE, and upload normally.
