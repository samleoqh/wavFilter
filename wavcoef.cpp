#include "wavfix.h"

int WaveFile::ReadSamples(int num, int16_t *sarray[MAX_CHANNELS])
{
	int Size;
	int nChan = nChannels;
	int nByte = (nBitsPerSample==16) ? 2 : 1;
	int i = 0;
	bool unend = 1;
	
	for ( i=0; i < num && unend; i++ )  
	{
		unend = (fread(&sarray[0][i],nByte,1,fWav) == 1);
		
		if((nChan == 2) && unend){
			unend = (fread(&sarray[1][i],nByte,1,fWav) == 1);
		}						
	}
	
	Size = unend ? i : (i-1);
	return Size;
}

// put wav header and output pcm data together to make a new wav 
int WaveFile::outWav(const char *newWav, const char* pcmfile)
{
	FILE* newfile;
	if ((newfile = fopen(newWav, "wb")) != 0) 
	{
		fwrite(wavheader, 1, sizeof(Wav_Header), newfile);
		
		FILE* datafile;
		if ((datafile = fopen(pcmfile, "r")) != 0)
		{
			uint16_t B_SIZE = 1024;
			uint8_t* buffer = new uint8_t[B_SIZE](); 
			size_t byteread = 0;
			while ((byteread = fread(buffer, 1, B_SIZE, datafile)) >0 )
			{
				fwrite(buffer, sizeof buffer[0], byteread, newfile);
			}			
			delete [] buffer;
			fclose(datafile);
		}		
		fclose(newfile);
	}
	
	return 0;	
}


void WaveFile::printheader()
{
	if (fWav!=0)
	{
		cout<<"------------wav header information-----------"<<endl;
		cout<<"Riff_Chunk_Header------ "<<"chunkID: "
		<<wavheader->riff.ckID[0]<<wavheader->riff.ckID[1]
		<<wavheader->riff.ckID[2]<<wavheader->riff.ckID[3]<<endl;
		cout<<"chunkSize:	"<<wavheader->riff.cksize <<endl;
		cout<<"format:		"
		<<wavheader->riff.format[0]<<wavheader->riff.format[1]
		<<wavheader->riff.format[2]<<wavheader->riff.format[3]<<endl;
		cout<<"Fmt_Chunk_Header------- "<<"chunkID: "
		<<wavheader->fmt.ckid[0]<<wavheader->fmt.ckid[1]
		<<wavheader->fmt.ckid[2]<<wavheader->fmt.ckid[3]<< endl;
		cout<<"chunkSize:	"<<wavheader->fmt.cksize<< endl;
		cout<<"audioFormat:	"<<wavheader->fmt.audioformat<< endl;
		cout<<"numChannels:	"<<wavheader->fmt.numchannels<< endl;
		cout<<"sampleRate:	"<<wavheader->fmt.samplerate<< endl;
		cout<<"byteRate:	"<<wavheader->fmt.byterate<< endl;
		cout<<"blockAlign:	"<<wavheader->fmt.blockalign<< endl;
		cout<<"bitsPerSample:	"<<wavheader->fmt.bitspersample<<endl;
		cout<<"Data_Chunk_Header------ "<<"chunkID: "	
		<<wavheader->data.ckid[0]<<wavheader->data.ckid[1]
		<<wavheader->data.ckid[2]<<wavheader->data.ckid[3]<<endl;
		cout<<"chunksize: 	 " << wavheader->data.cksize<<endl;
		cout<<"-------------------------------------------"<<endl;	
	}	
}

Coeffilter::Coeffilter(const char* fName)
{
	SIZE = 0;
	pCoeff.open(fName);
		
	if(pCoeff.is_open() && pCoeff.good()){
		int number;
		while (pCoeff>>number) { SIZE++; }
		
		pCoeff.clear();
		pCoeff.seekg(0);
		
		if(SIZE != 0) 
		{
			int i=0;
			coeffs = new int16_t[SIZE];			
			while ((i<SIZE) && (pCoeff>>coeffs[i])) { i++;}
		}
	}	
	else {
		cout<<"read filterfile "<<fName<<" failed."<<endl;	
	}

}

void Coeffilter::printout()
{
	int i=0; 
	cout<<"-------------------------------------------"<<endl;
	printf("coeffs[%d] = \n", SIZE);
	while(i < SIZE) 
	{	
		if(i == 0) cout<<"{"<<endl;
		cout<< coeffs[i] <<",	";
		if (((++i)%8 )== 0) cout<<endl;					
	}	
	cout<< endl <<"}" <<";"<<endl;		
}
