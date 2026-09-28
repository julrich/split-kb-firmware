# Sofle rev1 / Miryoku
BOOTLOADER = atmel-dfu
USER_NAME = manna-harbour_miryoku

RGBLIGHT_ENABLE = yes
ENCODER_ENABLE = yes
OLED_ENABLE = yes
OLED_DRIVER = ssd1306
LTO_ENABLE = yes

# Trim core features that are not used (flash is tight on the ATmega32u4).
AUDIO_ENABLE = no
MAGIC_ENABLE = no
SPACE_CADET_ENABLE = no
GRAVE_ESC_ENABLE = no
