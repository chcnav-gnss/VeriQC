/**********************************************************************//**
		VeriQC

	   Common Module
*-
@file GNSSSignal.c
@author CHC
@date 2022/04/15
@brief define GNSS signal

**************************************************************************/
#include <string.h>

#include "CommonFuncs.h"
#include "GNSSSignal.h"

/** notice might be different with the FreqChannel broadcast in MSM7 */
static unsigned char s_GLOFreqChMap[28] = {
	 8, 3,12,13, 8, 3,12,13, /**< plane 1*/
	 5, 0, 7, 6, 5, 0, 7, 6, /**< plane 2*/
	11, 4,10, 9,11, 4,10, 9, /**< plane 3*/
	11, 2,14, 0 };

/** if the freq num of Glonass is valid update, 1 bit for one sat*/
#if _RTCM_POST_PROJECT_
static unsigned int s_GLOFreqChValidMask = 0xFFFFFFFF; /*default enable*/
#else
static unsigned int s_GLOFreqChValidMask = 0;
#endif

/**********************************************************************//**
@brief get the glonass frequency channel mapping array

notice only MSM5 and MSM7 has ExtSatInfo which indicate the FreqChannel number
for specific PRN

other MSM type need a preset s_GLOFreqChMap
in case the map array need to change, use SetGloFreqChMap to alter

@return the mapping array head pointer

@author CHC
@date 2022/04/20 14:22:50
@note
History:
- 2022.04.20 Startup
**************************************************************************/
unsigned char* GetGloFreqChMap(void)
{
	return s_GLOFreqChMap;
}

/**********************************************************************//**
@brief get the glonass frequency channel number from PRN (1 based)

@param SatID [In] 1 based

@return glonass frequency channel number + 7

@author CHC
@date 2022/04/16 14:22:50
@note
History:
- 2022.04.22 fix a bug about SatID input 0
**************************************************************************/
unsigned char GetGloFreqCh(unsigned int SatID)
{
	unsigned int Index = SatID - 1;
	if (Index >= sizeof(s_GLOFreqChMap) / sizeof(s_GLOFreqChMap[0]))
	{
		return 0;
	}
	return s_GLOFreqChMap[Index];
}


/**********************************************************************//**
@brief get the carrier wavelength by the FreqID

@param FreqID 	  [In] FreqID, could the any level beyond that
@param GloFreqNum [In] glonass frequency number, unsigned, 0 based, Offset 0

@return  carrier wavelength

@author CHC
@date 2022/04/22 19:52:10
@note
History:\n
- 2022.04.22 Start up
**************************************************************************/
double GetCarrierWaveLength(unsigned int FreqID, unsigned int GloFreqNum)
{
	const static double s_CarrierWaveLengthMap[TOTAL_EX_GNSS_SYSTEM_NUM][MAX_FREQ_INDEX_NUM] =
	{
		[GPS] = {LIGHT_SPEED / GPS_FREQ_L1,      LIGHT_SPEED / GPS_FREQ_L2,      LIGHT_SPEED / GPS_FREQ_L5,      0,                       LIGHT_SPEED / GPS_FREQ_L6 },
		[GLO] = {LIGHT_SPEED / GLO_FREQ_G1_BASE, LIGHT_SPEED / GLO_FREQ_G2_BASE, LIGHT_SPEED / GLO_FREQ_G3, 	   LIGHT_SPEED / GLO_FREQ_G1A,LIGHT_SPEED / GLO_FREQ_G2A },
		[GAL] = {LIGHT_SPEED / GAL_FREQ_E1,      LIGHT_SPEED / GAL_FREQ_E5A,     LIGHT_SPEED / GAL_FREQ_E5B,     LIGHT_SPEED / GAL_FREQ_E5, LIGHT_SPEED / GAL_FREQ_E6 },
		[BDS] = {LIGHT_SPEED / BDS_FREQ_B1,      LIGHT_SPEED / BDS_FREQ_B3,      LIGHT_SPEED / BDS_FREQ_B2,      LIGHT_SPEED / BDS_FREQ_L1, LIGHT_SPEED / BDS_FREQ_B2A},
		[QZS] = {LIGHT_SPEED / GPS_FREQ_L1,      LIGHT_SPEED / GPS_FREQ_L2,      LIGHT_SPEED / GPS_FREQ_L5,      0,                       LIGHT_SPEED / GPS_FREQ_L6 },
		[SBA] = {LIGHT_SPEED / GPS_FREQ_L1,      0,                            LIGHT_SPEED / GPS_FREQ_L5,      0,                       0 },
		[NIC] = {LIGHT_SPEED / NIC_FREQ_I5,      0,                            						0,      0,                       0 },
	};

	const static double s_GloWaveLengthMap[2][14] = {
		{GLO_WAVELENGTH_G1(0),GLO_WAVELENGTH_G1(1),GLO_WAVELENGTH_G1(2),GLO_WAVELENGTH_G1(3),GLO_WAVELENGTH_G1(4),GLO_WAVELENGTH_G1(5),GLO_WAVELENGTH_G1(6),
		 GLO_WAVELENGTH_G1(7),GLO_WAVELENGTH_G1(8),GLO_WAVELENGTH_G1(9),GLO_WAVELENGTH_G1(10),GLO_WAVELENGTH_G1(11),GLO_WAVELENGTH_G1(12),GLO_WAVELENGTH_G1(13)},
		{GLO_WAVELENGTH_G2(0),GLO_WAVELENGTH_G2(1),GLO_WAVELENGTH_G2(2),GLO_WAVELENGTH_G2(3),GLO_WAVELENGTH_G2(4),GLO_WAVELENGTH_G2(5),GLO_WAVELENGTH_G2(6),
		 GLO_WAVELENGTH_G2(7),GLO_WAVELENGTH_G2(8),GLO_WAVELENGTH_G2(9),GLO_WAVELENGTH_G2(10),GLO_WAVELENGTH_G2(11),GLO_WAVELENGTH_G2(12),GLO_WAVELENGTH_G2(13)},
	};
	unsigned int System = SYSTEM(FreqID);
	unsigned int FreqIndexOneSys = FREQ_ONE_SYS(FreqID);

	if ((System < TOTAL_EX_GNSS_SYSTEM_NUM) && (FreqIndexOneSys < MAX_FREQ_INDEX_NUM))
	{
		if ((GLO == System) && (FreqIndexOneSys < G3))
		{
			if (GloFreqNum >= 14)
			{
				return 0;
			}

			return s_GloWaveLengthMap[FreqIndexOneSys][GloFreqNum];
		}
		else
			return s_CarrierWaveLengthMap[System][FreqIndexOneSys];
	}

	return 0;
}

/**********************************************************************//**
@brief get short system name from extended Signal ID

@param ExSystemID [In] extended Signal ID
@return system name string

@author CHC
@date 2022/04/25 19:52:10
@note
History:\n
- 2022.04.25 Start up
**************************************************************************/
const char* GetSystemNameString(unsigned int ExSystemID)
{
	/** MUST be 3 characters */
	static const char* s_pSysName[TOTAL_EX_GNSS_SYSTEM_NUM] =
	{
		[GPS] = "GPS",
		[GLO] = "GLO",
		[GAL] = "GAL",
		[BDS] = "BDS",
		[SBA] = "SBA",
		[QZS] = "QZS",
		[NIC] = "NIC",
	};

	if (ExSystemID >= TOTAL_EX_GNSS_SYSTEM_NUM)
	{
		return "???";
	}
	return s_pSysName[ExSystemID];
}
