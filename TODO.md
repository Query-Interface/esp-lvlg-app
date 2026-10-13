[ ] check lgvl and sound do not use the same cpu
[ ] check memory management
[ ] Issue with audio power
---
That confirms it's power. At volume 50, the loud passage around 33 s made the amplifier draw enough current to drop the supply voltage, and the SD card glitched. At volume 10 the
  amplifier draws much less, so the rail holds up. The software side is fine.

  Hardware fixes, most effective first:
  1. Power the MAX98357A from 5 V (USB or VBUS) instead of 3.3 V. Its VIN accepts 2.5–5.5 V, and this keeps its current spikes off the 3.3 V regulator that feeds the ESP32 and the SD
     card. The I2S signals can stay at 3.3 V logic, so no level shifting is needed.
  2. Add a bulk capacitor at the amplifier: 220–470 µF electrolytic plus 100 nF ceramic between VIN and GND, as close to the board as possible.
  3. Decouple the SD card: 10 µF plus 100 nF at its VCC pin.
  4. Shorten the power and ground wires to the amplifier, and use thicker ones. Breadboard jumpers drop a noticeable amount of voltage at ~1 A.
  5. Optionally reduce the amplifier gain using the GAIN pin (left floating, the default is 9 dB; tied to GND it's 12 dB, so check the datasheet table). Lower gain plus higher software
     volume means smaller current peaks.

  How is the amplifier powered right now: from the board's 3V3 pin, or from 5V/VIN? That determines which fix matters most.

  Until then: keep the volume around 10–20. Once the hardware is fixed, I can also add the retry-on-read-error wrapper, so a single SD glitch causes a tiny skip instead of stopping
  playback.

  Status of the earlier issues:
  - Fast or choppy sound: fixed (I2S write timeout).
  - WiFi out-of-memory crash: fixed (WiFi starts first, ALWAYSINTERNAL set to 4096).
  - NULL timer crash: fixed (guards on lv_timer_ready).
  - Internal RAM is still tight (~17 KB "min ever"). The WiFi middle-ground settings are still open if you want them.
---