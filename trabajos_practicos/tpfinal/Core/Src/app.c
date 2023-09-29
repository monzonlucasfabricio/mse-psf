#include "app.h"

#define A_NORMAL 50
#define OFFSET 147
#define A_VALUES 4095
#define MAX_SAMPLES 300
#define SAMPLING_FREQ 1000000

void create_sine_wave(uint16_t tone)
{

	float32_t val = 0;
	float32_t t = 0;

	/* Time base:
	 *	I have 300 samples that I can show on the screen
	 *	I can sample with low or high frequency
	 *	Lets say I have:
	 *	fs = 100.000
	 *	ts = 1/100.000 = 10uS. I will be taking a sample every 10uS
	 *  This means that if I have:
	 *  f0 = 100 and fs = 100.000
	 *  t0 = 1/100 = 10mS / 10uS = 1000 -> This means that I need 1000 samples to show 1 period of the wave
	 *
	 */

	/* Values are going to be stored here */
    uint16_t arr[MAX_SAMPLES] = {0};

    uint32_t steps = SAMPLING_FREQ/10;
    uint32_t freq = SAMPLING_FREQ*2;
    /* We are goin to change the sample frequency to see how the T scale works */
    while(freq >= SAMPLING_FREQ)
    {
		for (uint16_t i = 0; i < MAX_SAMPLES; i++)
		{
			t = i/(float)freq;

			/* Get the value from the CMSIS-DSP library */
			val =  2048 * arm_sin_f32 (t*tone*2*PI) + 2048;

			/* Normalize in float */
			val = (val/A_VALUES)*A_NORMAL;

			/* Cast the value to send an integer for plot */
			uint16_t val16 = (uint16_t)val;

			arr[i] = val16 + OFFSET;
		}

		for (uint16_t i = 0; i < MAX_SAMPLES; i++)
		{
			ili_draw_pixel(X_START_AXIS + i, arr[i], ILI_COLOR_WHITE);
		}
		HAL_Delay(1000);
		freq = freq - steps;
		refresh_cartesian_axis();
    }
}

void sample_adc_and_show(uint16_t channel)
{
	uint16_t sample = 0;
	int16_t val = 0;
	int16_t adc [MAX_SAMPLES] = {0};
	DBG_CyclesCounterInit(CLOCK_SPEED);

	while(1)
	{
		/* Reset the cycle counter */
		DBG_CyclesCounterReset();

		adc[sample] = (int16_t)ADC_Read(0) - 2048;

		adc[sample] = (adc[sample]/(float)2048)*(float)A_NORMAL;

		ili_draw_pixel(X_START_AXIS + sample, adc[sample] + OFFSET, ILI_COLOR_WHITE);

		if (++sample == MAX_SAMPLES)
		{
			HAL_Delay(10);
			sample = 0;
			refresh_cartesian_axis();
		}

		while(DBG_CyclesCounterRead() < CLOCK_SPEED/SAMPLING_FREQ);
	}
}

int map_values(uint16_t value, uint16_t fromLow, uint16_t fromHigh, uint16_t toLow, uint16_t toHigh)
{
	// Asegurarse de que el valor esté en el rango de entrada
	if (value < fromLow) {
		value = fromLow;
	}
	if (value > fromHigh) {
		value = fromHigh;
	}

	/* Do the mapping */
	int result = (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
	return result;
}

void refresh_cartesian_axis(void)
{
	create_space_for_plot(X_START_AXIS + 4, Y_START_AXIS, X_END_AXIS, Y_END_AXIS, ILI_COLOR_BLACK);

}

void create_cartesian_axis_for_plot()
{
	create_space_for_plot(X_START_AXIS, Y_START_AXIS, X_END_AXIS, Y_END_AXIS,ILI_COLOR_BLACK);
	create_axis_lines();
}

void set_title(void)
{
    ili_draw_string_withbg(30, 10, "PSF 2023 - FIR FILTER", ILI_COLOR_WHITE, ILI_COLOR_BLACK, &font_ubuntu_mono_24);
}

void create_space_for_plot(uint16_t x_start, uint16_t y_start, uint16_t alto, uint16_t ancho, uint16_t color)
{
    ili_set_address_window(x_start, y_start, alto, ancho);
    ili_fill_color(color, (alto - x_start + 1) * (ancho - y_start + 1));
}

void create_axis_lines(void)
{
	ili_draw_line(X_START_AXIS , Y_START_AXIS, X_START_AXIS , Y_END_AXIS, 4, ILI_COLOR_WHITE);
	ili_draw_line(X_START_AXIS, Y_END_AXIS , X_END_AXIS , Y_END_AXIS, 4, ILI_COLOR_WHITE);

	/* X axis numbers */
	ili_draw_string(X_START_AXIS, Y_END_AXIS + 4 , "0   1   2   3   4   5   6   7   8   9   10   11   12", ILI_COLOR_WHITE,&font_microsoft_16);
}
