#include "app.h"
#include "fir_low_pass_05_1_10k.h"
#include "fir_low_pass_25_3_10k.h"
#include "fir_low_pass_25_3_100k.h"

#define A_NORMAL 60
#define OFFSET 147
#define A_VALUES 4095
#define MAX_SAMPLES 300
#define SAMPLING_FREQ 100000

void parse_amplitudes(int16_t arr[], uint16_t len);
void clear_input_freq(void);

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

		for (uint16_t i = MAX_SAMPLES; i > 0; i++)
		{
			ili_draw_pixel(X_START_AXIS + i, arr[i], ILI_COLOR_WHITE);
		}
		HAL_Delay(1000);
		freq = freq - steps;
		refresh_cartesian_axis();
    }
}

void create_sine_wave_2(uint16_t tone)
{

	float32_t val = 0;
	float32_t t = 0;

	/* Values are going to be stored here */
    uint16_t arr[MAX_SAMPLES] = {0};

    uint32_t steps = SAMPLING_FREQ/10;
    uint32_t freq = SAMPLING_FREQ*2;
    /* We are goin to change the sample frequency to see how the T scale works */
    while(1)
    {
		for (uint16_t i = 0; i < MAX_SAMPLES; i++)
		{
			t = i/(float)SAMPLING_FREQ;

			/* Get the value from the CMSIS-DSP library */
			val =  2048 * arm_sin_f32 (t*tone*2*PI) + 2048;

			/* Cast the value to send an integer for plot */
			uint16_t val16 = (uint16_t)val;

			arr[i] = val16 + OFFSET;

		}

		uint16_t j = 0;
		for (uint16_t i = MAX_SAMPLES; i > 0; i--)
		{
			ili_draw_pixel(X_START_AXIS + j, arr[i], ILI_COLOR_WHITE);
			j++;
		}
		refresh_cartesian_axis();
    }
}

void parse_amplitudes(int16_t arr[], uint16_t len)
{
	/* Find the maximum value */
	int16_t max = abs(arr[0]);
	float tmp;

	for (uint16_t i = 0; i < len; i++)
	{
		int16_t curr = abs(arr[i]);
		if (curr > max) max = curr;
	}
	/* */
	for (uint16_t i = 0; i < len; i++)
	{
		tmp = (arr[i]/(float)max)*A_NORMAL;
		arr[i] = (int16_t)tmp ;
	}
}

void create_sine_wave_and_fir(uint16_t tone)
{

	uint16_t fix_tone = tone;
	float32_t val = 0;
	float32_t t = 0;

	/* Values are going to be stored here */
    int16_t sine[MAX_SAMPLES];
	int16_t y	[MAX_SAMPLES + h3_LENGTH - 1] = {0};
	int16_t tmp [MAX_SAMPLES];

	/* This is to do sweep */
	uint32_t steps = SAMPLING_FREQ/10;

	/* Con este factor sacamos solo 300 puntos de los 512 para mostrar en pantalla */
	int16_t factor  = (MAX_SAMPLES + h3_LENGTH - 1) / MAX_SAMPLES;

	/* First I want to see the fir filter wave */
	parse_amplitudes(h3, h3_LENGTH);
	parse_amplitudes(HAbs3, H3_PADD_LENGTH);
	for (uint16_t i = 0; i < MAX_SAMPLES; i++)
	{
		ili_draw_pixel(X_START_AXIS + i, (h3[i] - (h3[i]*2) + OFFSET), ILI_COLOR_WHITE);
	}
	while(1)
    {
		for (uint16_t freq = SAMPLING_FREQ; freq < SAMPLING_FREQ*10; freq += 10)
		{

			for (uint16_t i = 0; i < MAX_SAMPLES; i++)
			{
				t = i/(float)SAMPLING_FREQ;

				/* Get the value from the CMSIS-DSP library */
				val =  2048 * arm_sin_f32 (t*tone*2*PI);

				sine[i] = val;
			}

			tone = tone + 10;
			if (tone > 40*fix_tone)
			{
				tone = fix_tone;
			}

			/* Here we get the convolution so is the output */
			arm_conv_q15(sine, MAX_SAMPLES, h3, h3_LENGTH, y);

			/* This is to format the output to show in the screen */
			parse_amplitudes(y, MAX_SAMPLES + h3_LENGTH - 1);

			/* This is to get only MAX_SAMPLES number of values so we can show in the screen */
			for (uint16_t j = 0; j < MAX_SAMPLES; j++)
			{
				tmp[j] = y[j*factor + h3_LENGTH/2];
			}

			for (uint16_t i = 0; i < MAX_SAMPLES; i++)
			{
				/* tmp[i]*2 + OFFSET is to mirror the signal on the Y axis and to put in the correct place in the screen */
				ili_draw_pixel(X_START_AXIS + i, tmp[i] - (tmp[i]*2) + OFFSET, ILI_COLOR_WHITE);
				ili_draw_pixel(X_START_AXIS + i + 1, tmp[i] - (tmp[i]*2) + OFFSET, ILI_COLOR_WHITE);
				ili_draw_pixel(X_START_AXIS + i + 2, tmp[i] - (tmp[i]*2) + OFFSET, ILI_COLOR_WHITE);
				ili_draw_pixel(X_START_AXIS + i, (HAbs3[i] - (HAbs3[i]*2) + OFFSET), ILI_COLOR_BLUE);
			}
			refresh_cartesian_axis();
	//		show_input_freq(tone);
		}
    }
}

void create_sine_wave_and_fir_freq(uint16_t tone)
{

	/* This example will be in the domain of the frequency */
	/* Necesito inicializar un vector de 512 puntos. */
	/* 54 + N - 1 = 512 => 512-54+1 = N = 459 */
	arm_cfft_instance_q15 CS;
	uint32_t samples = 512 - h_PADD_LENGTH + 1;

	float32_t val = 0;
	float32_t t = 0;

	/* Values are going to be stored here */
    int16_t sine  [ (samples + h_LENGTH - 1)*1 ];
	q15_t fftInOut[ (samples + h_LENGTH - 1)*2 ];
	q15_t fftAbs  [ (samples + h_LENGTH - 1)*1 ];
	int16_t y     [ (samples + h_LENGTH - 1)*1 ]; // y[512]

	while(1)
    {
		for (uint16_t i = 0; i < samples; i++)
		{
			t = i/(float)SAMPLING_FREQ;

			/* Get the value from the CMSIS-DSP library */
			val =  2048 * arm_sin_f32 (t*tone*2*PI) + 2048;

			/* Normalize in float */
			// val = (val/A_VALUES)*A_NORMAL;

			/* Cast the value to send an integer for plot */
			// uint16_t val16 = (uint16_t)val;

			// sine[i] = val16 + OFFSET;
			sine[i] = val;
			fftInOut[i*2] = sine[i];
			fftInOut[i*2 + 1] = 0;
		}

		/* Transformada de fourier al dominio de la frequencia */
		init_cfft_instance( &CS, (samples + h_LENGTH - 1));
		arm_cfft_q15( &CS, fftInOut, 0, 1 );

		/* Valor absoluto */
		arm_cmplx_mag_q15( fftInOut, fftAbs, (samples + h_LENGTH - 1));

		arm_mult_q15(fftAbs, HAbs, fftAbs, (samples + h_PADD_LENGTH - 1));

		uint16_t j = 0;
		for (uint16_t i = 0; i < MAX_SAMPLES; i++)
		{
			ili_draw_pixel(X_START_AXIS + i, (fftAbs[i]/(float)A_VALUES)*(float)A_NORMAL + OFFSET, ILI_COLOR_WHITE);
		}
		refresh_cartesian_axis();
    }
}


void sample_adc_and_show(uint16_t channel)
{
	uint16_t sample = 0;
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


void sample_adc_and_filter(uint16_t channel)
{
	uint16_t sample = 0;
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
	set_title();
	create_space_for_plot(X_START_AXIS, Y_START_AXIS, X_END_AXIS, Y_END_AXIS,ILI_COLOR_BLACK);
	create_axis_lines();
}


void show_input_freq(uint16_t freq)
{
	static lastfreq = 0;
	char tmp[20];
	if (lastfreq = 0)
	{
		lastfreq = freq;
	}
	sprintf(tmp, "F0:%05d FS:10kHz", freq);
	ili_draw_string_withbg(10, 50, tmp, ILI_COLOR_WHITE, ILI_COLOR_BLACK, &font_ubuntu_mono_24);
}

void clear_input_freq(void)
{
	ili_set_address_window(10, 50, 20, 60);
	ili_fill_color(ILI_COLOR_WHITE, 100);
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
