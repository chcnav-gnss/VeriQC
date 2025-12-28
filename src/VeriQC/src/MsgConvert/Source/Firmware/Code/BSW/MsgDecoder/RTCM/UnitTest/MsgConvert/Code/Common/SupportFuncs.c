#ifdef MSG_CONVERT
/**********************************************************************//**
			 VeriQC

			 Common Module
 *-
 @file SupportFuncs.c
 @author CHC
 @date 2022/06/29

@brief common used operations and functions for all the module

**************************************************************************/
#include<math.h>
#include "SupportFuncs.h"
#include "Common/GNSSSignal.h"
#include "Decoder/Rinex/RinexDataTypes.h"
#include "GNSSConstants.h"
#include "TimeDefines.h"

#include "../MsgConvertDataTypes.h"

#include "BSW/MsgEncoder/RawMsg/RawMsgIDDefines.h"

typedef struct _RAWMSG_ID_INFO_T
{
	int RawMsgIndex;
	int RawMsgID;
} RAWMSG_ID_INFO_T;

typedef int (*GET_ID_FUNC)(int);

/**********************************************************************//**
@brief  get AllSatIndex from PRN, SatIndex is used for obs

@param  ExSystemID	  [In] GNSS system ID
@param  PRN			 [In] satellite prn/slot number

@return <0:error,>=0:SatIndex, (0 --- (MAX_SAT_NUM-1))

@author CHC
@date   2023/05/06
@note
**************************************************************************/
int GetAllSatIndex(unsigned int ExSystemID, unsigned int PRN)
{
	if (PRN == 0)
	{
		return -1;
	}
	switch (ExSystemID)
	{
		case GPS:
			if (PRN < MIN_GPS_PRN || PRN > MAX_GPS_PRN)
			{
				return -1;
			}
			return PRN - MIN_GPS_PRN;
		case GLO:
			if (PRN < MIN_GLO_SLOT || PRN > MAX_GLO_SLOT)
			{
				return -1;
			}
			return MAX_GPS_SAT_NUM + PRN - MIN_GLO_SLOT;
		case GAL:
			if (PRN < MIN_GAL_PRN || PRN > MAX_GAL_PRN)
			{
				return -1;
			}
			return MAX_GPS_SAT_NUM + MAX_GLO_SAT_NUM + PRN - MIN_GAL_PRN;
		case BDS:
			if (PRN < MIN_BDS_PRN || PRN > MAX_BDS_PRN)
			{
				return -1;
			}
			return MAX_GPS_SAT_NUM + MAX_GLO_SAT_NUM + MAX_GAL_SAT_NUM +  PRN - MIN_BDS_PRN;
		case SBA:
			if (!((PRN >= MIN_SBAS_GEO_PRN && PRN <= MAX_SBAS_GEO_PRN) || (PRN >= MIN_QZSBAS_PRN && PRN <= MAX_QZSBAS_PRN)))
			{
				return -1;
			}

			if (PRN >= MIN_SBAS_GEO_PRN && PRN <= MAX_SBAS_GEO_PRN)
			{
				return MAX_GPS_SAT_NUM + MAX_GLO_SAT_NUM +MAX_GAL_SAT_NUM + MAX_BDS_SAT_NUM + PRN - MIN_SBAS_GEO_PRN;
			}
			else if (PRN >= MIN_QZSBAS_PRN && PRN <= MAX_QZSBAS_PRN)
			{
				return MAX_GPS_SAT_NUM + MAX_GLO_SAT_NUM +MAX_GAL_SAT_NUM +  MAX_BDS_SAT_NUM + MAX_SBAS_GEO_SAT_NUM + PRN - MIN_QZSBAS_PRN;
			}
		case QZS:
			if (PRN < MIN_QZSS_PRN || PRN > MAX_QZSS_PRN)
			{
				return -1;
			}
			return MAX_GPS_SAT_NUM + MAX_GLO_SAT_NUM +MAX_GAL_SAT_NUM + MAX_BDS_SAT_NUM + MAX_SBAS_ALL_SAT_NUM + PRN - MIN_QZSS_PRN;
		case NIC:
			if (PRN < MIN_NIC_PRN || PRN > MAX_NIC_PRN)
			{
				return -1;
			}
			return MAX_GPS_SAT_NUM + MAX_GLO_SAT_NUM +MAX_GAL_SAT_NUM + MAX_BDS_SAT_NUM + MAX_SBAS_ALL_SAT_NUM + MAX_QZSS_SAT_NUM + PRN - MIN_NIC_PRN;
		default:
			return -1;
	}
}

/**********************************************************************//**
@brief translate rinex system to internal used system ID

@param RMRangeSystem [In] raw message system


@return ExSystemID

@author CHC
@date 2022/05/16
@note
**************************************************************************/
unsigned int RinexSystemToExSystem(int RindexSystem)
{
	const static unsigned int s_ExSystemMap[] =
	{
		GPS,
		GLO,
		GAL,
		BDS,
		SBA,
		QZS,
		NIC,
	};

	if (RindexSystem < (sizeof(s_ExSystemMap) / sizeof(unsigned int)))
	{
		return s_ExSystemMap[RindexSystem];
	}

	return NA8;
}

/**********************************************************************//**
@brief translate internal used system ID to rinex system

@param ExSystemID [In] internal used system ID

@return RinexSystem

@author CHC
@date 2022/05/16
@note
**************************************************************************/
unsigned int ExSystemToRinexSystem(int ExSystemID)
{
	const static unsigned int s_RinexSystemMap[] =
	{
		RINEX_SYS_GPS,
		RINEX_SYS_GLO,
		RINEX_SYS_GAL,
		RINEX_SYS_BDS,
		RINEX_SYS_SBA,
		RINEX_SYS_QZS,
		RINEX_SYS_NIC,
	};

	if (ExSystemID < (sizeof(s_RinexSystemMap) / sizeof(unsigned int)))
	{
		return s_RinexSystemMap[ExSystemID];
	}

	return NA8;
}

/**********************************************************************//**
@brief translate internal used system ID to rinex system index

@param ExSystemID [In] internal used system ID

@return Rinex System index

@author CHC
@date 2024/07/15
@note
**************************************************************************/
unsigned int ExSystemToRinexSysIndex(int ExSystemID)
{
	const static unsigned int s_RinexSystemMap[] =
	{
		RINEX_SYS_INDEX_GPS,
		RINEX_SYS_INDEX_GLO,
		RINEX_SYS_INDEX_GAL,
		RINEX_SYS_INDEX_BDS,
		RINEX_SYS_INDEX_SBA,
		RINEX_SYS_INDEX_QZS,
		RINEX_SYS_INDEX_NIC,
	};

	if (ExSystemID < (sizeof(s_RinexSystemMap) / sizeof(unsigned int)))
	{
		return s_RinexSystemMap[ExSystemID];
	}

	return NA8;
}

/**********************************************************************//**
@brief  HRCX type id to convert file index

@param TypeID        [In] HRCX type id

@retval <0:error >=0:convert file index

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int HRCXTypeIDToConvertFileIndex(int TypeID)
{
	const static int s_ConvertFileIndexMap[] =
	{
		MSG_CONVERT_FILE_INDEX_GPSEPH,/**< 32019*/
		MSG_CONVERT_FILE_INDEX_GLOEPH,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		-1,
		-1,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH, /**< 32046 */
	};

	if ((TypeID >= 32019) && (TypeID <= 32046))
	{
		return s_ConvertFileIndexMap[TypeID - 32019];
	}

	return -1;
}

/**********************************************************************//**
@brief  RTCM2 type id to convert file index

@param TypeID        [In] RTCM2 type id

@retval <0:error >=0:convert file index

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int RTCM2TypeIDToConvertFileIndex(int TypeID)
{
	if (TypeID == 17)
	{
		return MSG_CONVERT_FILE_INDEX_GPSEPH;
	}

	return -1;
}

/**********************************************************************//**
@brief  RTCM3 type id to convert file index

@param TypeID        [In] RTCM3 type id

@retval <0:error >=0:convert file index

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int RTCM3TypeIDToConvertFileIndex(int TypeID)
{
	const static int s_ConvertFileIndexMap[] =
	{
		MSG_CONVERT_FILE_INDEX_GPSEPH,/**< RTCM1019*/
		MSG_CONVERT_FILE_INDEX_GLOEPH,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		MSG_CONVERT_FILE_INDEX_NICEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		-1,
		MSG_CONVERT_FILE_INDEX_QZSSEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH, /**< RTCM1046 */
	};

	if ((TypeID >= 1019) && (TypeID <= 1046))
	{
		return s_ConvertFileIndexMap[TypeID - 1019];
	}

	return -1;
}

/**********************************************************************//**
@brief  RawMsg decode id to convert file index

@param DecodeID        [In] RawMsg decode id

@retval <0:error >=0:convert file index

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int RawMsgDecodeIDToConvertFileIndex(int DecodeID)
{
	const static int s_ConvertFileIndexMap[] =
	{
		-1,
		-1,
		-1,
		-1,
		MSG_CONVERT_FILE_INDEX_GPSEPH,
		MSG_CONVERT_FILE_INDEX_GLOEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_QZSSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_NICEPH,
		MSG_CONVERT_FILE_INDEX_BASEOBS,

		/** Huace raw msg */
		-1,
		-1,
		-1,
		-1,
		MSG_CONVERT_FILE_INDEX_GPSEPH,
		MSG_CONVERT_FILE_INDEX_GLOEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_QZSSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_NICEPH,

		MSG_CONVERT_FILE_INDEX_BDSEPH, /**< UN */
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		-1,
		-1,
		MSG_CONVERT_FILE_INDEX_BASEOBS,
		MSG_CONVERT_FILE_INDEX_REFSTATION,
		MSG_CONVERT_FILE_INDEX_GPSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_GLOEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,

		-1, /**< KMD */
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_GPSEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_GLOEPH,

		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
	};

	if (DecodeID < (sizeof(s_ConvertFileIndexMap)/sizeof(s_ConvertFileIndexMap[0])))
	{
		return s_ConvertFileIndexMap[DecodeID];
	}

	return -1;
}

/**********************************************************************//**
@brief  Rinex id to convert file index

@param RinexID        [In] Rinex id

@retval <0:error >=0:convert file index

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int RinexIDToConvertFileIndex(int RinexID)
{
	const static int s_ConvertFileIndexMap[] =
	{
		-1,
		MSG_CONVERT_FILE_INDEX_GPSEPH,
		MSG_CONVERT_FILE_INDEX_GLOEPH,
		MSG_CONVERT_FILE_INDEX_QZSSEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		-1,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_GALEPH,
		MSG_CONVERT_FILE_INDEX_NICEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		MSG_CONVERT_FILE_INDEX_BDSEPH,
		-1,
		MSG_CONVERT_FILE_INDEX_MUTIEPH,
	};

	if ((RinexID >= RINEX_ID_OBS) && (RinexID <= RINEX_ID_MUTIEPH))
	{
		return s_ConvertFileIndexMap[RinexID - RINEX_ID_OBS];
	}

	return -1;
}

/**********************************************************************//**
@brief  get file index

@param RawDataType        [In] raw data type
@param DecodeID           [In] decode id

@retval <0:error >=0: file index

@author CHC
@date 2024/07/18
@note
**************************************************************************/
int GetFileIndex(int RawDataType, int DecodeID)
{
	int FileIndex = -1;

	const static GET_ID_FUNC s_GetFileIndexFuncMap[] = {
		RTCM3TypeIDToConvertFileIndex,
		RTCM2TypeIDToConvertFileIndex,
		RawMsgDecodeIDToConvertFileIndex,
		RawMsgDecodeIDToConvertFileIndex,
		HRCXTypeIDToConvertFileIndex,
		NULL,
		RinexIDToConvertFileIndex,
	};

	if (RawDataType < (sizeof(s_GetFileIndexFuncMap)/sizeof(s_GetFileIndexFuncMap[0])))
	{
		if (NULL != s_GetFileIndexFuncMap[RawDataType])
		{
			FileIndex = s_GetFileIndexFuncMap[RawDataType](DecodeID);
		}
	}

	return FileIndex;
}

/**********************************************************************//**
@brief  HRCX type id to rinex id

@param TypeID        [In] HRCX type id

@retval <0:error >=0: rinex id

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int HRCXTypeIDToRinexID(int TypeID)
{
	const static int s_ConvertFileIndexMap[] =
	{
		RINEX_ID_GPSEPH,/**< 32019*/
		RINEX_ID_GLOEPH,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		RINEX_ID_BDSEPH,
		-1,
		-1,
		RINEX_ID_GALEPH_FNAV,
		RINEX_ID_GALEPH_INAV, /**< 32046 */
	};

	if ((TypeID >= 32019) && (TypeID <= 32046))
	{
		return s_ConvertFileIndexMap[TypeID - 32019];
	}

	return -1;
}

/**********************************************************************//**
@brief  RTCM2 type id to rinex id

@param TypeID        [In] RTCM2 type id

@retval <0:error >=0: rinex id

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int RTCM2TypeIDToRinexID(int TypeID)
{
	if (TypeID == 17)
	{
		return RINEX_ID_GPSEPH;
	}

	return -1;
}

/**********************************************************************//**
@brief  RTCM3 type id to rinex id

@param TypeID        [In] RTCM3 type id

@retval <0:error >=0: rinex id

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int RTCM3TypeIDToRinexID(int TypeID)
{
	const static int s_ConvertFileIndexMap[] =
	{
		RINEX_ID_GPSEPH,/**< RTCM1019*/
		RINEX_ID_GLOEPH,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		RINEX_ID_NICEPH,
		RINEX_ID_BDSEPH,
		-1,
		RINEX_ID_QZSSEPH,
		RINEX_ID_GALEPH_FNAV,
		RINEX_ID_GALEPH_INAV, /**< RTCM1046 */
	};

	if ((TypeID >= 1019) && (TypeID <= 1046))
	{
		return s_ConvertFileIndexMap[TypeID - 1019];
	}

	return -1;
}

/**********************************************************************//**
@brief  RawMsg decode id to rinex id

@param DecodeID        [In] RawMsg decode id

@retval <0:error >=0: rinex id

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int RawMsgDecodeIDToRinexID(int DecodeID)
{
	const static int s_ConvertFileIndexMap[] =
	{
		-1,
		-1,
		-1,
		-1,
		RINEX_ID_GPSEPH,
		RINEX_ID_GLOEPH,
		RINEX_ID_GALEPH,
		RINEX_ID_GALEPH_FNAV,
		RINEX_ID_GALEPH_INAV,
		RINEX_ID_GALEPH_RCED,
		RINEX_ID_QZSSEPH,
		RINEX_ID_BDSEPH,
		RINEX_ID_BDSEPH_CNAV1,
		RINEX_ID_BDSEPH_CNAV2,
		RINEX_ID_BDSEPH_CNAV3,
		RINEX_ID_NICEPH,
		-1,

		/** Huace raw msg */
		-1,
		-1,
		-1,
		-1,
		RINEX_ID_GPSEPH,
		RINEX_ID_GLOEPH,
		RINEX_ID_GALEPH,
		RINEX_ID_GALEPH_FNAV,
		RINEX_ID_GALEPH_INAV,
		RINEX_ID_GALEPH_RCED,
		RINEX_ID_QZSSEPH,
		RINEX_ID_BDSEPH,
		RINEX_ID_BDSEPH_CNAV1,
		RINEX_ID_BDSEPH_CNAV2,
		RINEX_ID_BDSEPH_CNAV3,
		RINEX_ID_NICEPH,

		RINEX_ID_BDSEPH_CNAV2, /**< UN */
		RINEX_ID_BDSEPH,
		-1,
		-1,
		-1,
		-1,
		RINEX_ID_GPSEPH,
		RINEX_ID_BDSEPH_CNAV2,
		RINEX_ID_BDSEPH,
		RINEX_ID_GLOEPH,
		RINEX_ID_GALEPH,

		-1, /**< KMD */
		RINEX_ID_BDSEPH,
		RINEX_ID_GPSEPH,
		RINEX_ID_GALEPH_INAV,
		RINEX_ID_GLOEPH,

		RINEX_ID_BDSEPH_CNAV1,
		RINEX_ID_BDSEPH_CNAV2,
		RINEX_ID_BDSEPH_CNAV3,
	};

	if (DecodeID < (sizeof(s_ConvertFileIndexMap)/sizeof(s_ConvertFileIndexMap[0])))
	{
		return s_ConvertFileIndexMap[DecodeID];
	}

	return -1;
}

/**********************************************************************//**
@brief  get rinex id

@param RawDataType        [In] raw data type
@param DecodeID           [In] decode id

@retval <0:error >=0: rinex id

@author CHC
@date 2024/07/18
@note
**************************************************************************/
int GetRinexID(int RawDataType, int DecodeID)
{
	int RinexID = -1;

	const static GET_ID_FUNC s_GetRinexIDFuncMap[] = {
		RTCM3TypeIDToRinexID,
		RTCM2TypeIDToRinexID,
		RawMsgDecodeIDToRinexID,
		RawMsgDecodeIDToRinexID,
		HRCXTypeIDToRinexID,
		NULL,
		NULL,
	};

	if (RawDataType < (sizeof(s_GetRinexIDFuncMap)/sizeof(s_GetRinexIDFuncMap[0])))
	{
		if (NULL != s_GetRinexIDFuncMap[RawDataType])
		{
			RinexID = s_GetRinexIDFuncMap[RawDataType](DecodeID);
		}
	}

	return RinexID;
}

/**********************************************************************//**
@brief  HRCX type id to RawMsg id

@param TypeID        [In] HRCX type id

@retval <0:error >=0: rinex id

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int HRCXTypeIDToRawMsgID(int TypeID)
{
	const static int s_RawMsgIDMap[] =
	{
		RAW_MSG_ID_EPH_GPS,/**< 32019*/
		RAW_MSG_ID_EPH_GLO,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		RAW_MSG_ID_EPH_BD2,
		-1,
		-1,
		RAW_MSG_ID_EPH_GAL_FNAV,
		RAW_MSG_ID_EPH_GAL_INAV, /**< 32046 */
	};

	if ((TypeID >= 32019) && (TypeID <= 32046))
	{
		return s_RawMsgIDMap[TypeID - 32019];
	}

	return -1;
}

/**********************************************************************//**
@brief  RTCM2 type id to rinex id

@param TypeID        [In] RTCM2 type id

@retval <0:error >=0: rinex id

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int RTCM2TypeIDToRawMsgID(int TypeID)
{
	if (TypeID == 17)
	{
		return RAW_MSG_ID_EPH_GPS;
	}

	return -1;
}

/**********************************************************************//**
@brief  RTCM3 type id to rinex id

@param TypeID        [In] RTCM3 type id

@retval <0:error >=0: rinex id

@author CHC
@date 2024/07/18
@note
**************************************************************************/
static int RTCM3TypeIDToRawMsgID(int TypeID)
{
	const static int s_RawMsgIDMap[] =
	{
		RAW_MSG_ID_EPH_GPS,/**< RTCM1019*/
		RAW_MSG_ID_EPH_GLO,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		RAW_MSG_ID_EPH_NAVIC,
		RAW_MSG_ID_EPH_BD2,
		-1,
		RAW_MSG_ID_EPH_QZSS,
		RAW_MSG_ID_EPH_GAL_FNAV,
		RAW_MSG_ID_EPH_GAL_INAV, /**< RTCM1046 */
	};

	if ((TypeID >= 1019) && (TypeID <= 1046))
	{
		return s_RawMsgIDMap[TypeID - 1019];
	}

	return -1;
}

#if defined (__GNUC__) || defined (_MSC_VER) || defined (__CC_ARM)
typedef union _DOUBLE_INT_UNION
{
	double             F64;
	unsigned int       U32[2];
	signed int         S32[2];
	unsigned long long U64;
	signed long long   S64;

} DOUBLE_INT_UNION;
#else
#error unknown compiler for float format
#endif

/**********************************************************************//**
@brief judge if a double is NaN

@param Value [In]  double Value

@author CHC
@date 2022/04/03 19:52:10
@note
History:\n
- 2022.04.03 Start up
**************************************************************************/
int IsNaN(double Value)
{
	DOUBLE_INT_UNION Data;

	Data.F64 = Value;
	if ((Data.U32[1] & 0x7ff00000) == 0x7ff00000)
	{
		if ((Data.U32[1] & 0x000fffff) || (Data.U32[0]))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}




#endif
