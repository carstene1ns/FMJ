
#ifndef DEFS_H
#define DEFS_H

//= Typedef ==========================================================

typedef unsigned char  Byte;
typedef unsigned short Word;
typedef unsigned int   DWord;

//- PCX Header -
typedef struct {
	Byte maker;             // PCX이면 항상 10임.
	Byte version;           // PCX 버전.
	Byte code;              // RLE 압축이면 1, 아니면 0.
	Byte bpp;               // 픽셀당 비트수.
	Word x1, y1, x2, y2;    // 화면 좌표.
	Word hres, vres;        // 수평 해상도, 수직 해상도.
	Byte pal16[48];         // 16색상.
	Byte vmode;             // 비디오 모드 번호.
	Byte nplanes;           // 컬러 플레인의 개수. 256이면 8임.
	Word bpl;               // 라인당 바이트 수.
	Word palinfo;           // 팔레트 정보.
	Word shres, svres;      // 스캐너의 수평, 수직 해상도.
	Byte unused[54];        // 사용하지 않음.
} PCXHDR;

//- 스프라이트 저장 구조체 -
typedef struct {
	Byte *PartMem;
	Word TotalSize;
} SpriteMem;

typedef struct {
	Byte *SData;
	Word ex;
	Word ey;
} SpriteMem2;

//- FMJ 무기 저장 구조체.
typedef struct {
	Byte *WeapMem;
	Word SizeX, SizeY;
	Word TotalSize;
	Word WeapCost;
	Word WeapWeight;
} WeaponMem;

//- FMJ 주인공이 가지고 있는 아이템.
typedef struct {
	Word ArmsFlag;
	Word ArmsCnt;
} HostWeapon;

//- FMJ의 데이타를 로드해서 저장하는 구조체.
typedef struct {
	Byte FName[20];
	int  Mission;
} FMJSaveData;

#endif
