# ESL Blaster

USB IR transmitter firmware and hardware design files.

Sometimes available for purchase on [Tindie](https://www.tindie.com/products/furrtek/esl-blaster).

## Installation

For Linux and Windows 7 you might have to download the [STM32 VCP driver](https://www.st.com/en/development-tools/stsw-stm32102.html). Windows 8 and up should install the correct driver automatically.

The device will simply appear as a Virtual COM Port. As there's no real COM Port interface, speed and data format doesn't matter.

## Firmware update

Short the two solder pads on the back:
![ESL Blaster bootloader pads](jp_boot.png)

Plug the device in and use `dfu-util-0.9-win64` with `FWxx\Release\program.bat` to update the firmware.

Don't forget to disconnect the pads once done.

* FW01: Basic functionality
* FW02: PP16 high speed protocol support, more storage for remote mode frames (14 instead of 8)

## Using a non-rechargeable (CR2032) battery

Disconnect the two solder pads next to the push-button to disable the charging circuit:
![ESL Blaster charger pads](jp_charge.png)

## Protocol info

This is the top-level serial protocol info to communicate with the ESL Blaster, not the ESL IR protocol itself.

Commands are a single character (byte):
* `?`: Ask for ID string. Replies with `ESLBlasterXY`. X is the hardware revision (B), Y is the firmware version minus 1 (currently 0 or 1).
* `L`: Load frame data. Format: `LsdrRx...`. s: frame size in bytes. d: delay between repeats. r: repeat count low byte. R: repeat count high byte. x: frame data...
  * In FW02, the highest bit of R is used to indicate PP16 transmission. Repeat max count is therefore reduced to 32767 instead of 65535.
* `T`: Transmit loaded frame. Returns `K` when done.
* `S`: Emergency stop. Stops any IR transmit operation.
* `W`: Start user flash write sequence. Send 1024 bytes in 128 byte blocks, wait for `K` (ok) or `N` (error) reply after each block, and one last reply for program ok/fail.
* `R`: Read user flash (starts at 0x8007C00). Replies with 512 bytes.
