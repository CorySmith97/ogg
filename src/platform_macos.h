#pragma once
#include <stdint.h>
#include <stdbool.h>

void platform_init(const char *name, uint32_t width, uint32_t height);
void platform_handle_events(bool *quit);
void platform_deinit(void);
void platform_present(void);
bool is_key_down(int key);
bool is_key_released(int key);
