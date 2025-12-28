/**********************************************************************//**
		   VeriQC

	Interface Module
*-
@file   MsgConvertTransQC.c
@author CHC
@date   2024/06/18
@brief  tansfer MsgConvert module infomation to QC

**************************************************************************/
#include "VeriQC.h"
#include "TimeDefines.h"
#include "MsgConvertTransQC.h"
#include "Rinex/RinexDecoder.h"
#include "BSW/MsgDecoder/RTCM/RTCMSupportFunc.h"
#include "DataQC.h"
#include "CommonBase.h"
#include "MsgDecodeDefinesEx.h"
#include "VeriQCSPP.h"
#include "RawMsgAscii/RawMsgDecodeIDDefines.h"
#include "MsgConvert.h"


extern int g_RangeToQCSysID[] = { /**< Range System order */
	QC_GNSS_SYSID_GPS,  /**< GPS */
	QC_GNSS_SYSID_GLN,	/**< GLO */
	QC_GNSS_SYSID_SBAS,	/**< SBA */
	QC_GNSS_SYSID_GAL,	/**< GAL */
	QC_GNSS_SYSID_BDS,	/**< BDS */
	QC_GNSS_SYSID_QZS,	/**< QZS */
	QC_GNSS_SYSID_NUL,  /**< NIC, QC2 reserved */
};

int g_RangeToQCSatPRN[] = { /**< Range System order */
	0,      /**< GPS */
	-37,    /**< GLO */
	0,      /**< SBA */
	0,      /**< GAL */
	0,      /**< BDS */
	-160,   /**< QZS */
	0,      /**< NIC, QC2 reserved */
};

int g_RangeToQCSysMask[] = { /**< Range System order */
	QC_SYS_GPS,   /**< GPS */
	QC_SYS_GLO,	  /**< GLO */
	QC_SYS_SBS,	  /**< SBA */
	QC_SYS_GAL,	  /**< GAL */
	QC_SYS_CMP,	  /**< BDS */
	QC_SYS_QZS,	  /**< QZS */
	QC_SYS_NONE,  /**< NIC, QC2 reserved */
};

/**********************************************************************//**
@brief Convert Range Sat PRN to QC SatNo

@param RangeSys     [In]     Range system
@param RangeSat     [In]     Range Sat PRN

@return QC SatNo

@author CHC
@date 2024/07/25
@note
History:\n
**************************************************************************/
extern int RangeToQCSatNo(int RangeSys, int RangeSat)
{
	int QCSysMask = g_RangeToQCSysMask[RangeSys];
	int QCSatPRN = RangeSat + g_RangeToQCSatPRN[RangeSys];

	return GetQCSatNo(QCSysMask, QCSatPRN);
}

/**********************************************************************//**
@brief Convert Range Sig ID to QC Sig ID

@param RangeSys     [In]     Range system
@param RangeSig     [In]     Range Sig ID

@return QC Sig ID

@author CHC
@date 2024/07/25
@note
History:\n
**************************************************************************/
extern int RangeToQCSigID(int RangeSys, int RangeSig)
{
	int QCCode = -1;
	int QCSysIndex = g_RangeToQCSysID[RangeSys];

	const static int s_RangSigToQCCodeTable[QC_MAX_NUM_SYS + 1][32] =
	{
		[QC_GNSS_SYSID_GPS] =
		{
			/**  0 -  6 */
			QC_CODE_L1C, QC_CODE_L2C, -1,          -1,          QC_CODE_L1P, QC_CODE_L2P, QC_CODE_L5I,
			/**  7 - 13 */
			-1,          QC_CODE_L1W, QC_CODE_L2W, QC_CODE_L5X, -1,          QC_CODE_L1S, QC_CODE_L2L,
			/** 14 - 20 */
			QC_CODE_L5Q, -1,          QC_CODE_L1L, QC_CODE_L2S, -1,          -1,          QC_CODE_L1X,
			/** 21 - 27 */
			QC_CODE_L2X, -1,          -1,          -1,          -1,          -1,          -1,
			/** 28 - 31 */
			-1,          -1,          -1,          -1,
		},
		[QC_GNSS_SYSID_BDS] =
		{
			/**  0 -  6 */
			QC_CODE_L2I, QC_CODE_L7I, QC_CODE_L6I, -1,          QC_CODE_L2I, QC_CODE_L7I, QC_CODE_L6I,
			/**  7 - 13 */
			QC_CODE_L1P, QC_CODE_L7P, QC_CODE_L5P, QC_CODE_L1D, QC_CODE_L7D, QC_CODE_L5D, QC_CODE_L1X,
			/** 14 - 20 */
			-1,          QC_CODE_L5X, -1,          -1,          -1,          -1,          QC_CODE_L2Q,
			/** 21 - 27 */
			QC_CODE_L7Q, QC_CODE_L6Q, QC_CODE_L1S, QC_CODE_L1L, QC_CODE_L1Z, QC_CODE_L6D, QC_CODE_L6P,
			/** 28 - 31 */
			QC_CODE_L6Z, -1,          -1,          -1,
		},
		[QC_GNSS_SYSID_GAL] =
		{
			/**  0 -  6 */
			-1,          QC_CODE_L1B, QC_CODE_L1C, QC_CODE_L1X, -1,          -1,          QC_CODE_L6B,
			/**  7 - 13 */
			QC_CODE_L6C, QC_CODE_L6X, -1,          -1,          QC_CODE_L5I, QC_CODE_L5Q, QC_CODE_L5X,
			/** 14 - 20 */
			-1,          -1,          QC_CODE_L7I, QC_CODE_L7Q, QC_CODE_L7X, QC_CODE_L8I, QC_CODE_L8Q,
			/** 21 - 27 */
			QC_CODE_L8X, -1,          -1,          -1,          -1,          -1,          -1,
			-1,          -1,          -1,          -1,
		},
		[QC_GNSS_SYSID_QZS] =
		{
			/**  0 -  6 */
			QC_CODE_L1C, -1,          -1,          -1,          -1,          -1,          QC_CODE_L5I,
			/**  7 - 13 */										             
			-1,          -1,          -1,          QC_CODE_L5X, -1,          QC_CODE_L1S, QC_CODE_L2L,
			/** 14 - 20 */										             
			QC_CODE_L5Q, -1,          QC_CODE_L1L, QC_CODE_L2S, -1,          -1,          QC_CODE_L1X,
			/** 21 - 27 */
			QC_CODE_L2X, -1,          -1,          -1,          -1,          -1,          QC_CODE_L6L,
			QC_CODE_L6X, -1,          -1,          -1,
		},
		[QC_GNSS_SYSID_GLN] =
		{
			/**  0 -  6 */
			QC_CODE_L1C, QC_CODE_L2C, QC_CODE_L3I, -1,          QC_CODE_L1P, QC_CODE_L2P, QC_CODE_L3Q,
			/**  7 - 13 */
			-1,          -1,          -1,          QC_CODE_L3X, -1,          -1,          QC_CODE_L6A,
			/** 14 - 20 */
			-1,          -1,          -1,          -1,          -1,          -1,          -1,
			/** 21 - 27 */
			-1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1,
		},
		[QC_GNSS_SYSID_NUL] =
		{
			/**  0 -  6 */
			-1, -1, -1, -1, -1, -1, -1,
			/**  7 - 13 */
			-1, -1, -1, -1, -1, -1, -1,
			/** 14 - 20 */
			-1, -1, -1, -1, -1, -1, -1,
			/** 21 - 27 */
			-1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1,
		},
	};

	return s_RangSigToQCCodeTable[QCSysIndex][RangeSig];
}

/**********************************************************************//**
@brief get Rinex ID for decoded message

@param SourceType     [In]     Message source type, see GNSS_STRFMT_XX
@param DecodeTypeID   [In]     Message type ID

@return Rinex ID

@author CHC
@date 2024/07/25
@note
History:\n
**************************************************************************/
int GetDecodeMsgRinexID(int SourceType, int DecodeTypeID)
{
	int RinexID = -1;
	if (SourceType == MSG_DECODE_TYPE_RTCM2)
	{
		switch (DecodeTypeID)
		{
		case 18:
		case 19:
			RinexID = RINEX_ID_OBS;
			break;
		case 17:
			RinexID = RINEX_ID_GPSEPH;
			break;
		default:
			break;
		}
	}
	else if (SourceType == MSG_DECODE_TYPE_RTCM3)
	{
		switch (DecodeTypeID)
		{
		case 1001:
		case 1002:
		case 1003:
		case 1004:
		case 1009:
		case 1010:
		case 1011:
		case 1012:
		case 1071:
		case 1072:
		case 1073:
		case 1074:
		case 1075:
		case 1076:
		case 1077:
		case 1081:
		case 1082:
		case 1083:
		case 1084:
		case 1085:
		case 1086:
		case 1087:
		case 1091:
		case 1092:
		case 1093:
		case 1094:
		case 1095:
		case 1096:
		case 1097:
		case 1101:
		case 1102:
		case 1103:
		case 1104:
		case 1105:
		case 1106:
		case 1107:
		case 1111:
		case 1112:
		case 1113:
		case 1114:
		case 1115:
		case 1116:
		case 1117:
		case 1121:
		case 1122:
		case 1123:
		case 1124:
		case 1125:
		case 1126:
		case 1127:
			RinexID = RINEX_ID_OBS;
			break;
		case 1019:
			RinexID = RINEX_ID_GPSEPH;
			break;
		case 1020:
			RinexID = RINEX_ID_GLOEPH;
			break;
		case 1041:
			RinexID = RINEX_ID_NICEPH;
			break;
		case 1042:
			RinexID = RINEX_ID_BDSEPH;
			break;
		case 1044:
			RinexID = RINEX_ID_QZSSEPH;
			break;
		case 1045:
		case 1046:
			RinexID = RINEX_ID_GALEPH;
			break;
		default:
			break;
		}
	}
	else if ((SourceType == MSG_DECODE_TYPE_RAWMSGA) || (SourceType == MSG_DECODE_TYPE_RAWMSGB) || (SourceType == MSG_DECODE_TYPE_CHCRAWMSGB) || (SourceType == MSG_DECODE_TYPE_UNRAWMSGB))
	{
		switch (DecodeTypeID)
		{
		case RAWMSG_DECODE_ID_RANGE:
		case RAWMSG_DECODE_ID_RANGECMP:
		case RAWMSG_DECODE_ID_GNSSOBS:
		case RAWMSG_DECODE_ID_GNSSOBSCMP:
		case RAWMSG_DECODE_ID_UNOBSVM:
		case RAWMSG_DECODE_ID_UNOBSVH:
		case RAWMSG_DECODE_ID_KMDRANGE:
			RinexID = RINEX_ID_OBS;
			break;
		case RAWMSG_DECODE_ID_EPH_GPS:
		case RAWMSG_DECODE_ID_GPSEPH:
		case RAWMSG_DECODE_ID_UNGPSEPH:
		case RAWMSG_DECODE_ID_KMDGPSEPH:
			RinexID = RINEX_ID_GPSEPH;
			break;
		case RAWMSG_DECODE_ID_EPH_GLO:
		case RAWMSG_DECODE_ID_GLOEPH:
		case RAWMSG_DECODE_ID_UNGLOEPH:
		case RAWMSG_DECODE_ID_KMDGLOEPH:
			RinexID = RINEX_ID_GLOEPH;
			break;
		case RAWMSG_DECODE_ID_EPH_GAL:
		case RAWMSG_DECODE_ID_EPH_GALNAVF:
		case RAWMSG_DECODE_ID_EPH_GALNAVI:
		case RAWMSG_DECODE_ID_EPH_GALRCED:
		case RAWMSG_DECODE_ID_GALEPH:
		case RAWMSG_DECODE_ID_GALFNAVEPH:
		case RAWMSG_DECODE_ID_GALINAVEPH:
		case RAWMSG_DECODE_ID_GALRCEDEPH:
		case RAWMSG_DECODE_ID_UNGALEPH:
		case RAWMSG_DECODE_ID_KMDGALINAVEPH:
			RinexID = RINEX_ID_GALEPH;
			break;
		case RAWMSG_DECODE_ID_EPH_QZS:
		case RAWMSG_DECODE_ID_QZSEPH:
			RinexID = RINEX_ID_QZSSEPH;
			break;
		case RAWMSG_DECODE_ID_EPH_BD2:
		case RAWMSG_DECODE_ID_EPH_BD3CNAV1:
		case RAWMSG_DECODE_ID_EPH_BD3CNAV2:
		case RAWMSG_DECODE_ID_EPH_BD3CNAV3:
		case RAWMSG_DECODE_ID_BDSEPH:
		case RAWMSG_DECODE_ID_BDSCNAV1EPH:
		case RAWMSG_DECODE_ID_BDSCNAV2EPH:
		case RAWMSG_DECODE_ID_BDSCNAV3EPH:
		case RAWMSG_DECODE_ID_UNBD3EPH:
		case RAWMSG_DECODE_ID_UNBDSEPH:
		case RAWMSG_DECODE_ID_KMDBDSEPH:
		case RAWMSG_DECODE_ID_UNBD3EPHEM:
		case RAWMSG_DECODE_ID_UNBD2EPHEM:
		case RAWMSG_DECODE_ID_NVBDSCNAV1EPH:
		case RAWMSG_DECODE_ID_NVBDSCNAV2EPH:
		case RAWMSG_DECODE_ID_NVBDSCNAV3EPH:
			RinexID = RINEX_ID_BDSEPH;
			break;
		case RAWMSG_DECODE_ID_EPH_NIC:
		case RAWMSG_DECODE_ID_NICEPH:
			RinexID = RINEX_ID_NICEPH;
			break;
		default:
			break;
		}
	}
	else if (SourceType == GNSS_STRFMT_HRCX)
	{
		switch (DecodeTypeID)
		{
		case 32074:
		case 32084:
		case 32094:
		case 32124:
			RinexID = RINEX_ID_OBS;
			break;
		case 32019:
			RinexID = RINEX_ID_GPSEPH;
			break;
		case 32020:
			RinexID = RINEX_ID_GLOEPH;
			break;
		case 32042:
			RinexID = RINEX_ID_BDSEPH;
			break;
		case 32045:
		case 32046:
			RinexID = RINEX_ID_GALEPH;
			break;
		default:
			break;
		}
	}

	return RinexID;
}

/**********************************************************************//**
@brief convert system string to ExSystem mask

@param pSysStr       [In]     system string, GRES...

@author CHC
@date 2025/08/11
@note
History:\n
**************************************************************************/
int SysStr2ExSysMask(char* pSysStr)
{
	int Sys = 0, iLoop;
	int StrLen = (int)strlen(pSysStr);
	for (iLoop = 0; iLoop < StrLen; iLoop++)
	{
		switch (pSysStr[iLoop])
		{
		case 'G':
		case 'g':
			Sys += (1 << GPS);
			break;
		case 'S':
		case 's':
			Sys += (1 << SBA);
			break;
		case 'R':
		case 'r':
			Sys += (1 << GLO);
			break;
		case 'E':
		case 'e':
			Sys += (1 << GAL);
			break;
		case 'J':
		case 'j':
			Sys += (1 << QZS);
			break;
		case 'C':
		case 'c':
			Sys += (1 << BDS);
			break;
		case 'I':
		case 'i':
			Sys += (1 << NIC);
			break;
		default:
			break;
		}
	}
	if (Sys == 0)
	{
		return 255;/**< default : all system  */
	}
	else
	{
		return Sys;
	}
}

/**********************************************************************//**
@brief  get Rinex Convert/Combine process progress

@return progress

@author CHC
@date 2025/08/18
@note
**************************************************************************/
int GetRinexProgress()
{
	int Progress = GetMsgConvertProgress();

	return Progress;
}