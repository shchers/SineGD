/*
 * testgd.c  - an example of sine wave modeling and sine waves modulation with
 *             output results to PNG by GD library
 *
 * Components will be required: gcc libgd2-xpm-dev libfftw3-dev
 *   $ apt-get install gcc libgd2-xpm-dev libfftw3-dev
 *
 * Compiling and running this file (on cs) looks like this.
 *   $ gcc testgd.c -std=gnu99 -lgd -lm -lfftw3 -Wall -o testgd
 *   $ ./testgd
 *   $ firefox testgd.png
 *
 * (C) 2013 Sergey Shcherbakov <shchers@gmail.com>
 */
#include <stdio.h>
#include <gd.h>
#include <math.h>
#include <fftw3.h>
#include <malloc.h>
#include <memory.h>

// Dimensions of image in pixels
#define IMAGE_WIDTH  1280
#define IMAGE_HEIGHT 800

// Position of some blue lines drawn in the image.
#define BORDER       10
#define LEFT         BORDER
#define RIGHT        IMAGE_WIDTH - BORDER
#define TOP          BORDER
#define BOTTOM       IMAGE_HEIGHT - BORDER

// See the bottom of this code for a discussion of some output possibilities.
const char*   filename =   "testgd.png";

int main(){
	FILE*       outfile;                                  // defined in stdio
	gdImagePtr  image;                                    // a GD image object
	int         blue, green, red, black;                  // some GD colors
	int         x;                                        // array subscripts

	printf("Creating %i by %i image.\n", IMAGE_WIDTH, IMAGE_HEIGHT);
	image = gdImageCreate(IMAGE_WIDTH, IMAGE_HEIGHT);

	// First color will be used as background color for canvas (white in this case)
	gdImageColorAllocate(image, 0xcc, 0xcc, 0xcc);

	// Colors for drawing graphs
	blue  = gdImageColorAllocate(image, 0,0,255);        //  (red,green,blue)
	green = gdImageColorAllocate(image, 0,255,0);
	red   = gdImageColorAllocate(image, 255, 0, 0);
	black = gdImageColorAllocate(image, 0,0,0);

	// Drawing border
	gdImageLine(image, LEFT,TOP,     RIGHT,TOP,    blue); // draw lines in image
	gdImageLine(image, RIGHT,TOP,    RIGHT,BOTTOM, blue); //  +-----------------+
	gdImageLine(image, RIGHT,BOTTOM, LEFT,BOTTOM,  blue); //  |0,0       WIDTH,0|
	gdImageLine(image, LEFT,BOTTOM,  LEFT,TOP,     blue); //  |0,HEIGHT         |

	// Allocate memory for FFT
	int nPoints = (RIGHT - BORDER);
	fftw_complex *fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
	double *lpAmps = (double *)malloc(sizeof (double) * nPoints);
	fftw_plan plan_r2c = fftw_plan_dft_r2c_1d(nPoints, lpAmps, fft_out, FFTW_MEASURE);

	// Reset array
	memset(lpAmps, 0, sizeof (double) * nPoints);
	memset(fft_out, 0, sizeof (fftw_complex) * nPoints);

	int amp1_last = 0;
	int amp2_last = 0;
	int final_amp_last = 0;
	int amp = 0;

	gdImageLine(image, LEFT, (IMAGE_HEIGHT - 2*BORDER)/5, RIGHT, (IMAGE_HEIGHT - 2*BORDER)/5, black);
	gdImageLine(image, LEFT, (IMAGE_HEIGHT - 2*BORDER)*2/5, RIGHT, (IMAGE_HEIGHT - 2*BORDER)*2/5, black);
	gdImageLine(image, LEFT, (IMAGE_HEIGHT - 2*BORDER)*3/5, RIGHT, (IMAGE_HEIGHT - 2*BORDER)*3/5, black);

	// Plot sine graphs
	for (x=0; x < (RIGHT - BORDER); x++) {
		// Signal with 100% amplitude
		float amp1 = sin(2*M_PI*((float)x/((IMAGE_WIDTH - 2*BORDER)/24)));
		// Signal with 25% amplitude and pi/2 (i.e. 90deg) phase shift
		float amp2 = sin(2*M_PI*((float)x/((IMAGE_WIDTH - 2*BORDER)/83)) + M_PI/2)/4;
		float final_amp = amp1 + amp2;
		lpAmps[x] = final_amp;

		// Drawing signal #1
		amp = (int)(amp1 * (IMAGE_HEIGHT - 2*BORDER)/16);
		gdImageLine(image,
				LEFT + x,
				(IMAGE_HEIGHT - 2*BORDER)/5 - amp1_last,
				LEFT + x,
				(IMAGE_HEIGHT - 2*BORDER)/5 - amp,
				blue);
		amp1_last = amp;

		// Drawing signal #2
		amp = (int)(amp2 * (IMAGE_HEIGHT - 2*BORDER)/16);
		gdImageLine(image,
				LEFT + x,
				(IMAGE_HEIGHT - 2*BORDER)*2/5 - amp2_last,
				LEFT + x,
				(IMAGE_HEIGHT - 2*BORDER)*2/5 - amp,
				green);
		amp2_last = amp;

		// Drawing final signal
		amp = (int)(final_amp * (IMAGE_HEIGHT - 2*BORDER)/16);
		gdImageLine(image,
				LEFT + x,
				(IMAGE_HEIGHT - 2*BORDER)*3/5 - final_amp_last,
				LEFT + x,
				(IMAGE_HEIGHT - 2*BORDER)*3/5 - amp,
				red);
		final_amp_last = amp;
	}

	// Calculate FFT
	fftw_execute(plan_r2c);

	// Plot FFT magnitudes
	double r, i, mag;
	for (x=0; x < nPoints/2; x++) {
		r = fft_out[x][0];
		i = fft_out[x][1];

		// Calculate signal magnitude and print it to stdout
		mag = sqrt(r*r + i*i);
		printf("%2d %11.7f\n", x, mag);

		amp = (int)((mag/(double)nPoints) * (IMAGE_HEIGHT - 2*BORDER)/2);
		gdImageLine(image,
				LEFT + x,
				(IMAGE_HEIGHT - 2*BORDER),
				LEFT + x,
				(IMAGE_HEIGHT - 2*BORDER) - amp,
				red);
	}

	// Release memory
	fftw_destroy_plan(plan_r2c);
	fftw_free(fft_out);
	free(lpAmps);

	// Finally, write the image out to a file.
	printf("Creating output file '%s'.\n", filename);
	outfile = fopen(filename, "wb");
	gdImagePng(image, outfile);
	fclose(outfile);

	/**********
	* Notes about the output :
	*
	*  1. "wb" here means "write binary"; the 'binary' part only
	*     applies to Windows but doesn't hurt on unix boxes.
	*
	*  2. If this had be a true color image then probably a JPEG would
	*     be a better choice than a PNG; in that case you'd just say
	*        gdImageJpeg(image, outfile, quality);
	*     where 0<=quality<=100, or quality=-1 for the libjpeg default.
	*
	*  3. To send the output image to standard output, you'd just say
	*        gdImagePng(image, stdout);
	*      in which case you'd run the program like this :
	*        $ ./GD_example > outputfile.png
	*
	************/

}
