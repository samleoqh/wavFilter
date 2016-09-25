#include "wavfix.h"

#define DEBUG

//#define FILTER_LEN 63
#define MAX_INPUT_LEN    80
// maximum length of filter than can be handled
#define MAX_FLT_LEN      63
// buffer to hold all of the input samples
#define BUFFER_LEN       (MAX_FLT_LEN - 1 + MAX_INPUT_LEN)
// array to hold input samples
// number of samples to read per loop
#define SAMPLES     80


int16_t insamp[ BUFFER_LEN ];

void firFixedInit( void );

void firFixed( int16_t *coeffs, int16_t *input, int16_t *output,
			   int length, int filterLength );
			   
int firFixedOut(const char* inpcm,const char* outpcm, 
				int16_t *coeffs, int LEN);

int main (int argc, char* argv[])
{
	const char *inwav, *incoeff, *inpcm;
	const char *outpcm = "outputFixed.pcm";	
	bool isWav, isPcm;
	string input1, input2;	
	
	switch ( argc )
	{
		case 1:
			cout<<"pease enter pcm or wav filename: ";
			getline(cin, input1);
			inwav = input1.c_str();	
			cout<<"pease enter coeficient filename: ";
			getline(cin, input2);
			incoeff= input2.c_str();
			break;			
		case 2:	
			input1 = inwav = argv[1];		
			cout<<"pease enter coeficient filename: ";
			getline(cin, input2);
			incoeff= input2.c_str();			
			break;
		case 3:
			input1 = inwav = argv[1];
			input2 = incoeff = argv[2];	
			break;
		case 4:
			input1 = inwav = argv[1];
			input2 = incoeff = argv[2];
			outpcm = argv[3];
			break;
		default:
			break;
	}
	
	isWav = input1.find(".wav") < input1.length();
	isPcm = input1.find(".pcm") < input1.length();

	Coeffilter mycoeff(incoeff);
	int COEF_LEN = mycoeff.length();
	int16_t *coeffs = mycoeff.getcoeffs();
	
	if(isPcm)
	{
		inpcm = inwav;
		firFixedOut(inpcm, outpcm, coeffs, COEF_LEN);
	}
	else if(isWav)
	{
		WaveFile wav(inwav);
		
		if (!wav.isValidwav()) {
			cout<<"the wave file : "<<inwav<<" is not a valid wave format file!"<<endl;
			return -1;
		}		
		
		FILE    *out_fid;
		out_fid = fopen( outpcm, "w+" );
		if ( out_fid == 0 ) {
			cout<<"couldn't open : " << outpcm << endl;
			return -1;
		}
				
		int16_t *Channel[MAX_CHANNELS];
		int16_t *outChan[MAX_CHANNELS];			
		int size = 0;
		int nBytes = (wav.nBits() == 16) ? 2 : 1;
		bool istereo = (wav.nChan() == 2)? 1 : 0;
		Channel[0] = new int16_t[SAMPLES]();
		Channel[1] = new int16_t[SAMPLES]();
		outChan[0] = new int16_t[SAMPLES]();
		outChan[1] = new int16_t[SAMPLES]();
		
		do {
			size = wav.ReadSamples(SAMPLES, Channel);

			firFixed( coeffs, Channel[0], outChan[0], size, COEF_LEN );
			if (istereo) 
				firFixed( coeffs, Channel[1], outChan[1], size, COEF_LEN );
			
			for (int i =0; i < size; i++)
			{
				fwrite( &outChan[0][i], nBytes, 1, out_fid );
				if (istereo) 
					fwrite( &outChan[1][i], nBytes, 1, out_fid );
			}								
		} while ( size >0);
		
		delete [] Channel[0];
		delete [] Channel[1];
		delete [] outChan[0];
		delete [] outChan[1];

		fclose( out_fid );		
 
	#ifdef DEBUG 
		mycoeff.printout();	
		wav.printheader();
		
		string oldw = inwav, newfile="fixed_";
		newfile+=oldw;
		wav.outWav(newfile.c_str(), outpcm);
	#endif
 	}else 
	{
		cout<<"wrong files without any suffixe. "<<endl;
		return -1;
	}
	
	return 0;
}

int firFixedOut(const char* inpcm, const char* outpcm, int16_t *coeffs, int LEN)
{
	int size;
	int16_t input[SAMPLES];
	int16_t output[SAMPLES];
	FILE    *in_fid;
	FILE    *out_fid;
	//open the .pcm file
    in_fid = fopen( inpcm, "r" );
    if ( in_fid == 0 ) {
    	cout<<"couldn't read pcm data: " << endl;
        return -1;
    }

    // open the output waveform file
	out_fid = fopen( outpcm, "w+" );
	if ( out_fid == 0 ) {
    	cout<<"couldn't open : " << outpcm << endl;
        return -1;
    }
    // initialize the filter
    firFixedInit(); 					
    // process all of the samples
    do {
    	// read samples from file
    	size = fread( input, sizeof(int16_t), SAMPLES, in_fid );
    	// perform the filtering
    	firFixed( coeffs, input, output, size, LEN );
    	// write samples to file
    	fwrite( output, sizeof(int16_t), size, out_fid );
    } while ( size != 0 );
    
    //fclose( in_fid );
    fclose( out_fid );
    
    return 0;
}

void firFixedInit( void )
{
    memset(insamp, 0, sizeof( insamp ) );
}

// the FIR filter function
void firFixed( int16_t *coeffs, int16_t *input, int16_t *output, int length, int filterLength )
{
    int32_t acc;      // accumulator for MACs
    int16_t *coeffp; // pointer to coefficients
    int16_t *inputp; // pointer to input samples
    int n;
    int k;
    // put the new samples at the high end of the buffer
    memcpy( &insamp[filterLength - 1], input,
            length * sizeof(int16_t) );
    // apply the filter to each input sample
    for ( n = 0; n < length; n++ ) {
        // calculate output n
        coeffp = coeffs;
        inputp = &insamp[filterLength - 1 + n];
        // load rounding constant
        acc = 1 << 14;
        // perform the multiply-accumulate
        for ( k = 0; k < filterLength; k++ ) {
            acc += (int32_t)(*coeffp++) * (int32_t)(*inputp--);
        }
        // saturate the result
        if ( acc > 0x3fffffff ) {
            acc = 0x3fffffff;
        } else if ( acc < -0x40000000 ) {
            acc = -0x40000000;
        }
        // convert from Q30 to Q15
        output[n] = (int16_t)(acc >> 15);
    }
     // shift input samples back in time for next time
     memmove( &insamp[0], &insamp[length],
              (filterLength - 1) * sizeof(int16_t) );
}



