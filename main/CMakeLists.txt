idf_component_register(SRCS main.c audio_i2s.c detection.c keywords_encodings.c blink_led.c
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_i2s esp_driver_gpio tinykw)
