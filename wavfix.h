#ifndef WAVFIX_H_
#define WAVFIX_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

#define  MAX_CHANNELS   2

//define WAVE file structure and Class
//big endian = BE, little endian = LE File-offset-bytes = FOB  
struct Riff_CK_Header
{
	uint8_t ckID[4]; 	//BE 4-byte FOB:0   = "RIFF"
	uint32_t cksize; 	//LE 4-byte FOB:4
	uint8_t format[4];	//BE 4-byte FOB:8	= "WAVE" 
};

struct Fmt_CK_Header
{
	uint8_t  ckid[4];		//BE 4-byte FOB:12 = "fmt"
	uint32_t cksize;		//LE 4-byte FOB:16
	uint16_t audioformat;	//LE 2-byte FOB:20 1=PCM, 6=mulaw, 7=alaw
	uint16_t numchannels;	//LE 2-byte FOB:22 1=Mono, 2=Sterio
	uint32_t samplerate;	//LE 4-byte FOB:24 sample frequency in HZ
	uint32_t byterate;		//LE 4-byte FOB:28 bytes per second
	uint16_t blockalign;	//LE 2-byte FOB:32 2=16-bit mono, 4=16-bit stereo
	uint16_t bitspersample;	//LE 2-byte FOB:34
		
};

struct Data_CK_Header
{
	uint8_t ckid[4];	//BE 4-byte FOB:36 	= "data" 
	uint32_t cksize;	//LE 4-byte FOB:40 sampled data length
};

typedef struct _HEADER 
{
	Riff_CK_Header  riff;
	Fmt_CK_Header   fmt;
	Data_CK_Header  data;

} Wav_Header;


class WaveFile
{
	FILE *fWav;
	Wav_Header *wavheader;
	uint16_t nChannels, blockalign;
	uint16_t nBitsPerSample;
	uint32_t samplerate, byterate;
	bool isValid;
	
  public:
	WaveFile()
	{ 
		fWav = NULL; 
		nChannels = blockalign = nBitsPerSample =0; 
		samplerate = byterate = 0; 
		isValid = 0;
	}
	WaveFile(const char *filename)
	{
		fWav = fopen(filename, "r");
		if (fWav!=0)
		{
			wavheader = new Wav_Header; 
			fread(wavheader, 1, sizeof (Wav_Header), fWav);
			
			nChannels = wavheader->fmt.numchannels;
			blockalign = wavheader->fmt.blockalign;
			samplerate = wavheader->fmt.samplerate;
			byterate = wavheader->fmt.byterate;
			nBitsPerSample = wavheader->fmt.bitspersample;
			
			isValid = (nChannels == 1 || nChannels == 2) &&
					  (byterate == (nChannels*samplerate*nBitsPerSample)/8) &&
					  (blockalign == (nChannels*nBitsPerSample)/8);
		}
		else{
			cout<<"read wave file failure...."<<endl;
		}
	}
	~WaveFile()
	{
		if (fWav != NULL) {
			fclose(fWav); 
			delete wavheader;
		}
	}
	bool isValidwav(void){ return isValid;	}
	int nChan(void){return nChannels;}
	int nBits(void){return nBitsPerSample;}

	int ReadSamples(int num, int16_t *sarray[MAX_CHANNELS]);
	void printheader();
	int outWav(const char *newWav, const char* pcmfile);
};

//define Coeffilter Class
class Coeffilter
{	
	ifstream pCoeff;
	int16_t *coeffs;
	int SIZE;

  public:
	Coeffilter(){ SIZE = 0; }
	Coeffilter(const char* fName);
	~Coeffilter()
	{
		if(pCoeff.is_open()) pCoeff.close();		
		if(SIZE !=0 ) delete [] coeffs;
	}
		
	int length() {return SIZE;}
	int16_t * getcoeffs() {return coeffs;}
	void printout(); // printout coeffs data for debug
};

#endif /* WAVFIX_H_ */
