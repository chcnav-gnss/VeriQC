/**********************************************************************//**
		   VeriQC

	Interface Module
*-
@file   MsgConvertTransQC.h
@author CHC
@date   2024/06/18
@brief  tansfer MsgConvert module infomation to QC

**************************************************************************/

#ifndef _MSGCONVERT_TRANS_QC_H_
#define _MSGCONVERT_TRANS_QC_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern int g_RangeToQCSysID[];
extern int g_RangeToQCSatPRN[];
extern int g_RangeToQCSysMask[];
extern int RangeToQCSatNo(int RangeSys, int RangeSat);
extern int RangeToQCSigID(int RangeSys, int RangeSig);


int GetDecodeMsgRinexID(int SourceType, int DecodeTypeID);
int SysStr2ExSysMask(char* pSysStr);

#ifdef __cplusplus
}
#endif

#endif