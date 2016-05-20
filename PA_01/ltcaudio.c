#include <math.h>
#include "ltcaudio.h"
#include "rs232.h"
#include "timer.h"


hangisi = 0;
onceki = 0;

void doserial()
{	
	char mode[] = { '8', 'N', '1', 0 };
	if (RS232_OpenComport(0, 9600, mode))
	{
		printf("Can not open comport\n");
		return(1);
	}
}

void stopserial()
{
	//RS232_CloseComport(0);
//	if (simdik)free(simdik);
}

int AllocateBuffers(paData* pd, int bufsize)
{
	int j;
	for (int i = 0; i < BUFFERCOUNT; i++)
	{
		j = i;
		pd[i].IsLoaded = 1;
		pd[i].payLoad = (ltcsnd_sample_t*)calloc(bufsize, sizeof(ltcsnd_sample_t));	
	}
	return 0;
}

int lSetCurrentDateTime(SMPTETimecode *pst, LTCEncoder *pencoder)
{

	time_t t;
	struct tm * tmp;
	t = time(NULL);
	tmp = localtime(&t);
	char sene[4] = { 0 };
	char sene2[3] = { 0 };
	sprintf(sene, "%d", tmp->tm_year);
	memcpy(sene2, &sene[1], 2);
	const char timezone[6] = "+0200";
	strcpy(pst->timezone, timezone);
	pst->years = atoi(sene2);
	pst->months = tmp->tm_mon;
	pst->days = tmp->tm_mday;
	pst->hours = tmp->tm_hour;
	pst->mins = tmp->tm_min;
	pst->secs = tmp->tm_sec;
	pst->frame = 0;
	ltc_encoder_set_timecode(pencoder, pst);
	return 0;
}

int lSetCurrentTime(SMPTETimecode *pst, LTCEncoder *pencoder)
{
	time_t t;
	struct tm * tmp;
	t = time(NULL);
	tmp = localtime(&t);
	pst->hours = tmp->tm_hour;
	pst->mins = tmp->tm_min;
	pst->secs = tmp->tm_sec;
	pst->frame = 0;
	ltc_encoder_set_timecode(pencoder, pst);
	char * simdik = malloc(sizeof(char)* 100);
	strftime(simdik, strlen(simdik), "%H:%M:%S", tmp);
	printf("%s\n", simdik);
	return 0;
}

int LoadLtcData(paData* p, int pbufsize, LTCEncoder *pencoder)
{
	ltcsnd_sample_t *buf;
	memset(p->payLoad, 0, pbufsize);
	int len;
	for (int i = 0; i < LTCFRAMECOUNT; i++)
	{
		ltc_encoder_encode_frame(pencoder);
		buf = ltc_encoder_get_bufptr(pencoder, &len, 1);
		memcpy(p->payLoad + (len * i), buf, len);
		ltc_encoder_get_timecode(pencoder, &st);
		p->cst = st;
		ltc_encoder_inc_timecode(pencoder);
	}
	p->IsLoaded = 1;
	return 0;
}

LTCEncoder* lGetLtcEncoder()
{
	LTCEncoder *encoder;
	double fps = LTCFRAMERATE;
	double sample_rate = SAMPLERATE;
	encoder = ltc_encoder_create(sample_rate, fps, fps == 25 ? LTC_TV_625_50 : LTC_TV_525_60, LTC_USE_DATE);
	return encoder;
}

int prepareLtc()
{
	encoder = lGetLtcEncoder();
	if (!encoder)
	{
		printf("ltc encoder cannot retrieved \n");
		return 1;
	}
	lSetCurrentDateTime(&st, encoder);
	return 0;
}
	

int paStreamCallBack(const void *inputBuffer, void *outputBuffer, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{

	paData *pd = (paData*)userData;
	ltcsnd_sample_t* out = (ltcsnd_sample_t*)outputBuffer;
	(void)inputBuffer;

	if (!pd[hangisi].IsLoaded) LoadLtcData(&pd[hangisi], ltcbufsize, encoder);

	memcpy(out, pd[hangisi].payLoad, ltcbufsize);
	pd[hangisi].IsLoaded = 0;
		
	onceki = hangisi - 1;
	if (onceki < 0) onceki = 3;
	ss = pd[onceki].cst;
	sprintf(simdik, "*%02d%02d%02d\n", ss.hours, ss.mins, ss.secs);
	RS232_cputs(0,simdik);
	printf("%s\n", simdik);
	hangisi++;
	hangisi %= BUFFERCOUNT;
	return paContinue;

}

int pa()
{
	PaStreamParameters outputParameters;
	doserial();
	simdik = malloc(sizeof(char)* 7);

	err = Pa_Initialize();
	if (err != paNoError)
	{
		printf("ERROR: Pa_Initialize returned 0x%x\n", err);
		goto error;
	}

	if (prepareLtc()) goto error;

	ltcbufsize = ltc_encoder_get_buffersize(encoder) - 1;
	ltcbufsize = ltcbufsize * LTCFRAMECOUNT;
	
	pa_framesperbuffer = ltcbufsize / (1 * CHANNELS);

	if (AllocateBuffers(&pBuffers, ltcbufsize))
	{
		printf("ERROR: memory allocation failed\n");
		goto error;
	}

	//first time fill all buffer
	for (int i = 0; i < BUFFERCOUNT; i++)
	{
		LoadLtcData(&pBuffers[i], ltcbufsize, encoder);
	}

	memset(&outputParameters, 0, sizeof(PaStreamParameters));
	outputParameters.device = Pa_GetDefaultOutputDevice();
	if (outputParameters.device == paNoDevice) {
		fprintf(stderr, "Error: No default output device.\n");
		goto error;
	}

	outputParameters.channelCount = CHANNELS;
	outputParameters.sampleFormat = paUInt8;
	outputParameters.suggestedLatency = 0.050;
	outputParameters.hostApiSpecificStreamInfo = NULL;


	err = Pa_OpenStream(
		&stream,
		NULL,					// no input
		&outputParameters,
		SAMPLERATE,
		pa_framesperbuffer,
		paClipOff,
		paStreamCallBack,
		&pBuffers);
	if (err != paNoError) goto error;

	err = Pa_StartStream(stream);
	if (err != paNoError) goto error;

	while ((err = Pa_IsStreamActive(stream)) == 1) Pa_Sleep(1000);

	err = Pa_CloseStream(stream);
	Pa_Terminate();
	stopserial();
	stop_timer();
	return 0;

error:
	Pa_Terminate();
	fprintf(stderr, "Error number: %d\n", err);
	fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
	stopserial();
	return err;
}

static void PrintSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters)
{
	static double standardSampleRates[] = {
		8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
		44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
	};
	int     i, printCount;
	PaError err;

	printCount = 0;
	for (i = 0; standardSampleRates[i] > 0; i++)
	{
		err = Pa_IsFormatSupported(inputParameters, outputParameters, standardSampleRates[i]);
		if (err == paFormatIsSupported)
		{
			if (printCount == 0)
			{
				printf("\t%8.2f", standardSampleRates[i]);
				printCount = 1;
			}
			else if (printCount == 4)
			{
				printf(",\n\t%8.2f", standardSampleRates[i]);
				printCount = 1;
			}
			else
			{
				printf(", %8.2f", standardSampleRates[i]);
				++printCount;
			}
		}
	}
	if (!printCount)
		printf("None\n");
	else
		printf("\n");
}

int GetDeviceInfo()
{
	const PaDeviceInfo *deviceInfo;
	int devicesayisi = 0;


	int     structVersion;
	const char *    name;
	PaHostApiIndex  hostApi;
	int     maxInputChannels;
	int     maxOutputChannels;
	PaTime  defaultLowInputLatency;
	PaTime  defaultLowOutputLatency;
	PaTime  defaultHighInputLatency;
	PaTime  defaultHighOutputLatency;
	double  defaultSampleRate;
	PaError err;
	int defaultDisplayed = 0;
	PaStreamParameters inputParameters, outputParameters;

	err = Pa_Initialize();
	if (err != paNoError)
	{
		printf("ERROR: Pa_Initialize returned 0x%x\n", err);
		goto error;
	}

	printf("PortAudio version: 0x%08X\n", Pa_GetVersion());
	printf("Version text: '%s'\n", Pa_GetVersionText());

	devicesayisi = Pa_GetDeviceCount();
	if (devicesayisi < 0)
	{
		printf("ERROR: Pa_GetDeviceCount returned 0x%x\n", devicesayisi);
		err = devicesayisi;
		goto error;
	}
	printf("Number of devices = %d\n", devicesayisi);

	for (int i = 0; i < devicesayisi; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		printf("--------------------------------------- device #%d\n", i);
		if (i == Pa_GetDefaultOutputDevice())
		{
			printf((defaultDisplayed ? "," : "["));
			printf(" Default Output");
			defaultDisplayed = 1;
		}
		if (defaultDisplayed)  printf(" ]\n");

		printf("Name: %s\n", deviceInfo->name);
		printf("Host API                    = %s\n", Pa_GetHostApiInfo(deviceInfo->hostApi)->name);
		printf("Max inputs = %d", deviceInfo->maxInputChannels);
		printf(", Max outputs = %d\n", deviceInfo->maxOutputChannels);
		printf("Default low input latency   = %8.4f\n", deviceInfo->defaultLowInputLatency);
		printf("Default low output latency  = %8.4f\n", deviceInfo->defaultLowOutputLatency);
		printf("Default high input latency  = %8.4f\n", deviceInfo->defaultHighInputLatency);
		printf("Default high output latency = %8.4f\n", deviceInfo->defaultHighOutputLatency);
		printf("Default sample rate         = %8.2f\n", deviceInfo->defaultSampleRate);

		inputParameters.device = i;
		inputParameters.channelCount = deviceInfo->maxInputChannels;
		inputParameters.sampleFormat = paInt16;
		inputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
		inputParameters.hostApiSpecificStreamInfo = NULL;

		outputParameters.device = i;
		outputParameters.channelCount = deviceInfo->maxOutputChannels;
		outputParameters.sampleFormat = paInt16;
		outputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
		outputParameters.hostApiSpecificStreamInfo = NULL;

		//if (inputParameters.channelCount > 0)
		//{
		//	printf("Supported standard sample rates for half-duplex 16 bit %d channel input = \n",inputParameters.channelCount);
		//	PrintSupportedStandardSampleRates(&inputParameters, NULL);
		//}

		if (outputParameters.channelCount > 0)
		{
			printf("Supported standard sample rates for half-duplex 16 bit %d channel output = \n", outputParameters.channelCount);
			PrintSupportedStandardSampleRates(NULL, &outputParameters);
		}

		//if (inputParameters.channelCount > 0 && outputParameters.channelCount > 0)
		//{
		//	printf("Supported standard sample rates for full-duplex 16 bit %d channel input, %d channel output = \n",
		//		inputParameters.channelCount, outputParameters.channelCount);
		//	PrintSupportedStandardSampleRates(&inputParameters, &outputParameters);
		//}


		printf("----------------------------------------------\n");

	}
	Pa_Terminate();
	return 0;
error:
	Pa_Terminate();
	fprintf(stderr, "Error number: %d\n", err);
	fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
	return err;

}


