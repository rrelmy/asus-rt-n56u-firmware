/*
	spectrum.c

	    This program executes commands and outputs the command results.
	When getting SIGUSR1 signal, it will ...
	When getting SIGUSR2 signal, it will ...

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <stdint.h>
#include <syslog.h>

#include <bcmnvram.h>
#include <shutils.h>

#include <shared.h>


#define ADSL1_TONE 256
#define ADSL2_TONE 512

volatile int gotuser1 = 0;
volatile int gotterm = 0;

#define TMP_FILE_NAME_GET_BPC "/tmp/adsl/tc_bits_per_carrier.tmp"
#define TMP_FILE_NAME_GET_ADSL1 "/tmp/adsl/tc_snr_adsl1.tmp"
#define TMP_FILE_NAME_GET_ADSL2_PLUS "/tmp/adsl/tc_snr_adsl2_plus.tmp"



typedef enum{
	T1_413 = 0
	, G_LITE = 1
	, G_DMT = 2
	, ADSL2 = 3
	, ADSL2PLUS = 4
	, NOT_AVAILABLE = 5
}DSL_Modulation;


int getModulation(void)
{
	char buf[256] = {0};
	char *ptr = NULL;
	FILE *logFile = fopen( "/tmp/adsl/adsllog.log", "r" );
	int mode = 5;
	
	if( !logFile )
	{
		printf("Error: adsllog.log does not exist.\n");
		return;
	}
	while( fgets(buf, sizeof(buf), logFile) )
	{
		if( (ptr=strstr(buf, "Modulation :")) != NULL )
		{
			ptr += strlen("Modulation :")+1;
			mode = atoi(ptr);
			break;
		}
	}

	return mode;

}

static void execute_command(int option)
{
	char syscmd[128] = {0};

	if(option == 1)
	{
		sprintf(syscmd, "adslate getbpc" );
	}
	else if(option == 2)
	{
		sprintf(syscmd, "adslate getadsl1snr" );
	}
	else
	{
		sprintf(syscmd, "adslate getadsl2snr" );
	}
	system(syscmd);
}

static void write_snr1(FILE *f, float output[ADSL1_TONE])
{
	int i;

	fprintf(f, "\nspectrum_snr = {\n");
	for(i = 0; i < ADSL1_TONE; i++)
	{
		fprintf(f, "%.2f", output[i] );
		if( i != ADSL1_TONE-1 )
		{
			fprintf(f, ", " );
		}
	}
	fprintf(f, "};\n");
}

static void write_snr2(FILE *f, float output[ADSL2_TONE])
{
	int i;

	fprintf(f, "\nspectrum_snr = {\n");
	for(i = 0; i < ADSL2_TONE; i++)
	{
		fprintf(f, "%.2f", output[i] );
		if( i != ADSL2_TONE-1 )
		{
			fprintf(f, ", " );
		}
	}
	fprintf(f, "};\n");
}

//snr : signal to noise ratio
static void save_snr(void)
{
	FILE *f, *fsnr;
	int mode, i;
	static int runFlag = 0;
	char buf[256] = {0};
	float output1[ADSL1_TONE] = {0.0};
	float output2[ADSL2_TONE] = {0.0};

	if(runFlag == 1)
	{
		//skip this time.
		return;
	}

	mode = getModulation();
	if( mode == 5 ) //N/A
	{
		if ((f = fopen("/var/tmp/spectrum-snr", "w")) == NULL)
		{
			printf("Error: cannot open file: %s.\n", "/var/tmp/spectrum-snr" );
			runFlag = 0;
			return;
		}
		printf("Warn: adsl modem not up.\n");
		memset(output2, 0, ADSL2_TONE);
		write_snr2(f, output2);
		fclose(f);
		runFlag = 0;
		return;
	}
	else if( (mode == 4) || (mode == 3) ) //ADSL2+ or ADSL2
	{
		float snr;

		runFlag = 1;
		execute_command(3);

		if ((fsnr = fopen(TMP_FILE_NAME_GET_ADSL2_PLUS, "r")) == NULL)
		{
			printf("Error: cannot read %s.\n", TMP_FILE_NAME_GET_ADSL2_PLUS );
			runFlag = 0;
			return;
		}
		
		i=32; //first 32 tones(0~31) belong to upstream
		while( fgets(buf, sizeof(buf), fsnr) &&(i < ADSL2_TONE) )
		{
			sscanf(buf, "%f", &snr);
			output2[i++] = snr;
		}
		fclose(fsnr);

		if ((f = fopen("/var/tmp/spectrum-snr", "w")) == NULL)
		{
			printf("Error: cannot open file: %s.\n", "/var/tmp/spectrum-snr" );
			runFlag = 0;
			return;
		}
		write_snr2(f, output2);
		fclose(f);
		runFlag = 0;
		return;
	}
	else if( mode <= 2 ) //ADSL1
	{
		runFlag = 1;
		execute_command(2);

		if ((fsnr = fopen(TMP_FILE_NAME_GET_ADSL1, "r")) == NULL)
		{
			printf("Error: cannot read %s.\n", TMP_FILE_NAME_GET_ADSL1 );
			runFlag = 0;
			return;
		}
		
		i=32; //first 32 tones(0~31) belong to upstream
		while( fgets(buf, sizeof(buf), fsnr) &&(i < ADSL1_TONE) )
		{
			output1[i++] = atof(buf)/512.0;
		}
		fclose(fsnr);

		if ((f = fopen("/var/tmp/spectrum-snr", "w")) == NULL)
		{
			printf("Error: cannot open file: %s.\n", "/var/tmp/spectrum-snr" );
			runFlag = 0;
			return;
		}
		write_snr1(f, output1);
		fclose(f);
		runFlag = 0;
		return;
	}
	else
	{
		if ((f = fopen("/var/tmp/spectrum-snr", "w")) == NULL)
		{
			printf("Error: cannot open file: %s.\n", "/var/tmp/spectrum-snr" );
			runFlag = 0;
			return;
		}
		printf("Warn: unknown modulation type.\n");
		memset(output2, 0, ADSL2_TONE);
		write_snr2(f, output2);
		fclose(f);
		runFlag = 0;
		return;
	}

}

static void write_bpc1(FILE *f, unsigned int output[ADSL1_TONE])
{
	int i;

	if(!f) return;

	fprintf(f, "\nspectrum_bpc = {\n");
	for(i = 0; i < ADSL1_TONE; i++)
	{
		fprintf(f, "%d", output[i] );
		if( i != ADSL1_TONE-1 )
		{
			fprintf(f, ", " );
		}
	}
		fprintf(f, "};\n");

}

static void write_bpc2(FILE *f, unsigned int output[ADSL2_TONE])
{
	int i;

	if(!f) return;

	fprintf(f, "\nspectrum_bpc = {\n");
	for(i = 0; i < ADSL2_TONE; i++)
	{
		fprintf(f, "%d", output[i] );
		if( i != ADSL2_TONE-1 )
		{
			fprintf(f, ", " );
		}
	}
		fprintf(f, "};\n");

}

//bpc : bits per carrier
static void save_bpc(void)
{
	FILE *f, *fbpc;
	int mode, i, row;
	unsigned int bits = 0;
	static int runFlag = 0;
	char buf[256] = {0};
	unsigned int output1[ADSL1_TONE] = {0};
	unsigned int output2[ADSL2_TONE] = {0};

	if(runFlag == 1)
	{
		//skip this time.
		return;
	}

	mode = getModulation();
	if( mode == 5 ) //N/A
	{
		if ((f = fopen("/var/tmp/spectrum-bpc", "w")) == NULL)
		{
			printf("Error: cannot open file: %s.\n", "/var/tmp/spectrum-bpc" );
			runFlag = 0;
			return;
		}
		printf("Warn: adsl modem not up.\n");
		memset(output2, 0, ADSL2_TONE);
		write_bpc2(f, output2);
		fclose(f);
		runFlag = 0;
		return;
	}
	else if( (mode == 4) || (mode == 3) ) //ADSL2+ or ADSL2
	{
		runFlag = 1;
		execute_command(1);

		if ((f = fopen("/var/tmp/spectrum-bpc", "w")) == NULL)
		{
			printf("Error: cannot open file: %s.\n", "/var/tmp/spectrum-bpc" );
			runFlag = 0;
			return;
		}

		if ((fbpc = fopen(TMP_FILE_NAME_GET_BPC, "r")) == NULL)
		{
			printf("Error: cannot read %s.\n", TMP_FILE_NAME_GET_BPC );
			runFlag = 0;
			memset(output2, 0, ADSL2_TONE);
			write_bpc2(f, output2);
			fclose(f);
			return;
		}

		i = 0;
		while( fgets(buf, sizeof(buf), fbpc) && (i < ADSL2_TONE) )
		{
			sscanf(buf, "%x", &bits );
			output2[i++] = bits;
		}
		fclose(fbpc);
		write_bpc2(f, output2);
		fclose(f);
		runFlag = 0;
	}
	else if( mode <= 2 ) //ADSL1
	{
		runFlag = 1;
		execute_command(1);

		if ((f = fopen("/var/tmp/spectrum-bpc", "w")) == NULL)
		{
			printf("Error: cannot open file: %s.\n", "/var/tmp/spectrum-bpc" );
			runFlag = 0;
			return;
		}

		if ((fbpc = fopen(TMP_FILE_NAME_GET_BPC, "r")) == NULL)
		{
			printf("Error: cannot read %s.\n", TMP_FILE_NAME_GET_BPC );
			runFlag = 0;
			memset(output1, 0, ADSL1_TONE);
			write_bpc1(f, output1);
			fclose(f);
			return;
		}

		i = 0;
		while( fgets(buf, sizeof(buf), fbpc) && (i < ADSL1_TONE) )
		{
			sscanf(buf, "%x", &bits );
			output1[i++] = bits;
		}
		fclose(fbpc);
		write_bpc1(f, output1);
		fclose(f);
		runFlag = 0;
	}
	else
	{
		if ((f = fopen("/var/tmp/spectrum-bpc", "w")) == NULL)
		{
			printf("Error: cannot open file: %s.\n", "/var/tmp/spectrum-bpc" );
			runFlag = 0;
			return;
		}
		printf("Warn: adsl modem not up.\n");
		memset(output2, 0, ADSL2_TONE);
		write_bpc2(f, output2);
		fclose(f);
		runFlag = 0;
		return;
	}
	
}

static void sig_handler(int sig)
{
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		gotterm = 1;
		break;
	case SIGUSR1:
		gotuser1 = 1;
		break;
	}
}

int main(int argc, char *argv[])
{
	struct sigaction sa;

	printf("spectrum\nCopyright (C) 2012-2012 ASUSWRT\n\n");

	if (fork() != 0) return 0;

	unlink("/var/tmp/spectrum-snr");
	unlink("/var/tmp/spectrum-bpc");

	sa.sa_handler = sig_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	while (1) {
		sleep(30);
		if (gotterm) {
			exit(0);
		}
		if (gotuser1 == 1) {
			save_bpc();
			save_snr();
			gotuser1 = 0;
		}
	}
	return 0;
}
