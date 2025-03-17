
#ifndef CONSTANTS
  #define CONSTANTS

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

// SPI configuration constants.

#define ADC_LEFT_CS                       2 // GPIO number, audio in left channel chip select, U$2 pin 1
#define ADC_RIGHT_CS                      3 // GPIO number, audio in right channel chip select, U$2 pin 2
#define DAC_CS                            4 // GPIO number, audio out chip select, U$2 pin 3
#define DAC_LDAC                          5 // GPIO number, audio out load, U$2 pin 4
#define SPI_CLK                           8 // GPIO number, SCK, U$2 pin 9
#define ADC_MISO                          9 // GPIO number, MISO, U$2 pin 12
#define DAC_MOSI                         10 // GPIO number, MOSI, U$2 pin 11

// ADC configuration constants.

#define SPI_HOST                          SPI2_HOST  // Identifies the HSPI controller.
#define DMA_CHAN                          0          // DMA not being used.
#define ADC_PRECISION                     2          // The number of bytes per data point.
#define ADC_BITS                          12         // ADC resolution in bits.

// DAC configuration constants.

#define DAC_GAIN                          1          // 1 -> 1x, 0 -> 2x.
#define LEFT                              0
#define RIGHT                             1


// Timer configuration constants.

#define AUDIO_IN_SAMPLES_PER_SECOND       25000
#define AUDIO_OUT_SAMPLES_PER_SECOND      25000
#define SAMPLE_TIMER_DIVIDER              16 // Hardware timer clock divider for audio in and audio out sample timer.
#define SAMPLE_TICKS_PER_SECOND           (TIMER_BASE_CLK / SAMPLE_TIMER_DIVIDER)
#define TICKS_PER_AUDIO_IN_SAMPLE         SAMPLE_TICKS_PER_SECOND / AUDIO_IN_SAMPLES_PER_SECOND
#define TICKS_PER_AUDIO_OUT_SAMPLE        SAMPLE_TICKS_PER_SECOND / AUDIO_OUT_SAMPLES_PER_SECOND

#define AUDIO_IN_ARRAY_INDEX_MAX          255 // Max value of the audio-in samples array index.

#endif
