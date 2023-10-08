#include "app.h"
#include "fir_low_pass_05_1_10k.h"
#include "fir_low_pass_25_3_10k.h"
#include "fir_low_pass_25_3_100k.h"
#include "fir_band_pass_4_8_100k.h"
#include "fir_band_pass_4_8_100k2.h"
#include "fir_band_pass_1_2_10k.h"
#include "fir_low_pass_1_15_10k.h"
#include "fir_high_pass_1_15_10k.h"
#include "fir_band_pass_05_15_10k.h"
#include "pds.h"

extern DAC_HandleTypeDef hdac;

filter_t filter;

#define A_NORMAL 60
#define A_NORMAL_FFT 120
#define OFFSET 147
#define OFFSET_FFT Y_END_AXIS
#define A_VALUES 4095
#define SAMPLES_TO_SHOW 300
#define MAX_SAMPLES 512
#define FFT_SAMPLES MAX_SAMPLES + 212
#define SAMPLING_FREQ 10000
#define START_DRAWING X_START_AXIS + 4


void show_labels(void);
void parse_amplitudes(int16_t arr[], uint16_t len);
void parse_amplitudes_fft(int16_t arr[], uint16_t len);
void clear_input_freq(void);
void draw_fft(uint16_t x, int16_t y, uint16_t color );

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
    /* We are going to change the sample frequency to see how the T scale works */
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
//	uint32_t maxIndex;
	float tmp;

//	arm_max_q15(arr, len, &max, &maxIndex);

	for (uint16_t i = 0; i < len; i++)
	{
		int16_t curr = abs(arr[i]);
		if (curr > max) max = curr;
	}

	for (uint16_t i = 0; i < len; i++)
	{
		tmp = (arr[i]/(float)max)*A_NORMAL;
		arr[i] = (int16_t)tmp ;
	}
}

void parse_amplitudes_fft(int16_t arr[], uint16_t len)
{
	/* Find the maximum value */
	int16_t max = abs(arr[0]);
//	uint32_t maxIndex;
	float tmp;

//	arm_max_q15(arr, len, &max, &maxIndex);

	for (uint16_t i = 0; i < len; i++)
	{
		int16_t curr = abs(arr[i]);
		if (curr > max) max = curr;
	}

	for (uint16_t i = 0; i < len; i++)
	{
		tmp = (arr[i]/(float)max)*A_NORMAL_FFT;
		arr[i] = (int16_t)tmp ;
	}
}

void create_sine_wave_and_fir(uint16_t tone)
{
	arm_rfft_instance_q15 S;
	arm_rfft_instance_q15 S2;

	uint16_t fix_tone = tone;
	float32_t val = 0;
	float32_t t = 0;

	/* Values are going to be stored here */
    int16_t sine	[MAX_SAMPLES];
	int16_t y		[MAX_SAMPLES + h5_LENGTH - 1]; //300+162-1 = 461 para la convolucion
	int16_t tmp 	[MAX_SAMPLES];

	/* Arrays for FFT */
	/* fftOut have MAX_SAMPLES*2 because is real and imag values */
	/* Necesito potencia de 2 para que funcione */
	q15_t fftMag 	[(FFT_SAMPLES)/2 + 1];
    q15_t fftOut 	[(FFT_SAMPLES)*2];
    q15_t fftIn		[(FFT_SAMPLES)];
    int16_t fft_tmp [(MAX_SAMPLES)];
    int16_t fft_factor = (FFT_SAMPLES)/MAX_SAMPLES;
	q15_t maxValue;
	uint32_t maxIndex;


	q15_t fftMag2 		[(FFT_SAMPLES)/2 + 1];
	q15_t fftOut2 		[(FFT_SAMPLES)*2];
	q15_t fftIn2		[(FFT_SAMPLES)];
	int16_t fft_tmp2 	[(MAX_SAMPLES)];
	int16_t fft_factor2 = (FFT_SAMPLES)/MAX_SAMPLES;
	q15_t maxValue2;
	uint32_t maxIndex2;


	/* This is to do sweep */
	uint32_t steps = SAMPLING_FREQ/10;

	/* Con este factor sacamos solo 300 puntos de los 512 para mostrar en pantalla */
	int16_t factor  = (MAX_SAMPLES + h5_LENGTH - 1) / MAX_SAMPLES;

	/* First I want to see the fir filter wave */
//	parse_amplitudes(h3, h3_LENGTH);
//	parse_amplitudes(HAbs3, H3_PADD_LENGTH);

//	for (uint16_t i = 0; i < MAX_SAMPLES; i++)
//	{
//		ili_draw_pixel(X_START_AXIS + i, (h3[i] - (h3[i]*2) + OFFSET), ILI_COLOR_WHITE);
//	}

	while(1)
    {
		for (uint16_t i = 0; i < MAX_SAMPLES; i++)
		{
			t = i/(float)SAMPLING_FREQ;

			/* Get the value from the CMSIS-DSP library */
			val =  2048 * arm_sin_f32 (t*tone*2*PI);

			sine[i] = val;
			fftIn[i] = val;
		}

		tone = tone + 100;
		if (tone > 150*fix_tone)
		{
			tone = fix_tone;
		}

		/* Here we get the convolution so is the output */
		arm_conv_q15(sine, MAX_SAMPLES, h5, h5_LENGTH, y);

		for (uint16_t i = 0; i < MAX_SAMPLES + h5_LENGTH - 1; i++)
		{
			if (i < h5_LENGTH/2)
			{
				y[i] = 0;
			}
			else if (i > h5_LENGTH/2 + MAX_SAMPLES)
			{
				y[i] = 0;
			}
			else
			{
				fftIn2[i] = y[i];
			}
		}

//		for (uint16_t i = 0; i < MAX_SAMPLES; i++)
//		{
//			fftIn2[i] = y[i + h5_LENGTH];
//			y[i] = y[i + h5_LENGTH];
//		}

		/* This is to format the output to show in the screen */
		parse_amplitudes(y, MAX_SAMPLES + h5_LENGTH - 1);


		/* We want the FFT to show in the screen */
		/* We need to initialize the fft */
		arm_rfft_init_q15(&S, FFT_SAMPLES, 0, 1);

		/* Now we do the fft */
		arm_rfft_q15(&S, fftIn, fftOut);

		/* Get the max squared value */
		arm_cmplx_mag_squared_q15(fftOut, fftMag, FFT_SAMPLES/2 + 1);

		/* Get max value */
		arm_max_q15(fftMag, FFT_SAMPLES/2 + 1, &maxValue, &maxIndex);

		/* We want the FFT to show in the screen */
		/* We need to initialize the fft */
		arm_rfft_init_q15(&S2, FFT_SAMPLES, 0, 1);

		/* Now we do the fft */
		arm_rfft_q15(&S2, fftIn2, fftOut2);

		/* Get the max squared value */
		arm_cmplx_mag_squared_q15(fftOut2, fftMag2, FFT_SAMPLES/2 + 1);

		/* Get max value */
		arm_max_q15(fftMag2, FFT_SAMPLES/2 + 1, &maxValue2, &maxIndex2);

		/* Normalize the amplitude */
		parse_amplitudes_fft(fftOut2, (FFT_SAMPLES)*2);

		/* Normalize the amplitude */
		parse_amplitudes_fft(fftOut, (FFT_SAMPLES)*2);

		/* This is to get only MAX_SAMPLES number of values so we can show in the screen */
		for (uint16_t j = 0; j < MAX_SAMPLES; j++)
		{
			tmp[j] = y[j + h4_LENGTH/2];
		}

		for (uint16_t i = 0; i < MAX_SAMPLES - 5; i++)
		{
			/* tmp[i]*2 + OFFSET is to mirror the signal on the Y axis and to put in the correct place in the screen */
			ili_draw_pixel(START_DRAWING + i, tmp[i] - (tmp[i]*2) + OFFSET, ILI_COLOR_WHITE);

			/* The next two lines helps drawing better the shape */
			ili_draw_pixel(START_DRAWING + i + 1, tmp[i] - (tmp[i]*2) + OFFSET, ILI_COLOR_WHITE);
			ili_draw_pixel(START_DRAWING + i + 2, tmp[i] - (tmp[i]*2) + OFFSET, ILI_COLOR_WHITE);

			/* Frequency response from the filter */
			// ili_draw_pixel(X_START_AXIS + i, (HAbs3[i] - (HAbs3[i]*2) + OFFSET), ILI_COLOR_BLUE);

			/* Draw lineas for every frequency */
			draw_fft(START_DRAWING + i, fftOut[i], ILI_COLOR_BLUE);

			draw_fft(START_DRAWING + i, fftOut2[i], ILI_COLOR_RED);
		}
		refresh_cartesian_axis();
		show_input_freq(tone);
    }
}

void draw_fft(uint16_t x, int16_t y, uint16_t color )
{
	for (uint16_t i=0; i < y; i++){
		//(fft_tmp[i] - (fft_tmp[i]*2) + OFFSET)
		ili_draw_pixel(x, i - i*2+ OFFSET_FFT, color);
		ili_draw_pixel(x + 1, i - i*2 + OFFSET_FFT, color);
	}
}



void sample_adc_and_show(uint16_t channel)
{
	DBG_CyclesCounterInit(CLOCK_SPEED);
	uint16_t sample = 0;

	/* Sampling related functions */
	int16_t adc [MAX_SAMPLES];
	int16_t tmp [MAX_SAMPLES];
	int16_t y	[MAX_SAMPLES + h3_LENGTH - 1];

	/* Con este factor sacamos solo 300 puntos de los 512 para mostrar en pantalla */
	int16_t factor  = (MAX_SAMPLES + h3_LENGTH - 1) / MAX_SAMPLES;

	while(1)
	{
		/* Reset the cycle counter */
		DBG_CyclesCounterReset();

		adc[sample] = (int16_t)ADC_Read(0) - 2048;

		adc[sample] = (adc[sample]/(float)2048)*(float)A_NORMAL;

		if (sample < MAX_SAMPLES - 1){
			ili_draw_pixel(START_DRAWING + sample, adc[sample] - adc[sample]*2 + OFFSET, ILI_COLOR_WHITE);
		}

		if (++sample == MAX_SAMPLES)
		{
			HAL_Delay(10);
			sample = 0;
			refresh_cartesian_axis();
		}

		while(DBG_CyclesCounterRead() < CLOCK_SPEED/SAMPLING_FREQ);
	}
}


void sample_adc_and_filter1(uint16_t channel)
{
	DBG_CyclesCounterInit(CLOCK_SPEED);
	uint16_t sample = 0;

	/* Sampling related functions */
	int16_t adc [MAX_SAMPLES];
	int16_t tmp [MAX_SAMPLES];
	int16_t y	[MAX_SAMPLES + h6_LENGTH - 1]; //1024 + 65 - 1 = 1088

	/* Con este factor sacamos solo 300 puntos de los 512 para mostrar en pantalla */
	int16_t factor  = (MAX_SAMPLES + h6_LENGTH - 1) / MAX_SAMPLES; // 1.0625

	/* Arrays for FFT */
	/* fftOut have MAX_SAMPLES*2 because is real and imag values */
	/* Necesito potencia de 2 para que funcione. Entonces FFT_SAMPLES es 512 */
	arm_rfft_instance_q15 S;
	arm_rfft_instance_q15 S2;
	q15_t fftMag 		[(MAX_SAMPLES)/2 + 1]; 	// 513
	q15_t fftOut 		[(MAX_SAMPLES)*2];     	// 2048
	q15_t fftIn			[(MAX_SAMPLES)];		// 1024
	int16_t fft_tmp		[(MAX_SAMPLES)];		// 1024
//	int16_t fft_factor = (FFT_SAMPLES)/MAX_SAMPLES;

	q15_t fftMag2 		[(MAX_SAMPLES)/2 + 1];	// 513
	q15_t fftOut2 		[(MAX_SAMPLES)*2];		// 2048
	q15_t fftIn2		[(MAX_SAMPLES)];		// 1024
	int16_t fft_tmp2	[(MAX_SAMPLES)];		// 1024

	q15_t maxValue;
	uint32_t maxIndex;

	q15_t maxValue2;
	uint32_t maxIndex2;


	while(1)
	{
		/* Reset the cycle counter */
		DBG_CyclesCounterReset();

		/* ADC[1024] -> Is going to sample 1024 values before doing something with it */
		adc[sample] = (int16_t)ADC_Read(0) - 2048;
		fftIn[sample] = adc[sample];

		if (++sample == MAX_SAMPLES)
		{

			/* Here we get the convolution that is the output of the filter */
			/* The convolution will have an output of MAX_SAMPLES + h6_LENGTH = 1024 + 65 - 1 = 1088 */

//			arm_conv_q15(adc, MAX_SAMPLES, h6, h6_LENGTH, y);
//
//			/* When we do a convolution the first part will be garbage so we want to get rid of it */
//			uint16_t i = 0;
//			for (uint16_t k = h6_LENGTH - 1; k < SAMPLES_TO_SHOW + h6_LENGTH - 1; k++)
//			{
//				tmp[i] = y[k];
//				fftIn2[i] = tmp[i];
//				i++;
//			}

			/* We want the FFT to show in the screen */
			/* We need to initialize the fft */
			 arm_rfft_init_q15(&S, MAX_SAMPLES, 0, 1);

			/* Now we do the fft */
			 arm_rfft_q15(&S, fftIn, fftOut);

			/* Get the max squared value */
			 arm_cmplx_mag_squared_q15(fftOut, fftMag, MAX_SAMPLES/2 + 1);

			/* Get max value */
			// arm_max_q15(fftMag, FFT_SAMPLES/2 + 1, &maxValue, &maxIndex);

			/* We want the FFT to show in the screen */
			/* We need to initialize the fft */
			// arm_rfft_init_q15(&S2, FFT_SAMPLES, 0, 1);

			/* Now we do the fft */
			// arm_rfft_q15(&S2, fftIn2, fftOut2);

			/* Get the max squared value */
			// arm_cmplx_mag_squared_q15(fftOut2, fftMag2, FFT_SAMPLES/2 + 1);

			/* Get max value */
			// arm_max_q15(fftMag, FFT_SAMPLES/2 + 1, &maxValue2, &maxIndex2);

			/* Normalize the amplitude */
			 parse_amplitudes_fft(fftOut, (MAX_SAMPLES)*2);

			/* Normalize the amplitude */
			// parse_amplitudes_fft(fftOut2, (FFT_SAMPLES)*2);

			parse_amplitudes(adc, MAX_SAMPLES);
			parse_amplitudes(tmp, MAX_SAMPLES);


			for (uint16_t i = 0; i < SAMPLES_TO_SHOW - 5; i++)
			{
				/* tmp[i]*2 + OFFSET is to mirror the signal on the Y axis and to put in the correct place in the screen */
//				ili_draw_pixel(START_DRAWING + i, adc[i] - (adc[i]*2) + OFFSET, ILI_COLOR_BLUE);
//
//				ili_draw_pixel(START_DRAWING + i + 1, adc[i] - (adc[i]*2) + OFFSET, ILI_COLOR_BLUE);
//				ili_draw_pixel(START_DRAWING + i + 2, adc[i] - (adc[i]*2) + OFFSET, ILI_COLOR_BLUE);

//				ili_draw_pixel(START_DRAWING + i, tmp[i] - (tmp[i]*2) + OFFSET, ILI_COLOR_WHITE);

				/* The next two lines helps drawing better the shape */
				ili_draw_pixel(START_DRAWING + i + 1, tmp[i] - (tmp[i]*2) + OFFSET, ILI_COLOR_WHITE);
				ili_draw_pixel(START_DRAWING + i + 2, tmp[i] - (tmp[i]*2) + OFFSET, ILI_COLOR_WHITE);

				/* Draw lineas for every frequency */
				 draw_fft(START_DRAWING + i, fftOut[i], ILI_COLOR_BLUE);

				// draw_fft(START_DRAWING + i, fftOut2[i], ILI_COLOR_RED);
			}
			refresh_cartesian_axis();
//			show_max_freq(fftOut[maxIndex]);
			sample = 0;
		}

		while(DBG_CyclesCounterRead() < CLOCK_SPEED/SAMPLING_FREQ);
	}
}

void sample_adc_and_filter_low_pass(uint16_t channel)
{
	DBG_CyclesCounterInit(CLOCK_SPEED);
	uint16_t sample = 0;

	/* Sampling related functions */
	int16_t adc [MAX_SAMPLES];
	int16_t tmp [MAX_SAMPLES];
	int16_t y	[MAX_SAMPLES + h7_LENGTH - 1]; // N = 512 , M = 65 => N+M-1 = 512+65-1 = 576

	/* Arrays for FFT */
	/* fftOut have MAX_SAMPLES*2 because is real and imag values */
	/* Necesito potencia de 2 para que funcione. Entonces FFT_SAMPLES es 512 */
	arm_rfft_instance_q15 S;
	q15_t fftMag 		[(MAX_SAMPLES)/2 + 1]; 	// 257
	q15_t fftOut 		[(MAX_SAMPLES)*2];     	// 1024
	q15_t fftIn			[(MAX_SAMPLES)];		// 512

	arm_rfft_instance_q15 S2;
	q15_t fftMag2 		[(MAX_SAMPLES)/2 + 1];	// 257
	q15_t fftOut2 		[(MAX_SAMPLES)*2];		// 1024
	q15_t fftIn2		[(MAX_SAMPLES)];		// 512

	uint16_t k = 0;

	parse_amplitudes(HAbs7, H7_PADD_LENGTH);
	int16_t tmpVal = 0;

	while(filter == LOW_PASS)
	{
		/* Reset the cycle counter */
		DBG_CyclesCounterReset();

		/* ADC[512] -> Is going to sample 512 values before doing something with it */
		adc[sample] = (int16_t)ADC_Read(0) - 2048;
		fftIn[sample] = adc[sample];

		if (++sample == MAX_SAMPLES)
		{

			arm_conv_q15(adc, MAX_SAMPLES, h7, h7_LENGTH, y);

			/* Arrange the values */
			for (uint16_t i = 0; i < MAX_SAMPLES + h7_LENGTH; i++)
			{
				tmpVal = y[i] - (y[i]*2) + OFFSET;
				y[i] = tmpVal;
			}

			k = 0;
			for (uint16_t j = 0; j < MAX_SAMPLES + h7_LENGTH - 1; j++)
			{
				if (j < h7_LENGTH)
				{
					fftIn2[k] = 0;
					tmp[k] = 0;
				}
				else if ( j > h7_LENGTH && j < MAX_SAMPLES)
				{
					fftIn2[k] = y[j];
					tmp[k] = y[j];
				}
				k++;
			}

			arm_rfft_init_q15(&S, MAX_SAMPLES, 0, 1);
			arm_rfft_q15(&S, fftIn, fftOut);

			arm_rfft_init_q15(&S2, MAX_SAMPLES, 0, 1);
			arm_rfft_q15(&S, fftIn2, fftOut2);

			parse_amplitudes(tmp, MAX_SAMPLES);
			parse_amplitudes_fft(fftOut, MAX_SAMPLES);
			parse_amplitudes_fft(fftOut2, MAX_SAMPLES);

			for (uint16_t i = 0; i < SAMPLES_TO_SHOW - 5; i++)
			{
				ili_draw_pixel(START_DRAWING + i	, tmp[i] + OFFSET, ILI_COLOR_WHITE);
				ili_draw_pixel(START_DRAWING + i + 1, tmp[i] + OFFSET, ILI_COLOR_WHITE);

				ili_draw_pixel(START_DRAWING + i, HAbs7[i/2] - (HAbs7[i/2]*2) + OFFSET, ILI_COLOR_GREEN);
				ili_draw_pixel(START_DRAWING + i, HAbs7[i/2]*1.2 - (HAbs7[i/2]*2.2) + OFFSET, ILI_COLOR_GREEN);

				/* Draw lines for every frequency */
				draw_fft(START_DRAWING + i, fftOut[i + 7], ILI_COLOR_BLUE); // FFT from the signal filtered
				draw_fft(START_DRAWING + i, fftOut2[i + 7], ILI_COLOR_RED); // FFT from the signal not filtered

			}
			refresh_cartesian_axis();
			sample = 0;
		}

		while(DBG_CyclesCounterRead() < CLOCK_SPEED/SAMPLING_FREQ);
	}
}

void sample_adc_and_filter_high_pass(uint16_t channel)
{
	DBG_CyclesCounterInit(CLOCK_SPEED);
	uint16_t sample = 0;

	/* Sampling related functions */
	int16_t adc [MAX_SAMPLES];
	int16_t tmp [MAX_SAMPLES];
	int16_t y	[MAX_SAMPLES + h8_LENGTH - 1]; // N = 512 , M = 65 => N+M-1 = 512+65-1 = 576

	/* Arrays for FFT */
	/* fftOut have MAX_SAMPLES*2 because is real and imag values */
	/* Necesito potencia de 2 para que funcione. Entonces FFT_SAMPLES es 512 */
	arm_rfft_instance_q15 S;
	q15_t fftMag 		[(MAX_SAMPLES)/2 + 1]; 	// 257
	q15_t fftOut 		[(MAX_SAMPLES)*2];     	// 1024
	q15_t fftIn			[(MAX_SAMPLES)];		// 512

	arm_rfft_instance_q15 S2;
	q15_t fftMag2 		[(MAX_SAMPLES)/2 + 1];	// 257
	q15_t fftOut2 		[(MAX_SAMPLES)*2];		// 1024
	q15_t fftIn2		[(MAX_SAMPLES)];		// 512

	uint16_t k = 0;

	parse_amplitudes(HAbs8, H8_PADD_LENGTH);
	int16_t tmpVal = 0;

	while(filter == HIGH_PASS)
	{
		/* Reset the cycle counter */
		DBG_CyclesCounterReset();

		/* ADC[512] -> Is going to sample 512 values before doing something with it */
		adc[sample] = (int16_t)ADC_Read(0) - 2048;
		fftIn[sample] = adc[sample];

		if (++sample == MAX_SAMPLES)
		{

			arm_conv_q15(adc, MAX_SAMPLES, h8, h8_LENGTH, y);

			/* Arrange the values */
			for (uint16_t i = 0; i < MAX_SAMPLES + h8_LENGTH; i++)
			{
				tmpVal = y[i] - (y[i]*2) + OFFSET;
				y[i] = tmpVal;
			}

			k = 0;
			for (uint16_t j = 0; j < MAX_SAMPLES + h8_LENGTH - 1; j++)
			{
				if (j < h8_LENGTH)
				{
					fftIn2[k] = 0;
					tmp[k] = 0;
				}
				else if ( j > h8_LENGTH && j < MAX_SAMPLES)
				{
					fftIn2[k] = y[j];
					tmp[k] = y[j];
				}
				k++;
			}

			arm_rfft_init_q15(&S, MAX_SAMPLES, 0, 1);
			arm_rfft_q15(&S, fftIn, fftOut);

			arm_rfft_init_q15(&S2, MAX_SAMPLES, 0, 1);
			arm_rfft_q15(&S, fftIn2, fftOut2);

			parse_amplitudes(tmp, MAX_SAMPLES);
			parse_amplitudes_fft(fftOut, MAX_SAMPLES);
			parse_amplitudes_fft(fftOut2, MAX_SAMPLES);

			for (uint16_t i = 0; i < SAMPLES_TO_SHOW - 5 ; i++)
			{
				ili_draw_pixel(START_DRAWING + i, tmp[i] + OFFSET, ILI_COLOR_WHITE);
				ili_draw_pixel(START_DRAWING + i + 1, tmp[i] + OFFSET, ILI_COLOR_WHITE);

				ili_draw_pixel(START_DRAWING + i, HAbs8[i-40] - (HAbs8[i-40]*2) + OFFSET, ILI_COLOR_GREEN);
				ili_draw_pixel(START_DRAWING + i, HAbs8[i-40]*1.2 - (HAbs8[i-40]*2.2) + OFFSET, ILI_COLOR_GREEN);

				/* Draw lines for every frequency */
				draw_fft(START_DRAWING + i, fftOut[i + 7], ILI_COLOR_BLUE); // FFT from the signal filtered
				draw_fft(START_DRAWING + i, fftOut2[i + 7], ILI_COLOR_RED); // FFT from the signal not filtered

			}

			refresh_cartesian_axis();
			sample = 0;
		}
//		DAC_Write(&hdac, tmp[sample]);
		while(DBG_CyclesCounterRead() < CLOCK_SPEED/SAMPLING_FREQ);
	}
}


void sample_adc_and_filter_band_pass(uint16_t channel)
{
	DBG_CyclesCounterInit(CLOCK_SPEED);
	uint16_t sample = 0;

	/* Sampling related functions */
	int16_t adc [MAX_SAMPLES];
	int16_t tmp [MAX_SAMPLES];
	int16_t y	[MAX_SAMPLES + h9_LENGTH - 1]; // N = 512 , M = 65 => N+M-1 = 512+65-1 = 576

	/* Arrays for FFT */
	/* fftOut have MAX_SAMPLES*2 because is real and imag values */
	/* Necesito potencia de 2 para que funcione. Entonces FFT_SAMPLES es 512 */
	arm_rfft_instance_q15 S;
	q15_t fftMag 		[(MAX_SAMPLES)/2 + 1]; 	// 257
	q15_t fftOut 		[(MAX_SAMPLES)*2];     	// 1024
	q15_t fftIn			[(MAX_SAMPLES)];		// 512

	arm_rfft_instance_q15 S2;
	q15_t fftMag2 		[(MAX_SAMPLES)/2 + 1];	// 257
	q15_t fftOut2 		[(MAX_SAMPLES)*2];		// 1024
	q15_t fftIn2		[(MAX_SAMPLES)];		// 512

	uint16_t k = 0;

	parse_amplitudes(HAbs9, H9_PADD_LENGTH);

	int16_t f_fundamental;
	q15_t maxValue;
	uint32_t maxIndex;
	int16_t tmpVal;

	while(filter == BAND_PASS)
	{
		/* Reset the cycle counter */
		DBG_CyclesCounterReset();

		/* ADC[512] -> Is going to sample 512 values before doing something with it */
		adc[sample] = (int16_t)ADC_Read(0) - 2048;
		fftIn[sample] = adc[sample];

		if (++sample == MAX_SAMPLES)
		{

			arm_conv_q15(adc, MAX_SAMPLES, h9, h9_LENGTH, y);

			/* Arrange the values */
			for (uint16_t i = 0; i < MAX_SAMPLES + h9_LENGTH; i++)
			{
				tmpVal = y[i] - (y[i]*2) + OFFSET;
				y[i] = tmpVal;
			}

			k = 0;
			for (uint16_t j = 0; j < MAX_SAMPLES + h9_LENGTH - 1; j++)
			{
				if (j < h9_LENGTH)
				{
					fftIn2[k] = 0;
					tmp[k] = 0;
				}
				else if ( j > h9_LENGTH && j < MAX_SAMPLES)
				{
					fftIn2[k] = y[j];
					tmp[k] = y[j];
				}
				k++;
			}

			arm_rfft_init_q15(&S, MAX_SAMPLES, 0, 1);
			arm_rfft_q15(&S, fftIn, fftOut);

			arm_rfft_init_q15(&S2, MAX_SAMPLES, 0, 1);
			arm_rfft_q15(&S, fftIn2, fftOut2);

			parse_amplitudes(tmp, MAX_SAMPLES);
			parse_amplitudes_fft(fftOut, MAX_SAMPLES);
			parse_amplitudes_fft(fftOut2, MAX_SAMPLES);

			for (uint16_t i = 0; i < SAMPLES_TO_SHOW - 5 ; i++)
			{
				ili_draw_pixel(START_DRAWING + i	, tmp[i] + OFFSET, ILI_COLOR_WHITE);
				ili_draw_pixel(START_DRAWING + i + 1, tmp[i] + OFFSET, ILI_COLOR_WHITE);
				ili_draw_pixel(START_DRAWING + i + 2, tmp[i] + OFFSET, ILI_COLOR_WHITE);

				ili_draw_pixel(START_DRAWING + i, HAbs9[i/2+10] - (HAbs9[i/2+10]*2) + OFFSET, ILI_COLOR_GREEN);
				ili_draw_pixel(START_DRAWING + i, HAbs9[i/2+10]*1.2 - (HAbs9[i/2+10]*2.2) + OFFSET, ILI_COLOR_GREEN);

				/* Draw lines for every frequency */
				draw_fft(START_DRAWING + i, fftOut[i + 7], ILI_COLOR_BLUE); // FFT from the signal filtered
				draw_fft(START_DRAWING + i, fftOut2[i + 7], ILI_COLOR_RED); // FFT from the signal not filtered

			}

			refresh_cartesian_axis();
			sample = 0;
		}
//		DAC_Write(&hdac, tmp[sample]);
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
	show_labels();
	create_space_for_plot(X_START_AXIS, Y_START_AXIS, X_END_AXIS, Y_END_AXIS,ILI_COLOR_BLACK);
	create_axis_lines();
}

void show_max_freq(uint16_t freq)
{
	static char f0[6];
	sprintf(f0, "%05d", freq);
	ili_draw_string_withbg(265, 50, f0, ILI_COLOR_WHITE, ILI_COLOR_BLACK, &font_microsoft_16);
}

void show_input_freq(uint16_t freq)
{
	static char f0[6];
	sprintf(f0, "%05d", freq);
	ili_draw_string_withbg(35, 50, f0, ILI_COLOR_WHITE, ILI_COLOR_BLACK, &font_microsoft_16);
}

void show_labels(void)
{
	ili_draw_string_withbg(10, 50, "  ", ILI_COLOR_BLUE, ILI_COLOR_BLUE,&font_microsoft_16);
	ili_draw_string_withbg(20, 50, "FFT In", ILI_COLOR_WHITE, ILI_COLOR_BLACK,&font_microsoft_16);
	ili_draw_string_withbg(80, 50, "  ", ILI_COLOR_RED, ILI_COLOR_RED,&font_microsoft_16);
	ili_draw_string_withbg(90, 50, "FFT Out", ILI_COLOR_WHITE, ILI_COLOR_BLACK,&font_microsoft_16);
	ili_draw_string_withbg(165, 50, "  ", ILI_COLOR_GREEN, ILI_COLOR_GREEN,&font_microsoft_16);
	ili_draw_string_withbg(175, 50, "Filter", ILI_COLOR_WHITE, ILI_COLOR_BLACK,&font_microsoft_16);
	ili_draw_string_withbg(230, 50, "  ", ILI_COLOR_WHITE, ILI_COLOR_WHITE,&font_microsoft_16);
	ili_draw_string_withbg(240, 50, "Output", ILI_COLOR_WHITE, ILI_COLOR_BLACK,&font_microsoft_16);
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
	ili_draw_string(X_START_AXIS, Y_END_AXIS + 4 , "0                    1                      2                     3kHz", ILI_COLOR_WHITE,&font_microsoft_16);
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == BOTON1_Pin)
	{
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		filter = LOW_PASS;
	}
	else if(GPIO_Pin == BOTON2_Pin)
	{
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		filter = HIGH_PASS;
	}
	else
	{
		HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
		filter = BAND_PASS;
	}
}
