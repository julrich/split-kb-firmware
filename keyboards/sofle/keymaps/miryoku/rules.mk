# Sofle rev1 / Miryoku
BOOTLOADER = atmel-dfu
USER_NAME = manna-harbour_miryoku

# Per-key LEDs: RGB Matrix (see config.h). RGBLIGHT is off by default, stated
# here so enabling it later cannot happen by accident.
RGBLIGHT_ENABLE = no
RGB_MATRIX_ENABLE = yes
LTO_ENABLE = yes

# Encoder, OLED, mouse keys and extra keys are already enabled by the Sofle's
# own keyboard.json; only the unused core features are trimmed here because
# flash is tight on the ATmega32u4.
AUDIO_ENABLE = no
MAGIC_ENABLE = no
SPACE_CADET_ENABLE = no
GRAVE_ESC_ENABLE = no
