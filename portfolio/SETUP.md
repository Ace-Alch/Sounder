# Sounder: setup

## Current entry point

1. Read `Code & Sim/main.c` and the schematic images.
2. Use an AVR toolchain that targets ATmega328P. Supply `F_CPU=16000000UL` through the build settings before compiling.
3. Check the 16 MHz clock, ISP connector and 5 V supply against the board revision.
4. Program the device with the matching ISP settings.
5. Observe UART at 9600 baud, 8N1, and measure PB2 output frequency.

A verified build script and fuse settings are not provided by the inspected root documentation.

## Required records

- Editable Proteus and PCB source
- BOM and board revision
- Oscilloscope measurements
- Timer range verification

## Reproducibility record

Record the source commit, operating system, tool versions, dependencies, input files, configuration and command. Store the output separately from the expected result. Mark any manual step.
