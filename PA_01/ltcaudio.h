#include <stdio.h>
#include <math.h>
#include <time.h>
#include "ltc.h"
#include "portaudio.h"



#define SAMPLERATE 48000
#define LTCFRAMERATE 25						// 25 fps 
#define LTCFRAMECOUNT 25
#define BUFFERCOUNT 4
#define CHANNELS 1							// mono



PaError err;
LTCEncoder* encoder;
SMPTETimecode st;
SMPTETimecode ss;
int ltcbufsize;
int hangisi;
PaTime pt;
LTCFrame lfrm;
PaStream *stream;
char *simdik;
int onceki;
int pa_framesperbuffer;


typedef struct paDataStruct
{
	int IsLoaded;
	ltcsnd_sample_t* payLoad;
	SMPTETimecode cst;
//	struct paDataStruct *next;
//	time_t oan;
	
} paData;

paData pBuffers[BUFFERCOUNT];

int AllocateBuffers(paData* pd, int bufsize);
int lSetCurrentDateTime(SMPTETimecode *pst, LTCEncoder *pencoder);
int lSetCurrentTime(SMPTETimecode *pst, LTCEncoder *pencoder);
int LoadLtcData(paData* p, int pbufsize, LTCEncoder *pencoder);
LTCEncoder* lGetLtcEncoder();
int prepareLtc();
int paStreamCallBack(const void *inputBuffer, void *outputBuffer, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
int pa();
static void PrintSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
int GetDeviceInfo();
void gettime(char * simdik);
void doserial();
void stopserial();
