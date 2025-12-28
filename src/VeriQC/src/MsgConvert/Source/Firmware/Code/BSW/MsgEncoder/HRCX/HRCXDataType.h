/**********************************************************************//**
		VeriQC

		HRCX Encoder Module
*-
@file   HRCXDataType.h
@author CHC
@date   2024/02/27
@brief

**************************************************************************/
#ifndef _HRCX_DATA_TYPE_H_
#define _HRCX_DATA_TYPE_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _HRCX_BESTPOS_INFO_T
{
	unsigned short	Week;
	double			Sec;
	int				PosType;
	double			Lat; /**< unit: rad */
	double			Lon; /**< unit: rad */
	double			Height; /**< unit: meter */
	float			LatSigma; /**< unit: meter */
	float			LonSigma; /**< unit: meter */
	float			HeightSigma; /**< unit: meter */
	float			PosRMS;
	int				RefID;
	double			RefX;
	double			RefY;
	double			RefZ;
	float			Age;
	float			HDOP;
	float			VDOP;
	float			TDOP;
	float			PDOP;
	int				Ns;
	unsigned int	GPSSatUsedMask; /**< bit31:Sat32, bit0: Sat1*/
	UINT64			BDSSatUsedMask; /**< bit63:Sat64, bit0: Sat1*/
	UINT64			GALSatUsedMask; /**< bit63:Sat64, bit0: Sat1*/
	unsigned int	GLOSatUsedMask; /**< bit31:Sat32, bit0: Sat1*/
} HRCX_BESTPOS_INFO_T;

typedef struct _HRCX_POSSTATUS_EX_T
{
	unsigned short	HRCXVersion;
	unsigned short	PosStatusExVersion;
	unsigned short	GPSWeek;
	unsigned int	GPSSec;
	unsigned char	PosType;
	unsigned char	SWASIntegrityMark;
	unsigned short	ProviderID;
	float			COVxx;
	float			COVyy;
	float			COVzz;
	float			COVxy;
	float			COVyz;
	float			COVzx;
	unsigned char	Reserved[32];
} HRCX_POSSTATUS_EX_T;

#if !defined( __GCC__)
#pragma pack(push,1)
#endif

typedef struct _HRCX_BESTPOS_INFO_BODY_T
{
	unsigned short	MsgID;
	unsigned short	Week;
	unsigned int	Sec;
	unsigned char	PosType;
	INT64			Lat; /**< unit: rad */
	INT64			Lon; /**< unit: rad */
	INT64			Height; /**< unit: meter */
	unsigned int 	LatSigma; /**< unit: meter */
	unsigned int 	LonSigma; /**< unit: meter */
	unsigned int 	HeightSigma; /**< unit: meter */
	unsigned int 	PosRMS;
	int 			RefID;
	INT64			RefX;
	INT64			RefY;
	INT64			RefZ;
	int 			Age;
	unsigned int 	HDOP;
	unsigned int 	VDOP;
	unsigned int 	TDOP;
	unsigned int 	PDOP;
	unsigned char	Ns;
	unsigned int	GPSSatUsedMask; /**< bit31:Sat32, bit0: Sat1*/
	UINT64			BDSSatUsedMask; /**< bit63:Sat64, bit0: Sat1*/
	UINT64			GALSatUsedMask; /**< bit63:Sat64, bit0: Sat1*/
	unsigned int	GLOSatUsedMask; /**< bit31:Sat32, bit0: Sat1*/
} _PACKED_ HRCX_BESTPOS_INFO_BODY_T;

#if !defined( __GCC__)
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif /** _HRCX_DATA_TYPE_H_ */

