# Schematic Design
___________________________________________________
The circuit diagram is composed of several logical smaller circuits. Below are the components used in this project:
  1. LED: which can be set in High side or Low side.
  2. On/Off Switch: can be a 2-position switch or a tactile switch.
  3. Terminal Connection: TXD->RXD  &  RXD->TXD  :)
  4. Sounder: using PWM outputs or digital I/O pin.
  5. Potentiometer: connects to AD-Converter,  and GND as reference point.
  6. RESET: better to use a tactile switch
  7. ISP Connector: it is needed for PCB designing, not for simulation.
  8. Crystal Oscillator
  9. Power Circuitry
  10. By-Pass Capacitors: A good rule of thumb for digital ICs is to add a 100nF by-pass capacitor for each power
      pin pair and one larger ~10uF capacitor for each IC. Our microcontroller has two VCC and GND pairs and one
      AVCC and GND pair. It is also recommended to add a capacitor between AREF and GND. So in total 4 100nF
      capacitors and one 10uF capacitor would be good.
  11. Serial Communication 1x4 pin header: which could be used to connect an external terminal
________________________________________________________________
