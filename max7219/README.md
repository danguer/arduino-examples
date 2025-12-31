# MAX7219 Example (8 digit segment driver)
The MAX7219 is a 8 digit segment driver, it can also work as standard 8x8 LED driver, but since there is BCD support, it works much better with 7-segment parts.

The board I used have 8 segments and the MAX7219

The inputs are just Power(`GND`, `VCC`), `DIN` (Serial Data Input), `CLK` and `LOAD/CS`
Even it looks like a SPI module, only the MAX7221 supports it, so the input for MAX7219 is serial and is simple to use.

It needs to be sent 16 bit data which are split into lower 8 bits for the data and higher 8 bits for register.

The datasheet is very clear and helpful about all parts, my only complain is the functional diagram suggest the data is loaded with LSB first which is not the case.

For the address, there are few commands:

* Digit 0-7 data (`0x1` to `0x8`)
* Decode mode `0x9` (Disable / Enable BCD for each digit)
* Intensity `0xA` (16  levels of intensity going from `0x0` to `0xF`)
* Scan Limit `0xB` (Disable / Enable digit output)
* Shutdown `0xC` (Disable / Enable output)
* Display Test `0xF` (Disable / Enable test mode).

The `Display Test` just enable all segments and all digits at full intensity.
For `Shutdown` if data is `0x1` will enable output (TBH a bit odd to use that naming)

## Register: Scan Limit
The `Scan Limit` allows to how many digits to be display, so it can be up to `0x7` (to display 8 digits). As note, the reset value of `0x0` means that will show first digit; if you want to disable all output then you need to use the `Shutdown` command, or just set the first digit to `0x0` (no segment on) for `No Decode` mode, or `0xF` for `BCD` mode

## Register: Digit
The `Digit` data will store 8 bits for the specific digit, the data can be stored in two ways:
* Standard (No BCD / No Decode)
* BCD

In the Standard, you need to select through bits which part to show from the 7-segment, so for example to display a `1` you can store `0x30` or `D5=1,D4=1` (`B=1,C=1`)

In the BCD mode, the value from `0x0` to `0x9` (or `0-9`) will be converted into the proper segments on, so if a `0x1` is stored, the driver will properly convert to show segments `B=1,C=1`.
There are some special characters going from `0xA` to `0xF`:
* `0xA = dash(-)`
* `0xB = E`
* `0xC = H`
* `0xD = L`
* `0xE = P`
* `0xF` blank, so no output, the same as `0x0` in `No Decode Mode`

(Yes you can send a `HELP` message with `0x040C 0x030B 0x020D 0x010E`

## Register: Decode Mode
The `Decode Mode`register allows to use the `No Decode/BCD` mode, and the register is a bit interesting since is a mask, if the value is `0x0` means no decode so the data are just the segment to set on.
But if you want a specific digit to use BCD, you can use as mask, so for example to enable BCD on:
* Digit 0 `0x1`
* Digit 1 `0x2`
* Digit 0 and 1 `0x3`

## Register: Loading Data

The loading data is very simple, when you rise the `CLK` it will read the `DIN` and add to serial buffer (`MSB` to `LSB` order).
So sending a bit is very simple:
```
digitalWrite(PIN_CLK, LOW);
// check if MSB bit is on
if (cmd & 0x8000) {
    digitalWrite(PIN_DIN, HIGH);
} else {
    digitalWrite(PIN_DIN, LOW);
}
digitalWrite(PIN_CLK, HIGH);
```

And to send a 16 bit value:
```
// enable test mode
// 0x0F is test mode register
// 0x01 is enable data
uint16_t cmd = 0x0F01;
for (int i = 0; i < 16; i++) {
    digitalWrite(PIN_CLK, LOW);
    // check if MSB bit is on
    if (cmd & 0x8000) {
        digitalWrite(PIN_DIN, HIGH);
    } else {
        digitalWrite(PIN_DIN, LOW);
    }
    digitalWrite(PIN_CLK, HIGH);
    // shift one bit
    cmd <<= 1;
}
```

In the case of the MAX7219 when `LOAD` rises, it will load the last 16 bits from the `DIN` input into the decoder and run the command.
So this is achieved after sending the data:
```
// latch the data
// rise the LOAD pin from low to high and go back into low after that
digitalWrite(PIN_LOAD, LOW);
digitalWrite(PIN_LOAD, HIGH);
digitalWrite(PIN_LOAD, LOW);
```

## Commands
Everything is a 16 bit data, so there is a helper function to build the data:
```
uint16_t buildCmd(uint8_t registerAddress, uint8_t data)
{
    return ((uint16_t)registerAddress << 8) | data;
}
```

Note that this can be converted into a macro if is needed better performance.

So the commands are very easy to build, for example to send a number 6 to digit 0 (using BCD):
```
// 0x1 is the register for digit 0
uint16_t cmd = buildCmd(0x1, 6);
// this is the function using code previously
// posted to send data to module
sendCmd(cmd);
```
Or if not using BCD:
```
// 0x1 is the register for digit 0
// 6 uses A,C,D,E,F,G so is
// 0101 1111 or 0x5F
uint16_t cmd = buildCmd(0x1, 0x5F);
// this is the function using code previously
// posted to send data to module
sendCmd(cmd);
```

## Example
The example uses the test mode, but later sets all the registers to "reset" most of them, note that this is not really needed if you only reset the module and never use the test mode.
But I think is good to have it so you can configure everything before use:
```
cmdSetScanLimit(7); // Enable all 8 digits
cmdSetIntensity(0); // lowest intensity
cmdSetDecodeMode(0xFF); // all digits will use BCD

// set all segments to blank
for (uint8_t i = 0; i < 8; i++) {
  uint16_t cmd = buildCmd(REGISTER_DIGIT0 + i, 0xF);
  sendCmd(cmd);
}

// send all the possible values on BCD, finishing with blank
for (uint8_t i = 0; i < 8; i++) {
  for (uint8_t j = 0; j < 16; j++) {
    uint16_t cmd = buildCmd(REGISTER_DIGIT0 + i, j);
    sendCmd(cmd);
    delay(200);
  }
}

```

## Datasheet
https://www.analog.com/media/en/technical-documentation/data-sheets/max7219-max7221.pdf