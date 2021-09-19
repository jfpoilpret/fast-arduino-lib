# Default fuses configuration for ATmega328 (8MHz)
# - Internal RC clock 8MHz, startup time PWRDWN/RESET 6CK/14CK + 65ms
# - Boot reset vector enabled (no bootloader)
# - Reset enabled
# - SPI programming enabled
# - BOD enabled at 2.7V
# - WDT off
# Internal RC 8MHz, BOD 2.7V, SPI enabled, no bootloader
DEF_LFUSE=0xe2
DEF_HFUSE=0xde
DEF_EFUSE=0x05
# For other frequencies, only the following setting changes:
# - External crystal oscillator (freq 8.0+), startup time PWRDWN/RESET 16CK/14CK +65ms
DEF_LFUSE=0xff

# Default fuses configuration for ATmega644 (8MHz)
# - Internal RC clock 8MHz, startup time PWRDWN/RESET 6CK/14CK + 65ms
# - Boot reset vector enabled (no bootloader)
# - Reset enabled
# - SPI programming enabled
# - BOD enabled at 2.7V
# - WDT off
# Internal RC 8MHz, BOD 2.7V, SPI enabled, no bootloader
DEF_LFUSE=0xe2
DEF_HFUSE=0xd8
DEF_EFUSE=0xfd
# For other frequencies, only the following setting changes:
# - External crystal oscillator (freq 8.0+), startup time PWRDWN/RESET 16CK/14CK +65ms
DEF_LFUSE=0xf7

# Default fuses configuration for ATtiny84 (8MHz)
# - Internal RC clock 8MHz, startup time PWRDWN/RESET 6CK/14CK + 64ms
# - Reset enabled
# - SPI programming enabled
# - BOD enabled at 2.7V
# - WDT off
# Internal RC 8MHz, BOD 2.7V, SPI enabled
DEF_LFUSE=0xe2
DEF_HFUSE=0xdd
DEF_EFUSE=0x01
# For other frequencies, only the following setting changes:
# - External crystal oscillator (freq 8.0+), startup time PWRDWN/RESET 16CK/14CK +65ms
DEF_LFUSE=0xff

# Default fuses configuration for ATtiny85 (8MHz)
# - Internal RC clock 8MHz, startup time PWRDWN/RESET 6CK/14CK + 64ms
# - Reset enabled
# - SPI programming enabled
# - BOD enabled at 2.7V
# - WDT off
# Internal RC 8MHz, BOD 2.7V, SPI enabled
DEF_LFUSE=0xe2
DEF_HFUSE=0xdd
DEF_EFUSE=0xff

