//#include <stdio.h>
//#include <math.h>
//#include <time.h>
//#include "ltc.h"
//#include "portaudio.h"
//#include "pa_ringbuffer.h"
//#include "pa_util.h"
//
//
//#define SAMPLERATE 48000
//#define LTCFRAMERATE 25					
//#define LTCFRAMECOUNT 25
//#define BUFFERCOUNT 4
//#define CHANNELS 1	
//
//typedef long ring_buffer_size_t;
//typedef unsigned char SAMPLE;
//#define FILE_NAME       "audio_data.raw"
//#define SAMPLE_RATE 44100
//#define FRAMES_PER_BUFFER 512
//#define NUM_SECONDS 10
//#define NUM_CHANNELS 2
//#define NUM_WRITES_PER_BUFFER 4
//
//typedef struct
// {
//	     unsigned            frameIndex;
//	     int                 threadSyncFlag;
//	     SAMPLE             *ringBufferData;
//	     PaUtilRingBuffer    ringBuffer;
//	     FILE               *file;
//	     void               *threadHandle;
//	 }
// paTestData;