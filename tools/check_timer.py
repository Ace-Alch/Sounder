"""Run the original Sounder compare routine and interrupt with mock AVR registers."""

import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def firmware_block(source, signature):
    pattern = r"^" + re.escape(signature) + r" \{\n.*?^\}"
    return re.search(pattern, source, re.MULTILINE | re.DOTALL).group()


def main():
    args = argparse.ArgumentParser(description=__doc__)
    args.add_argument("--cc", default=os.environ.get("CC", "cc"), help="Host C compiler")
    compiler = args.parse_args().cc
    firmware = (ROOT / "Code & Sim" / "main.c").read_text(encoding="utf-8-sig")
    constants = "\n".join(
        re.search(r"^#define " + name + r" \d+", firmware, re.MULTILINE).group()
        for name in ("F_CPU", "MIN_FREQ", "MAX_FREQ")
    )
    compare = firmware_block(firmware, "void set_frequency(uint16_t freq)")
    interrupt = firmware_block(firmware, "ISR(TIMER1_COMPA_vect)")
    program = """
#include <stdint.h>
#include <stdio.h>
#define ISR(name) static void name(void)
#define PB2 2
static uint16_t OCR1A;
static uint8_t PORTB, system_active, paused;
""" + "\n".join([constants, compare, interrupt]) + """
int main(void) {
    const uint16_t requests[] = {50, 123, 440, 1000};
    const uint16_t expected[] = {28927, 65039, 18180, 7999};
    puts("requested_hz,ocr1a,ideal_hz");
    for (unsigned i = 0; i < sizeof requests / sizeof requests[0]; i++) {
        set_frequency(requests[i]);
        if (OCR1A != expected[i]) return 1;
        printf("%u,%u,%.2f\\n", (unsigned)requests[i], (unsigned)OCR1A,
               F_CPU / (2.0 * (OCR1A + 1)));
    }
    TIMER1_COMPA_vect();
    if (PORTB != 0) return 2;
    system_active = 1;
    TIMER1_COMPA_vect();
    if (PORTB != (1 << PB2)) return 3;
    TIMER1_COMPA_vect();
    if (PORTB != 0) return 4;
    paused = 1;
    TIMER1_COMPA_vect();
    return PORTB != 0;
}
"""
    with tempfile.TemporaryDirectory() as temp:
        source = Path(temp) / "timer.c"
        executable = Path(temp) / ("timer.exe" if os.name == "nt" else "timer")
        source.write_text(program, encoding="utf-8")
        subprocess.run([compiler, "-std=c99", "-Wall", "-Wextra", str(source), "-o", str(executable)],
                       check=True, timeout=20)
        subprocess.run([str(executable)], check=True, timeout=5)


if __name__ == "__main__":
    main()
