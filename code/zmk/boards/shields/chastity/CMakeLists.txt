if(CONFIG_SHIELD_CHASTITY_LEFT OR CONFIG_SHIELD_CHASTITY_RIGHT)
  zephyr_library_sources(uart_release.c)
endif()
