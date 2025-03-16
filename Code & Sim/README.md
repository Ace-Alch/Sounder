# Coding
## The ideal code must have these criteria and functions:
  1.	System ON/OFF Control: The system operates normally when the on/off switch is turned on. If it is turned off, the system remains powered but stops operating. During this time, it waits in a low-power mode until the switch is turned back on. This ensures energy efficiency and readiness to resume operation without a full restart.

  2.	Frequency Reporting to Virtual Terminal: The frequency of the sound is reported to the Proteus virtual terminal. The format of the message is: “Frequency is x Hz”, where x is the current frequency. The values are sent at regular intervals of 2 seconds to provide real-time feedback.

  3.	Frequency Control via Potentiometer: The potentiometer, connected to the ADC pin of the microcontroller, is used to control the sound frequency. The potentiometer’s analog voltage is converted to a digital value using the ADC, and this value is scaled to map the frequency within the range of 50 Hz to 1000 Hz. This enables dynamic and intuitive frequency adjustment.

  4.	Frequency Adjustment via Terminal: The Proteus virtual terminal allows users to fine-tune the frequency by sending a "+" or "-" character. Sending a "+" increases the frequency by 10 Hz, and "-" decreases it by 10 Hz. An additional feature enables users to directly set a desired frequency by typing a numerical value (e.g., 440) in the terminal.

  5.	Smooth LED Blinking with PWM: When the sounder is active, an LED blinks smoothly. The LED's brightness is controlled using PWM (Pulse Width Modulation), where the duty cycle is adjusted based on the current frequency. This creates a visual representation of the sound’s frequency.
