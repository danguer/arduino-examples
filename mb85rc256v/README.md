# FRAM memory MB85RC256V

This FRAM memory interacts through I2C, the datasheet is at:
https://www.fujitsu.com/uk/Images/MB85RC256V-20171207.pdf

The interaction is very simple, for writing:
1. Set Address (16 bits) (`beginTransmission` and `write`)
2. Write the byte (8 bits) (`write`)

For reading you can just start reading in the current memory address
with `requestFrom` and it will automatically increase the next memory
address.
Or you can set a random address:
1. Set Address (16 bits) (`beginTransmission` and `write`)
2. Read N bytes (`requestFrom`)

The address from the device depends on 3 pins `A2` (MSB), `A1`, and `A0` (LSB)
and can range from `0x50` to `0x57`

## Example

The example is simple, first will write `0xFF` in first address and will
start reducing it, later will read by random address and finally with using
sequential / page read.
The output will be like this:
```
Writing data
Reading back data, by address
Addr: 0 Data: FF
Addr: 1 Data: FE
Addr: 2 Data: FD
Addr: 3 Data: FC
Addr: 4 Data: FB
Addr: 5 Data: FA
Addr: 6 Data: F9
Addr: 7 Data: F8
Addr: 8 Data: F7
Addr: 9 Data: F6
Addr: A Data: F5
Addr: B Data: F4
Addr: C Data: F3
Addr: D Data: F2
Addr: E Data: F1
Addr: F Data: F0
Addr: 10 Data: EF
Addr: 11 Data: EE
Addr: 12 Data: ED
Addr: 13 Data: EC
Reading back data, by page
Addr: 0 Data: FF
Addr: 1 Data: FE
Addr: 2 Data: FD
Addr: 3 Data: FC
Addr: 4 Data: FB
Addr: 5 Data: FA
Addr: 6 Data: F9
Addr: 7 Data: F8
Addr: 8 Data: F7
Addr: 9 Data: F6
Addr: A Data: F5
Addr: B Data: F4
Addr: C Data: F3
Addr: D Data: F2
Addr: E Data: F1
Addr: F Data: F0
Addr: 10 Data: EF
Addr: 11 Data: EE
Addr: 12 Data: ED
Addr: 13 Data: EC
```