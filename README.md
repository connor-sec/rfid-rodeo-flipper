# RFID Rodeo (Flipper Zero app)

On-device Flipper → Proxmark3 converter. Reads a saved Flipper RFID dump
(`.rfid`) and prints the equivalent Proxmark3 clone command.

This is the on-device counterpart to the host-side
[RFID Rodeo](https://github.com/connor-sec/RFID_Rodeo) Wiegand parity
calculator. The web tool transforms typed values; this app reads a file the
Flipper already saved and does the same math on-device.

## Scope

**HID H10301 (26-bit) only.** That is the one on-disk mapping that is
firmware-verified. Any other key type (EM4100, Indala26, AWID, Pyramid, ...)
is reported as unsupported rather than guessed, because those formats store
their credential data differently on disk.

## Flow

1. **Pick file** — a file browser opens on `/ext/lfrfid`, filtered to `.rfid`.
2. **Result** — the file is parsed with the firmware `flipper_format` API:
   - `Key type == H10301`, 3-byte `Data`: shows file name, key type, FC, CN,
     the raw 3-byte payload, the 26-bit Wiegand frame hex, and the command
     `lf hid clone -w H10301 --fc <FC> --cn <CN>`.
   - Any other key type: "Unsupported: H10301 only".
   - `Left = Pick` returns to the browser. `Right = Save` (H10301 only).
3. **Saved** — writes the command to `/ext/apps_data/rfid_rodeo/<keyname>.txt`
   and confirms. `Back` returns to the browser.

## H10301 math

```
FC = data[0]                       (0-255)
CN = (data[1] << 8) | data[2]      (0-65535)
```

26-bit Wiegand frame (MSB-first, packed into 7 hex nibbles):
- bit 0: even parity over data bits 1-12 (FC[8] + top 4 bits of CN)
- bits 1-8: FC
- bits 9-24: CN
- bit 25: odd parity over data bits 13-24 (bottom 12 bits of CN)

Worked examples:
- FC=167, CN=34052 → payload `A7 85 04`, frame `14F0A08`,
  `lf hid clone -w H10301 --fc 167 --cn 34052`
- FC=170, CN=43690 → payload `AA AA AA`

## Build

```
./fbt fap_rfid_rodeo
```
Output (default compact config): `build/f7-firmware-C/.extapps/rfid_rodeo.fap`.

## License

AGPL-3.0-or-later. Derived from connor-sec/RFID_Rodeo (v1.4, AGPL-3.0).
See LICENSE.md.
