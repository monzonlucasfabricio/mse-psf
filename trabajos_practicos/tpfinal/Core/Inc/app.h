#ifndef APP_H_
#define APP_H_

#include "main.h"
#include "ili9341_lib.h"
#include "pds.h"
#include "arm_math.h"
#include "font_ubuntu_mono_24.h"
#include "font_microsoft_16.h"

#define X_START_AXIS 10
#define X_END_AXIS 310
#define Y_START_AXIS 80
#define Y_END_AXIS 214

void sample_adc_and_show(uint16_t channel);
void create_sine_wave(uint16_t tone);
void refresh_cartesian_axis(void);
void create_cartesian_axis_for_plot(void);
void set_title(void);
void create_space_for_plot(uint16_t x_start, uint16_t y_start, uint16_t alto, uint16_t ancho, uint16_t color);
void create_axis_lines(void);

#endif// APP_H_
