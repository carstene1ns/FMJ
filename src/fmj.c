/*****************************************************
  FULL METAL JACKET start up C code
******************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#include <i86.h>
#include <dos.h>

#include "modplay.h"

#define MAXFX 20

struct config_data {
		short sound_card;
		short addr;
		short IRQ;
		short DMA;
		char  dummy1;
		short dummy2;
}config = {0,0x220,5,1,0,0};

extern  int mission_no;
extern  int replay;
extern  int startASM(void);
extern  int FindSqrt(int);

extern  int ResolutionAdjust;               // 해상도 조절 변수.(외부에서 사용함)
extern  int ScreenSizeAdjust;               // 화면 크기 조절 변수.(외부에서 사용함)
extern  int BrightAdjust;                   // 밝기 조절 변수.(외부에서 사용함)
extern  int EffectAdjust;                   // 효과음 조절 변수.(외부에서 사용함)
extern  int MusicAdjust;                    // 음악 조절 변수.(외부에서 사용함)

extern	void vid_mode(int);

void SoundFX(unsigned number);
extern  Module * SONGptr_;

int SOUND;
int channel = 4;

unsigned char *wavfn[MAXFX] ={
			  "sounds/fmj01.wav",   // 0  _VALCAN_
			  "sounds/fmj02.wav",       // 1  _DING_
			  "sounds/fmj03.wav",       // 2  _EXPLO0_
			  "sounds/click3.wav",      // 3  _ARROW_
			  "sounds/click10.wav",     // 4  _ESC_
			  "sounds/sfx01.wav",       // 5  _ENTER_
			  "sounds/siren1.wav",      // 6  _SIREN_
			  "sounds/band.wav",        // 7  _BAND_
			  "sounds/sld.wav",         // 8  _SLD_
			  "sounds/gundry.wav",      // 9  _GUNDRY_
			  "sounds/explo1.wav",      // 10 _EXPLO1_
			  "sounds/explo2.wav",      // 11  _EXPLO2_
			  "sounds/sgunsh.wav",      // 12  _SGUNSH_
			  "sounds/sgunac.wav",      // 13  _SGUNAC_
			  "sounds/cannon.wav",      // 14  _CANNON_
			  "sounds/fthrow.wav",      // 15  _FTHROW_
			  "sounds/mchgun.wav",      // 16  _MGUN_
			  "sounds/drop.wav",        // 17  _DROP_
			  "sounds/buston.wav",      // 18  _BUSTON_
			  "sounds/bust.wav"};       // 19  _BUST_

Sample *FX[MAXFX];
word Port;
byte IRQ,DRQ;

void Intro(void)
{
	vid_mode(0x03);

	printf("FULL METAL JACKET Version 1.02e\n");
	printf("Action Simulation Game.\n");
	printf("Copyright (c) 1995, 1996 MIRINAE Software, Inc.\n\n");
}

void LoadFX(void)
{
    int i;

    for(i = 0 ; i < MAXFX ; i++){
	if ( (FX[i] = MODLoadSample(wavfn[i])) != NULL ) {
	} else printf("\nWAVE file loading Error\n!");
    }
}

void LoadCFG(void)
{
	FILE * fp;
	int buffer[5];

	fp = fopen("fmj.cfg","rb");

	if(fp == NULL)
	{
		ResolutionAdjust = 0;
		ScreenSizeAdjust = 0;
		BrightAdjust = 0;
		EffectAdjust = 0;
		MusicAdjust = 0;
		return;
	}

	fread(buffer,4,5,fp);

	ResolutionAdjust = buffer[0];
	ScreenSizeAdjust = buffer[1];
	BrightAdjust = buffer[2];
	EffectAdjust = buffer[3];
	MusicAdjust = buffer[4];

	fclose(fp);

}

void LoadConfig(void)
{
	FILE * fp;

	fp = fopen("config.cfg","rb");

	if(fp == NULL)
	{
		config.sound_card = 1;
		return;
	}

	fread(&config,sizeof(config),1,fp);

	fclose(fp);

}

void SaveCFG(void)
{
	FILE * fp;
	int buffer[5];

	fp = fopen("fmj.cfg","wb");

	buffer[0] = ResolutionAdjust;
	buffer[1] = ScreenSizeAdjust;
	buffer[2] = BrightAdjust;
	buffer[3] = EffectAdjust;
	buffer[4] = MusicAdjust;

	fwrite(buffer,4,5,fp);

	fclose(fp);

}

void SoundFX(unsigned number)
{
	if(SOUND)
	{
		MODPlaySample(channel,FX[number]);
		channel++;
		if( channel >= 8 ) channel = 4;
	}
}

PlayBGM(Module *Modulefile)
{
	if(SOUND)
	MODPlayModule(Modulefile,8,11111,Port,IRQ,DRQ,PM_TIMER);
}

main(int argc, char *argv[])
{
    Module *Song;
    int i;

    if( argc > 1 )
    {
	if( argv[1][0] == 'r' || argv[1][0] == 'R')
		replay = 1;
	else    replay = 0;

    }
    else {

	replay = 0;
    }

    Intro();

    LoadCFG();

    LoadConfig();

    switch(config.sound_card) {

	case 0 :
		SOUND = 0;
		printf("NO Sound!\n");
		delay(2000);
//		vid_mode(0x03);
//		system("logo.exe");

		startASM();
		break;

	case 1 :
		Port = (word)config.addr;
		IRQ = (byte)config.IRQ;
		DRQ = (byte)config.DMA;
		SOUND = 1;
		LoadFX();
		printf("Sound Blaster set\n");
		printf("Addr:%03x\n",Port);
		printf("IRQ:%d\n",IRQ);
		printf("DMA:%d\n",DRQ);
		delay(2000);
//		vid_mode(0x03);
//		system("logo.exe");

		startASM();
		break;

	case 2 :
		if (MODDetectCard(&Port,&IRQ,&DRQ))
		{
			SOUND = 0;
			printf("Sound Blaster not detected.\n");
			printf("NO Sound!\n");
			delay(2000);
//			vid_mode(0x03);
//			system("logo.exe");

			startASM();
		}
		else {
			SOUND = 1;
			LoadFX();
			printf("Sound Blaster detected\n");
			printf("Addr:%03x\n",Port);
			printf("IRQ:%d\n",IRQ);
			printf("DMA:%d\n",DRQ);
			delay(2000);
//			vid_mode(0x03);
//			system("logo.exe");

			startASM();
		}
		break;

    }

    SaveCFG();
    printf("\nThanks for playing FULL METAL JACKET Version 1.02e \n");

}

