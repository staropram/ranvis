#include <stdio.h>
#include <errno.h>
#include <pngwriter.h>

#define MAX_IMAGE_SIDELEN 1000

int main(int argc, char **argv) {
	// check args
	if(argc!=5) {
		fprintf(stderr,"USAGE\r\n   %s input sidelength_pixels bins output\r\n",argv[0]);
		exit(1);
	}

	// get sidelength
	errno=0;
	int sidelength = (int)strtol(argv[2],NULL,10);
	if(errno!=0||sidelength==0) {
		fprintf(stderr,"Error converting \"%s\" to an integer\r\n",argv[2]);
		exit(2);
	}

	if(sidelength>MAX_IMAGE_SIDELEN) {
		fprintf(stderr,"Image sidelength specified (%d) exceeds maximum output side length\r\n",sidelength);
		exit(3);
	}

	if(sidelength<0) {
		fprintf(stderr,"Image sidelength specified (%d) cannot be less than 0\r\n",sidelength);
		exit(4);
	}

	printf("got pixels: %d\r\n",sidelength);

	// get bins
	errno=0;
	int bins = (int)strtol(argv[3],NULL,10);
	if(errno!=0||bins==0) {
		fprintf(stderr,"Error converting \"%s\" to an integer\r\n",argv[3]);
		exit(2);
	}
	

	// open file
	FILE *f = NULL;
	f=fopen(argv[1],"r");
	if(f==NULL) {
		fprintf(stderr,"Error opening file %s for reading\r\n",argv[1]);
		exit(5);
	}

	// create memory buffer to capture intensity
	double **intensityMap = (double**)malloc(sidelength*sizeof(double*));
	for(int i=0; i<sidelength; i++) {
		intensityMap[i] = (double*)malloc(sidelength*sizeof(double));
		for(int j=0; j<sidelength; j++) {
			intensityMap[i][j] = 0.0;
		}
	}

   pngwriter png(sidelength,sidelength,0,argv[4]);

	unsigned char buf[1];

	// read the file 1 byte at a time, and convert to quadrants
	double currentX = 0.5, currentY = 0.5;
	double corners[4][2] = {
		{0,0}, // bottom left
		{0,1}, // top left
		{1,1}, // top right
		{1,0}  // bottom right
	};
	int quantizedX=0,quantizedY=0;
	double maxIntensity = 0, newIntensity = 0, numberOfIntensityBins = (double)bins;

	while(fread(buf,1,1,f)!=0) {
		// determine quadrant and move point towards that quadrant
		int quadrant = 0;
		if(*buf<64) {
			quadrant = 0;
		} else if(*buf<128) {
			quadrant = 1;
		} else if(*buf<192) {
			quadrant = 2;
		} else {
			quadrant = 3;
		}

		currentX = (currentX*0.5)+(0.5)*corners[quadrant][0];
		currentY = (currentY*0.5)+(0.5)*corners[quadrant][1];

		// quantize and plot current point
		quantizedX = 1+(int)(currentX*sidelength);
		quantizedY = 1+(int)(currentY*sidelength);
   	//png.plot(quantizedX,quantizedY, 1.0, 0.0, 0.0);

		// x,y coordinates for intensityMap
		int x = (int)(currentX*(sidelength-1));
		int y = (int)(currentY*(sidelength-1));

		intensityMap[x][y]++;
		newIntensity = intensityMap[x][y];
		if(newIntensity>maxIntensity) {
			maxIntensity = newIntensity;
		}
	}

	// construct bin boundaries
	double *binBoundaries = (double*)malloc(numberOfIntensityBins*sizeof(double));
	double binIncrement = maxIntensity/(numberOfIntensityBins-1);
	printf("Ma: %f\r\n",maxIntensity);
	printf("Boundaries:\r\n");
	for(int i=0; i<numberOfIntensityBins; i++) {
		binBoundaries[i] = binIncrement*i;
		printf("%f\r\n",binBoundaries[i]);
	}
	double *binCounts = (double*)malloc(numberOfIntensityBins*sizeof(double));
	for(int i=0; i<numberOfIntensityBins; i++) {
		binCounts[i] = 0;
	}

	double intensityIncrement = 1.0/(numberOfIntensityBins-1);
	// convert intensity plot to png
	for(int i=0; i<sidelength; i++) {
		for(int j=0; j<sidelength; j++) {
			double intensity = intensityMap[i][j];
			double plotIntensity = 0;
			for(int k=0; k<numberOfIntensityBins; k++) {
				if(intensity<=binBoundaries[k]) {
					plotIntensity = k*intensityIncrement;
					binCounts[k]++;
					break;
				}
			}
			//printf("%f ",plotIntensity);
   		//png.plot(i+1,j+1, 1.0-plotIntensity, 1.0-plotIntensity, 1.0-plotIntensity);
   		png.plot(i+1,j+1, plotIntensity, 0.0,0.0);
		}
	}

	printf("Counts:\r\n");
	for(int i=0; i<numberOfIntensityBins; i++) {
		printf("%f\r\n",binCounts[i]);
	}
	

	// check that file finised on EOF
	if(ferror(f)) {
		fprintf(stderr,"Error reading file\r\n");	
		exit(7);
	}

	if(!feof(f)) {
		printf("EOF OK\r\n");
	}

	// close file
	if(fclose(f)!=0) {
		fprintf(stderr,"Error closing file %s\r\n",argv[1]);
		exit(6);
	}


	int i;
   int y;

   png.close();
   
   return 0;
}
