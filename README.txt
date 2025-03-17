Hardware

The ADCs and DACs use the SPI protocol.  ADCs and DACs run on independent time-bases.  While, initially, they will be synchronized, future applications may require them to run independently.

ADC - The analog-to-digital converters are Texas Instruments ADS7042 12-bit single-channel ADCs with SPI interface.

DAC - The digital-to-analog converter is a Microchip MCP4822 12-bit 2-channel DAC with SPI interface.

The XLR input and output are single-channel, while the TRS input and output are 2-channel.  XLR and TRS do not operate simultaneously.  So, the XLR channel and the TRS tip (or left channel) are combined.

Processor

Computation is performed by a Xiao SEEED dev-board with an Espressif ESP32C3 processor.

Software

The software is based on Midifier C++ code.

The only processing required initially is direct analog-in to analog-out through the processor.

Timers

Use Timer Group 0 with Timer 0 to trigger analog-in and Timer 1 to trigger analog out.

Left and right ADCs are separate SPI devices, whereas the single dual DAC is one SPI device.