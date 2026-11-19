# RFID Rodeo (Flipper Zero app)

On-device Flipper → Proxmark3 converter. Reads a saved Flipper RFID dump
(`.rfid`) and prints the equivalent Proxmark3 clone command.

Built for [Momentum](https://momentum-fw.dev/) firmware **mntm-012**, not stock
Flipper firmware. See [Firmware](#firmware) below.

This is the on-device counterpart to the host-side
[RFID Rodeo](https://github.com/connor-sec/RFID_Rodeo) Wiegand parity
calculator. The web tool transforms typed values; this app reads a file the
Flipper already saved and does the same math on-device.

## Scope

**HID H10301 (26-bit) only.** That is the one on-disk mapping verified on
Momentum (mntm-012). Any other key type (EM4100, Indala26, AWID, Pyramid, ...)
is reported as unsupported rather than guessed, because those formats store
their credential data differently on disk.

## Flow

1. **Pick file** — a file browser opens on `/ext/lfrfid`, filtered to `.rfid`.
2. **Result** — the file is parsed with the firmware `flipper_format` API.
   It must be a Flipper RFID key file: header `Filetype: Flipper RFID key`
   and `Version: 1`.
   - `Key type == H10301` and `Data` is exactly 3 bytes: shows file name, key
     type, FC, CN, the raw 3-byte payload, the 26-bit Wiegand frame hex, and
     the command `lf hid clone -w H10301 --fc <FC> --cn <CN>`.
   - A shorter or longer `Data` field is rejected. The app does not keep the
     first 3 bytes of a longer field.
   - Any other key type: "Unsupported: H10301 only".
   - `Left = Pick` returns to the browser. `Right = Save` (H10301 only).
3. **Saved** — writes the command under `/ext/apps_data/rfid_rodeo/`. The file
   name is the path relative to `/ext/lfrfid` (slashes become underscores), so
   `Office/card.rfid` and `Home/card.rfid` do not overwrite each other. If
   that name already exists, a numeric suffix is added (`card1.txt`,
   `card2.txt`, ...) instead of truncating the existing file. `Back` returns
   to the browser.

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

## Firmware

This app targets Momentum **mntm-012** (mainline stable):

- https://momentum-fw.dev/releases/mntm-012
- https://github.com/Next-Flip/Momentum-Firmware/releases/tag/mntm-012

Copy this folder into a Momentum tree at `applications_user/rfid_rodeo`.

## Build

From the root of a full Momentum **mntm-012** firmware tree:

```
./fbt fap_rfid_rodeo
```

With the default compact config the FAP is
`build/f7-firmware-C/.extapps/rfid_rodeo.fap`.
Those paths are for a Momentum firmware tree, not a stock Flipper checkout
and not a standalone ufbt build.

### Optional: ufbt

[ufbt](https://pypi.org/project/ufbt/) can build this app without the rest of
the firmware tree. Point it at Momentum's SDK index. The `release` channel of
that index is mntm-012:

```
python3 -m pip install --upgrade ufbt
ufbt update --index-url https://up.momentum-fw.dev/firmware/directory.json --channel release
ufbt
```

The FAP is written to `dist/rfid_rodeo.fap`. If the release channel later
moves past mntm-012, pin this SDK directory instead (`--index-url` is the
branch root in branch mode):

```
ufbt update --index-url https://up.momentum-fw.dev/builds/firmware --branch mntm-012
```

## Host test

The H10301 frame math has no Flipper dependency. From this directory:

```
gcc -std=c11 -Wall -Wextra -Werror -o test_h10301 tests/test_h10301.c rfid_rodeo_h10301.c
./test_h10301
```

That test is not linked into the FAP (`application.fam` lists the app
sources explicitly).

## License

AGPL-3.0-or-later. Derived from connor-sec/RFID_Rodeo (v1.4, AGPL-3.0).
The full license text is in `LICENSE`. Attribution is in `LICENSE.md`.
