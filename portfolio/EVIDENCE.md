# Sounder: evidence register

| Evidence | Origin | Strength |
| :-- | :-- | :-- |
| Project purpose, stated status and technology list | User-supplied project list, 17 November 2026 | Owner statement; no new execution |
| Repository file inventory | [Ace-Alch/Sounder](https://github.com/Ace-Alch/Sounder), main | Paths and blob identifiers inspected |
| Previous README | `README-before-rebuild.md` | Historical documentation; claims require the listed checks |

## Evidence limits

The source requests 50�1000 Hz and defaults to 440 Hz. Timer1 uses no prescaler and a 16-bit compare register. At 16 MHz, a 50 Hz target requires 159999, which exceeds 65535. Thus the full requested range is not established by this implementation. Verify the output with a scope before claiming the range.

## Missing material

- Editable Proteus and PCB source
- BOM and board revision
- Oscilloscope measurements
- Timer range verification
