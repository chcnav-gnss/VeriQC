
#include "ProcInterface.h"

#include "GNSSBase.h"
#include "Logs.h"
#include "Common/GNSSNavDataType.h"
#include "BSW/MsgEncoder/RawMsg/RawMsgRangeDefines.h"
#include "Common/DataTypes.h"

VERIQC_ORBIT_PROCESSOR_T* VeriQCCreateOrbitProcessor();
int32_t VeriQCReleaseOrbitProcessor(VERIQC_ORBIT_PROCESSOR_T* pObj);
void VeriQCOrbitSetOpt(VERIQC_ORBIT_PROCESSOR_T* pObj, const VERIQC_SPPTC_OPT_T* pOpt, VERIQC_SPPTC_OPT_CTRL_E OptCtrl);
int32_t VeriQCOrbitGetOpt(VERIQC_ORBIT_PROCESSOR_T* pObj, VERIQC_SPPTC_OPT_T* pOpt);
CHC_NAV_T* VeriQCGetNav(VERIQC_ORBIT_PROCESSOR_T* pObj);
int32_t VeriQCOrbitAddUNIEphemeris(VERIQC_ORBIT_PROCESSOR_T* pObj, const UNI_EPHEMERIS_T* pEph);
int32_t VeriQCOrbitAddGLOEphemeris(VERIQC_ORBIT_PROCESSOR_T* pObj, const GLO_EPHEMERIS_T* pGLOEph);
int32_t VeriQCOrbitRangeFit(VERIQC_ORBIT_PROCESSOR_T* pObj, const RM_RANGE_LOG_T* pRangeObs);
int32_t VeriQCOrbitSatPos(VERIQC_ORBIT_PROCESSOR_T* pObj, const CHC_OBS_T* pObs, VERIQC_ORBIT_SOL_T* pSol, uint8_t IncrementUpdateMark);
int32_t VeriQCOrbitSetSelEphTime(VERIQC_ORBIT_PROCESSOR_T* pObj, const QC_TIME_T* pEphTime);
int32_t VeriQCOrbitOneSatCalc(VERIQC_ORBIT_PROCESSOR_T* pObj, const QC_TIME_T Time, VERIQC_SYSID_E SysId, unsigned int SatPRN,
    double* pSatPosVel, double* pSatClock);
int32_t VeriQCOrbitSatCalcByToe(VERIQC_ORBIT_PROCESSOR_T* pObj, const QC_TIME_T TransTime, const QC_TIME_T Time, VERIQC_SYSID_E SysId, unsigned int SatPRN, int Toe,
    double* pSatPosVel, double* pSatClock);

static VERIQC_ORBIT_IFX_T s_OrbitModuleGlb =
{
    VeriQCCreateOrbitProcessor,
    VeriQCReleaseOrbitProcessor,
    VeriQCOrbitSetOpt,
    VeriQCOrbitGetOpt,
    VeriQCGetNav,
    VeriQCOrbitAddUNIEphemeris,
    VeriQCOrbitAddGLOEphemeris,
    VeriQCOrbitRangeFit,
    VeriQCOrbitSatPos,
    VeriQCOrbitSetSelEphTime,
    VeriQCOrbitOneSatCalc,
    VeriQCOrbitSatCalcByToe
};

VERIQC_ORBIT_IFX_T* VeriQCGetOrbitIfx()
{
    return &s_OrbitModuleGlb;
}

void VeriQCOrbitConstruct(VERIQC_ORBIT_PROCESSOR_T* pObj)
{
    int memused = 0, memfree = 0;
    if (!pObj)
    {
        return;
    }
    memused = (int)VERIQC_GET_USEDMEM();
    memfree = (int)VERIQC_GET_FREEMEM();
    pObj->pOpt = VERIQC_MALLOC(sizeof(VERIQC_SPPTC_OPT_T));
    memset(pObj->pOpt, 0, sizeof(VERIQC_SPPTC_OPT_T));

    pObj->pNav = (CHC_NAV_T*)VERIQC_MALLOC(sizeof(CHC_NAV_T));
    memset(pObj->pNav, 0, sizeof(CHC_NAV_T));
    pObj->pNav->Num = 0;
    pObj->pNav->NumBD2 = 0;

    pObj->pNav->pEph = (CHC_EPH_T*)VERIQC_MALLOC(sizeof(CHC_EPH_T)*pObj->pNav->NumMax);
    memset(pObj->pNav->pEph, 0, sizeof(CHC_EPH_T)*pObj->pNav->NumMax);
    memused = (int)VERIQC_GET_USEDMEM();
    memfree = (int)VERIQC_GET_FREEMEM();

#if defined VERIQC_SURVEY
	pObj->pNav->NumBD3 = 0;
    pObj->pNav->NumMaxBD3 = CHC_NSATCMP;
	pObj->pNav->pEphBD3 = (CHC_EPH_T*)VERIQC_MALLOC(sizeof(CHC_EPH_T)*pObj->pNav->NumMaxBD3);
	memset(pObj->pNav->pEphBD3, 0, sizeof(CHC_EPH_T)*pObj->pNav->NumMaxBD3);
	memused = (int)VERIQC_GET_USEDMEM();
	memfree = (int)VERIQC_GET_FREEMEM();
#endif

	pObj->pNav->NumGLO = 0;
    pObj->pNav->NumGLOMax = (VERIQC_FLAG == 2 ? 5 : CHC_NSATGLO);
    pObj->pNav->pGEph = (CHC_GEPH_T*)VERIQC_MALLOC(sizeof(CHC_GEPH_T)*pObj->pNav->NumGLOMax);
    memset(pObj->pNav->pGEph, 0, sizeof(CHC_GEPH_T)*pObj->pNav->NumGLOMax);
    memused = (int)VERIQC_GET_USEDMEM();
    memfree = (int)VERIQC_GET_FREEMEM();

    /** Satellite fitting information */
	pObj->EphValidFlagNum = CHC_MAXSAT;
    pObj->SatInfoNum = VeriQCCreateSatInfoArray(&pObj->pSatIptInfo, pObj->EphValidFlagNum);       /**< Internal allocation, size determined internally. */
    pObj->pEphValidFlag = (unsigned char*)VERIQC_MALLOC(sizeof(unsigned char)*pObj->EphValidFlagNum);
    memset(pObj->pEphValidFlag, 0, sizeof(unsigned char)*pObj->EphValidFlagNum);
    pObj->pEphActiveValidFlag = (unsigned char*)VERIQC_MALLOC(sizeof(unsigned char)*pObj->EphValidFlagNum);
    memset(pObj->pEphActiveValidFlag, 0, sizeof(unsigned char)*pObj->EphValidFlagNum);
#if defined VERIQC_SURVEY
	pObj->EphValidFlagBD3Num = CHC_NSATCMP;
	VeriQCCreateSatInfoArray(&pObj->pSatIptInfoBD3, pObj->EphValidFlagBD3Num);       /**< Internal allocation, size determined internally. */
	pObj->pEphValidFlagBD3 = (unsigned char*)VERIQC_MALLOC(sizeof(unsigned char)*pObj->EphValidFlagBD3Num);
	memset(pObj->pEphValidFlagBD3, 0, sizeof(unsigned char)*pObj->EphValidFlagBD3Num);
	pObj->pEphActiveValidFlagBD3 = (unsigned char*)VERIQC_MALLOC(sizeof(unsigned char)*pObj->EphValidFlagBD3Num);
	memset(pObj->pEphActiveValidFlagBD3, 0, sizeof(unsigned char)*pObj->EphValidFlagBD3Num);

#endif

    pObj->pNav->pIonoParamGPS = (IONO_PARAM_T*)VERIQC_MALLOC(sizeof(IONO_PARAM_T));
    memset(pObj->pNav->pIonoParamGPS, 0, sizeof(IONO_PARAM_T));
    pObj->pNav->pIonoParamBDS = (IONO_PARAM_T*)VERIQC_MALLOC(sizeof(IONO_PARAM_T));
    memset(pObj->pNav->pIonoParamBDS, 0, sizeof(IONO_PARAM_T));
    pObj->pNav->pIonoParamQZS = (IONO_PARAM_T*)VERIQC_MALLOC(sizeof(IONO_PARAM_T));
    memset(pObj->pNav->pIonoParamQZS, 0, sizeof(IONO_PARAM_T));

    pObj->pEphTime = (QC_TIME_T *)VERIQC_MALLOC(sizeof(QC_TIME_T));
    memset(pObj->pEphTime, 0, sizeof(QC_TIME_T));
}

void VeriQCOrbitDestruct(VERIQC_ORBIT_PROCESSOR_T* pObj)
{
    if (!pObj)
    {
        return;
    }
    if (pObj->pOpt)
    {
        VERIQC_FREE(pObj->pOpt);
    }

    VeriQCReleaseSatInfoArray((struct _CHC_SATIPTINFO_T**)&pObj->pSatIptInfo, pObj->EphValidFlagNum);
    pObj->SatInfoNum = 0;
    VERIQC_FREE(pObj->pNav->pGEph);
    pObj->pNav->NumGLO = pObj->pNav->NumGLOMax = 0;
    VERIQC_FREE(pObj->pNav->pEph);
#if defined VERIQC_SURVEY
    VeriQCReleaseSatInfoArray((struct _CHC_SATIPTINFO_T **)&pObj->pSatIptInfoBD3, pObj->EphValidFlagBD3Num);
    VERIQC_FREE(pObj->pNav->pEphBD3);
    pObj->pNav->NumBD3 = pObj->pNav->NumMaxBD3 = 0;
    pObj->EphValidFlagBD3Num = 0;
    VERIQC_FREE(pObj->pEphValidFlagBD3); pObj->pEphValidFlagBD3 = NULL;
    VERIQC_FREE(pObj->pEphActiveValidFlagBD3); pObj->pEphActiveValidFlagBD3 = NULL;

#endif
    VERIQC_FREE(pObj->pNav->pIonoParamGPS); pObj->pNav->pIonoParamGPS = NULL;
    VERIQC_FREE(pObj->pNav->pIonoParamBDS); pObj->pNav->pIonoParamBDS = NULL;
    VERIQC_FREE(pObj->pNav->pIonoParamQZS); pObj->pNav->pIonoParamQZS = NULL;

    pObj->pNav->Num = pObj->pNav->NumMax = 0;
    
    VERIQC_FREE(pObj->pNav); pObj->pNav = NULL;
    pObj->EphValidFlagNum = 0;
    VERIQC_FREE(pObj->pEphValidFlag); pObj->pEphValidFlag = NULL;
    VERIQC_FREE(pObj->pEphActiveValidFlag); pObj->pEphActiveValidFlag = NULL;
    VERIQC_FREE(pObj->pEphTime); pObj->pEphTime = NULL;
    if (pObj->pEphIndex)
    {
        VERIQC_FREE(pObj->pEphIndex); pObj->pEphIndex = NULL;
    }
    if (pObj->pEphIndexBD3)
    {
        VERIQC_FREE(pObj->pEphIndexBD3); pObj->pEphIndexBD3 = NULL;
    }
}

VERIQC_ORBIT_PROCESSOR_T* VeriQCCreateOrbitProcessor()
{
	int memused = 0, memfree = 0;
    VERIQC_ORBIT_PROCESSOR_T* pobj = NULL;
	memused = (int)VERIQC_GET_USEDMEM();
	memfree = (int)VERIQC_GET_FREEMEM();
    pobj = VERIQC_MALLOC(sizeof(VERIQC_ORBIT_PROCESSOR_T));
    if (!pobj)
    {
        return NULL;
    }
	memset(pobj, 0, sizeof(VERIQC_ORBIT_PROCESSOR_T));
	memused = (int)VERIQC_GET_USEDMEM();
	memfree = (int)VERIQC_GET_FREEMEM();
    VeriQCOrbitConstruct(pobj);
    return pobj;
}

int32_t VeriQCReleaseOrbitProcessor(VERIQC_ORBIT_PROCESSOR_T* pObj)
{
    if (pObj)
    {
        VeriQCOrbitDestruct(pObj);
        VERIQC_FREE(pObj);
        pObj = NULL;
        return 0;
    }
    return-1;
}

void VeriQCOrbitSetOpt(VERIQC_ORBIT_PROCESSOR_T* pObj, const VERIQC_SPPTC_OPT_T* pOpt, VERIQC_SPPTC_OPT_CTRL_E OptCtrl)
{
    if (!pObj || !pOpt)
    {
        return;
    }
    VeriQCCopySPPTcOptCtrl(pOpt, pObj->pOpt, OptCtrl);
    if (!pObj->pEphIndex)
    {
        pObj->pEphIndex = (int*)VERIQC_MALLOC(sizeof(int) * CHC_MAXSAT * 2);
        memset(pObj->pEphIndex, -1, sizeof(int) * CHC_MAXSAT * 2);
    }
    if (!pObj->pEphIndexBD3)
    {
        pObj->pEphIndexBD3 = (int*)VERIQC_MALLOC(sizeof(int) * CHC_NSATCMP * 2);
        memset(pObj->pEphIndexBD3, -1, sizeof(int) * CHC_NSATCMP * 2);
    }
}

int32_t VeriQCOrbitGetOpt(VERIQC_ORBIT_PROCESSOR_T* pObj, VERIQC_SPPTC_OPT_T* pOpt)
{
    if (!pObj || !pOpt)
    {
        return -1;
    }
    *pOpt = *(pObj->pOpt);
    return 0;
}

CHC_NAV_T* VeriQCGetNav(VERIQC_ORBIT_PROCESSOR_T* pObj)
{
    if (!pObj)
    {
        return NULL;
    }
    return pObj->pNav;
}

/********************************************************************************************
@brief   add unify ephemeris data to orbit processor

@param   pObj[In]              orbit processor
@param   pEph[In]              unify ephemeris data

@author  CHC

@return  0:OK, -1:error
*********************************************************************************************/
int32_t VeriQCOrbitAddUNIEphemeris(VERIQC_ORBIT_PROCESSOR_T* pObj, const UNI_EPHEMERIS_T* pEph)
{
    int ephIndex;
    int sat, sys;
    int uniEphPrn, uniEphSys;
    BOOL bEphExist = FALSE;
    BOOL bBDS3Eph = FALSE;
    int realTimeMode;
    int IODE;
    int increNum;
    double deltaTOE;
    double delteTOC;
    CHC_NAV_T* pNav = NULL;
    CHC_EPH_T* pTmpEph = NULL;
    QC_TIME_T timeTmp;
    QC_TIME_T timeTOE, timeTOC;
    BOOL bUpdateEph;

    if (!pObj || !pEph)
    {
        GNSS_LOG_ERROR("VERIQC_orbit_addeph error:pobj=%d or peph=%d is null", pObj ? 1 : 0, pEph ? 1 : 0);
        return -1;
    }

    if (EPH_DATA_TYPE_GAL_REDUCED_CED == pEph->EphDataType)
    {
        return -1;
    }

    realTimeMode = pObj->pOpt->RealTimeMode;

    uniEphSys = pEph->SystemID;
    uniEphPrn = pEph->SatID;
#ifdef USING_QZSS_AS_GPS
    if (QZS == uniEphSys)/* convert QZSS to GPS */
    {
        uniEphSys = GPS;
        uniEphPrn = CHC_MAXPRNGPS_TRUE + uniEphPrn;
    }
#endif
    sys = VeriQCParseEphemerisSystem(uniEphSys);
    sat = SatNo(sys, uniEphPrn);
    if (sat <= 0 || sys == SYS_GLO || sys == SOLQ_NONE || NAV_DATA_STATE_VALID != pEph->EphState)
    {
        if (sys == SYS_GLO || sys == SOLQ_NONE)
        {
            GNSS_LOG_ERROR("VERIQC_orbit_addeph error:sys_id=%d,svid=%d", uniEphSys, uniEphPrn);
        }
        return -1;
    }

    if (!(pObj->pOpt->Sys & sys))
    {
        return -1;
    }

    if (EPH_DATA_TYPE_BDS_CNAV == pEph->EphDataType)
    {
        bBDS3Eph = TRUE;
    }

    IODE = pEph->IODE1;

    pNav = pObj->pNav;
    if (TRUE == realTimeMode) increNum = VERIQC_INCREA_EPH_ALM_REALTIME;
    else                      increNum = VERIQC_INCREA_EPH_ALM_POSTTIME;
    if (bBDS3Eph)
    {
        if (pNav->NumBD3 >= pNav->NumMaxBD3)
        {
            pTmpEph = VERIQC_MALLOC((pNav->NumMaxBD3 + increNum) * sizeof(CHC_EPH_T));
            if (pTmpEph)
            {
                memcpy(pTmpEph, pNav->pEphBD3, pNav->NumBD3 * sizeof(CHC_EPH_T));
                memset(pTmpEph + pNav->NumBD3, 0, (pNav->NumMaxBD3 + increNum - pNav->NumBD3) * sizeof(CHC_EPH_T));
                VERIQC_FREE(pNav->pEphBD3);
                pNav->pEphBD3 = pTmpEph;
                pNav->NumMaxBD3 += increNum;
            }
        }
    }
    else
    {
        if (pNav->Num >= pNav->NumMax)
        {
            pTmpEph = VERIQC_MALLOC((pNav->NumMax + increNum) * sizeof(CHC_EPH_T));
            if (pTmpEph)
            {
                memcpy(pTmpEph, pNav->pEph, pNav->Num * sizeof(CHC_EPH_T));
                memset(pTmpEph + pNav->Num, 0, (pNav->NumMax + increNum - pNav->Num) * sizeof(CHC_EPH_T));
                VERIQC_FREE(pNav->pEph);
                pNav->pEph = pTmpEph;
                pNav->NumMax += increNum;
            }
        }
    }
    if (realTimeMode)
    {
        if (bBDS3Eph)
        {
            for (ephIndex = 0; ephIndex < pNav->NumMaxBD3; ephIndex++)
            {
                timeTmp.Time = pNav->pEphBD3[ephIndex].Toe.Time;
                timeTmp.Sec = pNav->pEphBD3[ephIndex].Toe.Sec;
                /** If it is an existing ephemeris, then determine whether to update. */
                if (pNav->pEphBD3[ephIndex].Sat == sat)
                {
                    timeTOE = VeriQCUNIEphTime2Time(sys, pEph->Week, pEph->TOE);
                    deltaTOE = QCTimeMinusTime(&timeTOE, &timeTmp, NULL);

                    timeTmp.Time = pNav->pEphBD3[ephIndex].Toc.Time;
                    timeTmp.Sec = pNav->pEphBD3[ephIndex].Toc.Sec;
                    timeTOC = VeriQCUNIEphTime2Time(sys, pEph->Week, pEph->TOC);
                    delteTOC = QCTimeMinusTime(&timeTOC, &timeTmp, NULL);

                    bUpdateEph = FALSE;
                    if ((!IsAbsoluteDoubleZero(delteTOC)) ||
                        (pEph->Health != pNav->pEphBD3[ephIndex].SVH && 0 == pEph->Health) ||
                        (!IsAbsoluteDoubleZero(pEph->Tgd[0]) && IsAbsoluteDoubleZero(pNav->pEphBD3[ephIndex].Tgd[0])))
                    {
                        bUpdateEph = TRUE;
                    }

                    if ((bUpdateEph) ||
                        (!IsAbsoluteDoubleZero(deltaTOE)) ||
                        (pObj->pEphActiveValidFlagBD3[pEph->SatID - 1]))
                    {
                        VeriQCConvertUNIEphemeris(pEph, (struct _CHC_EPH_T*)&pNav->pEphBD3[ephIndex]);
                        pObj->pEphValidFlagBD3[pEph->SatID - 1] = 1;

                        pNav->pEphBD3[ephIndex].IODE = IODE; 

                    }
                    break;
                }
                /** If the satellite has not appeared before, then assign directly. */
                else if (pNav->pEphBD3[ephIndex].Sat == 0)
                {
                    VeriQCConvertUNIEphemeris(pEph, (struct _CHC_EPH_T*)&pNav->pEphBD3[ephIndex]);

                    pObj->pEphIndexBD3[pEph->SatID - 1] = pNav->NumBD3;
                    
                    pObj->pEphValidFlagBD3[pEph->SatID - 1] = 1;
                    pNav->NumBD3 += 1;
                    break;
                }
            }
        }
        else
        {
            for (ephIndex = 0; ephIndex < pNav->NumMax; ephIndex++)
            {
                timeTmp.Time = pNav->pEph[ephIndex].Toe.Time;
                timeTmp.Sec = pNav->pEph[ephIndex].Toe.Sec;
                /** if already have, judge if update */
                if (pNav->pEph[ephIndex].Sat == sat)
                {
                    timeTOE = VeriQCUNIEphTime2Time(sys, pEph->Week, pEph->TOE);
                    deltaTOE = QCTimeMinusTime(&timeTOE, &timeTmp, NULL);

                    timeTmp.Time = pNav->pEph[ephIndex].Toc.Time;
                    timeTmp.Sec = pNav->pEph[ephIndex].Toc.Sec;
                    timeTOC = VeriQCUNIEphTime2Time(sys, pEph->Week, pEph->TOC);
                    delteTOC = QCTimeMinusTime(&timeTOC, &timeTmp, NULL);

                    bUpdateEph = FALSE;
                    if ((!IsAbsoluteDoubleZero(deltaTOE)) ||
                        (pEph->Health != pNav->pEph[ephIndex].SVH && 0 == pEph->Health))
                    {
                        bUpdateEph = TRUE;
                    }

                    if ((bUpdateEph) || (!IsAbsoluteDoubleZero(delteTOC)) || pObj->pEphActiveValidFlag[sat - 1])
                    {
                        VeriQCConvertUNIEphemeris(pEph, (struct _CHC_EPH_T*)&pNav->pEph[ephIndex]);
                        pObj->pEphValidFlag[sat - 1] = 1;

                        pNav->pEph[ephIndex].IODE = IODE; 
                    }
                    break;
                }
                /** If the satellite has not appeared before, then assign directly. */
                else if (pNav->pEph[ephIndex].Sat == 0)
                {
                    VeriQCConvertUNIEphemeris(pEph, (struct _CHC_EPH_T*)&pNav->pEph[ephIndex]);

                    pObj->pEphIndex[sat - 1] = pNav->Num;

                    pNav->Num += 1;
                    pObj->pEphValidFlag[sat - 1] = 1;
                    if (SYS_CMP == sys)
                    {
                        pNav->NumBD2++;
                    }
                    break;
                }
            }
        }
    }
    else
    {
        if (bBDS3Eph)
        {
            for (ephIndex = 0; ephIndex < pNav->NumMaxBD3; ephIndex++)
            {
                if (pNav->pEphBD3[ephIndex].Sat == sat)
                {
                    bEphExist = TRUE;
                    break;
                }
                else if (pNav->pEphBD3[ephIndex].Sat == 0)
                {
                    VeriQCConvertUNIEphemeris(pEph, (struct _CHC_EPH_T*)&pNav->pEphBD3[pNav->NumBD3]);
                    pNav->NumBD3++;
                    break;
                }
            }
            if (TRUE == bEphExist)
            {
                for (ephIndex = pNav->NumBD3 - 1; ephIndex >= 0; ephIndex--)
                {
                    if (pNav->pEphBD3[ephIndex].Sat == sat)
                    {
                        timeTmp.Time = pNav->pEphBD3[ephIndex].Toe.Time;
                        timeTmp.Sec = pNav->pEphBD3[ephIndex].Toe.Sec;
                        timeTOE = VeriQCUNIEphTime2Time(sys, pEph->Week, pEph->TOE);
                        deltaTOE = QCTimeMinusTime(&timeTOE, &timeTmp, NULL);

                        timeTmp.Time = pNav->pEphBD3[ephIndex].Toc.Time;
                        timeTmp.Sec = pNav->pEphBD3[ephIndex].Toc.Sec;
                        timeTOC = VeriQCUNIEphTime2Time(sys, pEph->Week, pEph->TOC);
                        delteTOC = QCTimeMinusTime(&timeTOC, &timeTmp, NULL);

                        if (!IsAbsoluteDoubleZero(deltaTOE) ||
                            !IsAbsoluteDoubleZero(delteTOC) ||
                            (!IsAbsoluteDoubleZero(pEph->Tgd[0]) && IsAbsoluteDoubleZero(pNav->pEphBD3[pNav->NumBD3].Tgd[0])))
                        {
                            VeriQCConvertUNIEphemeris(pEph, (struct _CHC_EPH_T*)&pNav->pEphBD3[pNav->NumBD3]);
                            pNav->NumBD3++;
                        }
                        break;
                    }
                }
            }
        }
        else
        {
            for (ephIndex = 0; ephIndex < pNav->NumMax; ephIndex++)
            {
                if (pNav->pEph[ephIndex].Sat == sat)
                {
                    bEphExist = TRUE;
                    break;
                }
                else if (pNav->pEph[ephIndex].Sat == 0)
                {
                    VeriQCConvertUNIEphemeris(pEph, (struct _CHC_EPH_T*)&pNav->pEph[pNav->Num]);
                    pNav->Num++;
                    if (SYS_CMP == sys)
                    {
                        pNav->NumBD2++;
                    }
                    break;
                }
            }
            if (TRUE == bEphExist)
            {
                for (ephIndex = pNav->Num - 1; ephIndex >= 0; ephIndex--)
                {
                    if (pNav->pEph[ephIndex].Sat == sat)
                    {
                        timeTmp.Time = pNav->pEph[ephIndex].Toe.Time;
                        timeTmp.Sec = pNav->pEph[ephIndex].Toe.Sec;
                        timeTOE = VeriQCUNIEphTime2Time(sys, pEph->Week, pEph->TOE);
                        deltaTOE = QCTimeMinusTime(&timeTOE, &timeTmp, NULL);

                        timeTmp.Time = pNav->pEph[ephIndex].Toc.Time;
                        timeTmp.Sec = pNav->pEph[ephIndex].Toc.Sec;
                        timeTOC = VeriQCUNIEphTime2Time(sys, pEph->Week, pEph->TOC);
                        delteTOC = QCTimeMinusTime(&timeTOC, &timeTmp, NULL);

                        if (fabs(deltaTOE) > 0.0 || fabs(delteTOC) > 0.0)
                        {
                            VeriQCConvertUNIEphemeris(pEph, (struct _CHC_EPH_T*)&pNav->pEph[pNav->Num]);
                            pNav->Num++;
                            if (SYS_CMP == sys)
                            {
                                pNav->NumBD2++;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    return 0;

}

/********************************************************************************************
@brief   add glonass ephemeris data to orbit processor

@param   pObj[In]              orbit processor
@param   pGLOEph[In]           glonass ephemeris data

@author  CHC

@return  0:OK, -1:error
*********************************************************************************************/
int32_t VeriQCOrbitAddGLOEphemeris(VERIQC_ORBIT_PROCESSOR_T * pObj, const GLO_EPHEMERIS_T * pGLOEph)
{
    int gloEphIndex;
    int sat;
    int increNum;
    BOOL bEphExist;
    BOOL realTimeMode;
    CHC_NAV_T* pNav = NULL;
    CHC_GEPH_T* pTmpGloEph = NULL;
    QC_TIME_T timeTmp;
    QC_TIME_T timeTOE;
    double deltaTime;

    realTimeMode = pObj->pOpt->RealTimeMode;
    if (!pObj || !pGLOEph)
    {
        return -1;
    }
    sat = SatNo(SYS_GLO, pGLOEph->SatID);

    if (sat <= 0 || NAV_DATA_STATE_VALID != pGLOEph->EphState ||
        (EPH_DATA_TYPE_GLO != pGLOEph->EphDataType && EPH_DATA_TYPE_GLO_CDMA != pGLOEph->EphDataType))
    {
        GNSS_LOG_ERROR("VERIQC_orbit_add_glo_ephemeris error:state=%d, type=%d, svid=%d", pGLOEph->EphState, pGLOEph->EphDataType, pGLOEph->SatID);
        return -1;
    }
    if (pGLOEph->LeapYear == 0)
    {
        return -1;
    }

    if (!(pObj->pOpt->Sys & SYS_GLO))
    {
        return -1;
    }

    pNav = pObj->pNav;
    if (TRUE == realTimeMode) increNum = VERIQC_INCREA_EPH_ALM_REALTIME;
    else                      increNum = VERIQC_INCREA_EPH_ALM_POSTTIME;
    if (pNav->NumGLO >= pNav->NumGLOMax)
    {
        pTmpGloEph = VERIQC_MALLOC((pNav->NumGLOMax + increNum) * sizeof(CHC_GEPH_T));
        if (pTmpGloEph)
        {
            memcpy(pTmpGloEph, pNav->pGEph, pNav->NumGLO * sizeof(CHC_GEPH_T));
            memset(pTmpGloEph + pNav->NumGLO, 0, (pNav->NumGLOMax + increNum - pNav->NumGLO) * sizeof(CHC_GEPH_T));
            VERIQC_FREE(pNav->pGEph);
            pNav->pGEph = pTmpGloEph;
            pNav->NumGLOMax += increNum;
        }
    }
    if (realTimeMode)
    {
        for (gloEphIndex = 0; gloEphIndex < pNav->NumGLOMax; gloEphIndex++)
        {
            timeTmp.Time = pNav->pGEph[gloEphIndex].Toe.Time;
            timeTmp.Sec = pNav->pGEph[gloEphIndex].Toe.Sec;
            /** If it is an existing ephemeris, then determine whether to update. */

            if (pNav->pGEph[gloEphIndex].Sat == sat)
            {
                timeTOE = VeriQCGLOEphTime2Time(pGLOEph->LeapYear, pGLOEph->Nt, pGLOEph->Tb);

                deltaTime = QCTimeMinusTime(&timeTOE, &timeTmp, NULL);
                if (fabs(deltaTime) > 0.0 || ((pGLOEph->Bn >> 1) != pNav->pGEph[gloEphIndex].SVH && (pGLOEph->Bn >> 1) == 0) || pObj->pEphActiveValidFlag[sat - 1])
                {
                    VeriQCConvertGLOEphemeris(pGLOEph, (struct _CHC_GEPH_T*)&pNav->pGEph[gloEphIndex]);
                    pObj->pEphValidFlag[sat - 1] = 1;
                }
                break;
            }
            /** If the satellite has not appeared before, then assign directly. */
            if (pNav->pGEph[gloEphIndex].Sat == 0)
            {
                    VeriQCConvertGLOEphemeris(pGLOEph, (struct _CHC_GEPH_T*)&pNav->pGEph[gloEphIndex]);
                    pNav->NumGLO += 1;
                    pObj->pEphValidFlag[sat - 1] = 1;
                break;
            }
        }
    }
    else
    {
        bEphExist = 0;
        for (gloEphIndex = 0; gloEphIndex < pNav->NumGLOMax; gloEphIndex++)
        {
            if (pNav->pGEph[gloEphIndex].Sat == sat)
            {
                bEphExist = 1;
                break;
            }
            else if (pNav->pGEph[gloEphIndex].Sat == 0)
            {
                VeriQCConvertGLOEphemeris(pGLOEph, (struct _CHC_GEPH_T*)&pNav->pGEph[pNav->NumGLO]);
                pNav->NumGLO++;
                break;
            }
        }
        if (bEphExist)
        {
            for (gloEphIndex = pNav->NumGLO - 1; gloEphIndex >= 0; gloEphIndex--)
            {
                if (pNav->pGEph[gloEphIndex].Sat == sat)
                {
                    timeTmp.Time = pNav->pGEph[gloEphIndex].Toe.Time;
                    timeTmp.Sec = pNav->pGEph[gloEphIndex].Toe.Sec;
                    timeTOE = VeriQCGLOEphTime2Time(pGLOEph->LeapYear, pGLOEph->Nt, pGLOEph->Tb);
                    deltaTime = QCTimeMinusTime(&timeTOE, &timeTmp, NULL);

                    if (fabs(deltaTime) > 0.0)
                    {
                        VeriQCConvertGLOEphemeris(pGLOEph, (struct _CHC_GEPH_T*)&pNav->pGEph[pNav->NumGLO]);
                        pNav->NumGLO++;
                    }
                    break;
                }
            }
        }
    }

    return 0;
}

int32_t VeriQCOrbitRangeFit(VERIQC_ORBIT_PROCESSOR_T* pObj, const RM_RANGE_LOG_T* pRangeObs)
{
    int res = 0;
    CHC_OBS_T obs = { 0 };

    if (!pObj || !pRangeObs)
    {
        return -1;
    }
    if (!pObj->pOpt->EphInterpolation)
    {
        return -1;
    }
    if (pRangeObs->LogHeader.Week < 100)
    {
        return -1;
    }
    VeriQCMallocObs(&obs, VeriQCGetNsatFromRange(pRangeObs));
    VeriQCConvertObsRangeSimple(pObj->pOpt, pRangeObs, &obs);

    if (pObj->pOpt->RealTimeMode)
    {
        res = VeriQCOrbitFitting(CHC_SYS_ALL, 0, obs.pData, obs.ObsNum, pObj->pSatIptInfo, pObj->pNav, pObj->pOpt->MaxCPUTimeMsSatPara, pObj->pEphValidFlag, pObj->pEphActiveValidFlag, NULL, 0);
#if defined VERIQC_SURVEY
        res += VeriQCOrbitFitting(CHC_SYS_CMP, 0, obs.pData, obs.ObsNum, pObj->pSatIptInfoBD3, pObj->pNav, pObj->pOpt->MaxCPUTimeMsSatPara, pObj->pEphValidFlagBD3, pObj->pEphActiveValidFlagBD3, NULL, 1);
#endif
    }
    else
    {
        res = VeriQCOrbitFitting(CHC_SYS_ALL, 0, obs.pData, obs.ObsNum, pObj->pSatIptInfo, pObj->pNav, pObj->pOpt->MaxCPUTimeMsSatPara, NULL, NULL, NULL, 0);
#if defined VERIQC_SURVEY
        res += VeriQCOrbitFitting(CHC_SYS_CMP, 0, obs.pData, obs.ObsNum, pObj->pSatIptInfoBD3, pObj->pNav, pObj->pOpt->MaxCPUTimeMsSatPara, NULL, NULL, NULL, 1);
#endif
    }
    VeriQCFreeObs(&obs);

    return res;
}

int32_t VeriQCOrbitSatPos(VERIQC_ORBIT_PROCESSOR_T* pObj, const CHC_OBS_T* pObs, VERIQC_ORBIT_SOL_T* pSol, uint8_t IncrementUpdateMark)
{
    int i;
    int sateph = pObj->pOpt->SatEph;
    unsigned int numSat = 0;
    unsigned int numGAL = 0;
    //int stick, etick;
    //stick = tickget();
    if (!pObj || !pObs || pObs->ObsNum <= 0 || !pSol)
    {
        return -1;
    }

    pObj->pOpt->SatEph = EPHOPT_BRDC;
    if (pObj->pOpt->EphInterpolation)
    {
        numSat += SatPoss(pObj->pSatIptInfo, *(pObj->pEphTime), pObs->pData, pObs->ObsNum, pObj->pNav, pObj->pOpt->SatEph,
			pSol->pRs, pSol->pDts, pSol->pVar, pSol->pSVH, pSol->pType, pSol->pIODE, pSol->pToe, pSol->pExcludeSats, IncrementUpdateMark, 0, pSol->pSatNo, &numGAL);
#if defined VERIQC_SURVEY
        numSat += SatPoss(pObj->pSatIptInfoBD3, *(pObj->pEphTime), pObs->pData, pObs->ObsNum, pObj->pNav, pObj->pOpt->SatEph,
			pSol->pRs, pSol->pDts, pSol->pVar, pSol->pSVH, pSol->pType, pSol->pIODE, pSol->pToe, pSol->pExcludeSats, IncrementUpdateMark, 1, pSol->pSatNo, NULL);
#endif
        VeriQCCopyTimeFromSPP(&pObs->pData[0].Time, &pSol->UpdateTime);
    }
    else
    {
        numSat += SatPoss(NULL, *(pObj->pEphTime), pObs->pData, pObs->ObsNum, pObj->pNav, pObj->pOpt->SatEph,
            pSol->pRs, pSol->pDts, pSol->pVar, pSol->pSVH, pSol->pType, pSol->pIODE, pSol->pToe, pSol->pExcludeSats,0,0, pSol->pSatNo, &numGAL);
#if defined VERIQC_SURVEY
        numSat += SatPoss(NULL, *(pObj->pEphTime), pObs->pData, pObs->ObsNum, pObj->pNav, pObj->pOpt->SatEph,
			pSol->pRs, pSol->pDts, pSol->pVar, pSol->pSVH, pSol->pType, pSol->pIODE, pSol->pToe, pSol->pExcludeSats, 0,1, pSol->pSatNo, NULL);
#endif
        VeriQCCopyTimeFromSPP(&pObs->pData[0].Time, &pSol->UpdateTime);
        //reset ephvflg
        if (pObj->pEphValidFlag)
        {
            for (i = 0; i < pObs->ObsNum; i++)
            {
                if (pObj->pEphValidFlag[pObs->pData[i].Sat - 1])
                {
                    pObj->pEphValidFlag[pObs->pData[i].Sat - 1] = 0;
                }
            }
        }
    }

    pObj->pOpt->SatEph = sateph;
    //etick = tickget();
    //VERIQC_printf("VERIQC_orbit_satpos,stick=%d,etick=%d,dtick=%d\r\n", stick, etick, etick - stick);
    return 0;
}

int32_t VeriQCOrbitSetSelEphTime(VERIQC_ORBIT_PROCESSOR_T* pObj, const QC_TIME_T *pEphTime)
{
    if (!pObj)
    {
        return -1;
    }

    memcpy(pObj->pEphTime, pEphTime, sizeof(QC_TIME_T));
   
    return 1;
}

int32_t VeriQCOrbitOneSatCalc(VERIQC_ORBIT_PROCESSOR_T* pObj, const QC_TIME_T Time, VERIQC_SYSID_E SysID, unsigned int SatPRN,
    double* pSatPosVel, double* pSatClock)
{
    QC_TIME_T gTime;
    int sys, sat;
    BOOL bSuccess = FALSE;
    CHC_SATIPTINFO_T* pSatIptInfo = NULL;
    if ((!pObj) || (!pSatPosVel) || (!pSatClock))
    {
        return -1;
    }
    gTime.Time = Time.Time;
    gTime.Sec = Time.Sec;

#ifdef CHC_ENAQZS
#ifdef USING_QZSS_AS_GPS
    if (VERIQC_GNSS_SYSID_QZS == SysID)
    {
        SysID = VERIQC_GNSS_SYSID_GPS;
        SatPRN = CHC_MAXPRNGPS_TRUE + SatPRN - CHC_MINPRNQZS + 1;
    }
#endif
#endif
    sys = VeriQCConvertSysID(SysID);
    sat = SatNo(sys, SatPRN);

    if (0 == sat)
    {
        return -1;
    }

    if (pObj->pSatIptInfo)
    {
        if (sat == (&pObj->pSatIptInfo[sat - 1])->Sat)
        {
            pSatIptInfo = (&pObj->pSatIptInfo[sat - 1]);
        }
    }
    bSuccess = GetSatApprPosVelClock(NULL, gTime, sat, pObj->pNav, pSatPosVel, pSatClock, 0);

    if ((!bSuccess) && (CHC_SYS_CMP == sys))
    {
        pSatIptInfo = NULL;
        if (pObj->pSatIptInfoBD3)
        {
            if (sat == (&pObj->pSatIptInfoBD3[SatPRN - 1])->Sat)
            {
                pSatIptInfo = (&pObj->pSatIptInfoBD3[SatPRN - 1]);
            }
        }
        bSuccess = GetSatApprPosVelClock(NULL, gTime, sat, pObj->pNav, pSatPosVel, pSatClock, 1);
    }

    return (bSuccess ? 0 : -1);
}

/*********************************************************************************//**
@brief  sat calculate specify Eph Toe

@param pObj            [In]  orbit processor
@param TransTime       [In]  signal transmission time
@param Time            [In]  time of need
@param SysID           [In]  system ID
@param SatPRN          [In]  satellite PRN
@param pSatPosVel      [Out] sat coor(m): 3 Pos + 3 Vel
@param pSatClock       [Out] sat clock(s): clock + clock drift
@param BD3Flag         [In]  BD3 flag
@param Toe             [In]  specify Eph Toe

@return 0: success; -1: error

@author CHC
@date   2025/07/17
*************************************************************************************/
int32_t VeriQCOrbitSatCalcByToe(VERIQC_ORBIT_PROCESSOR_T* pObj, const QC_TIME_T TransTime, const QC_TIME_T Time, VERIQC_SYSID_E SysID, unsigned int SatPRN, int Toe,
    double* pSatPosVel, double* pSatClock)
{
    QC_TIME_T gTime;
    QC_TIME_T gTransTime;
    int Sys;
    int Sat;
    BOOL bSuccess = FALSE;
    CHC_SATIPTINFO_T* pSatIptInfo = NULL;
    if ((!pObj) || (!pSatPosVel) || (!pSatClock))
    {
        return -1;
    }
    gTime.Time = Time.Time;
    gTime.Sec = Time.Sec;

    gTransTime.Time = TransTime.Time;
    gTransTime.Sec = TransTime.Sec;

#ifdef CHC_ENAQZS
#ifdef USING_QZSS_AS_GPS
    if (VERIQC_GNSS_SYSID_QZS == SysID)
    {
        SysID = VERIQC_GNSS_SYSID_GPS;
        SatPRN = CHC_MAXPRNGPS_TRUE + SatPRN - CHC_MINPRNQZS + 1;
    }
#endif
#endif
    Sys = VeriQCConvertSysID(SysID);
    Sat = SatNo(Sys, SatPRN);

    if (0 == Sat)
    {
        return -1;
    }

    if (pObj->pSatIptInfo)
    {
        if (Sat == (&pObj->pSatIptInfo[Sat - 1])->Sat)
        {
            pSatIptInfo = (&pObj->pSatIptInfo[Sat - 1]);
        }
    }

    bSuccess = GetSatPosVelClockSpecifyToe(pSatIptInfo, gTransTime, gTime, Sat, pObj->pNav, pSatPosVel, pSatClock, 0, Toe);

    return (bSuccess ? 0 : -1);
}

int VeriQCOrbitFitting(int NavSys, int SolType, const CHC_OBSD_T* pObs, int Num, CHC_SATIPTINFO_T* pSatIptInfo, const CHC_NAV_T* pNav,
    int MaxCPUTimes, unsigned char* pEphValidFLAG, unsigned char* pEphActiveValidFlag, unsigned char* pAlmValidFlag, int BD3Flag)
{
    int res = 0;
    if (BD3Flag)
    {
        res = UpdateInterSatPosInfoBD3(SolType, pObs, Num, pSatIptInfo, pNav, MaxCPUTimes, pEphValidFLAG, pEphActiveValidFlag);
    }
    else
    {
        res = UpdateInterSatPosInfo(NavSys, SolType, pObs, Num, pSatIptInfo, pNav, MaxCPUTimes, pEphValidFLAG, pEphActiveValidFlag, pAlmValidFlag);
    }
    return res;
}

extern int VeriQCCreateSatInfoArray(CHC_SATIPTINFO_T** ppSatInfoArr, int Num)
{
    int nsatinfo = 0;
    if (!ppSatInfoArr)
    {
        return nsatinfo;
    }
    *ppSatInfoArr = (CHC_SATIPTINFO_T*)VERIQC_MALLOC(sizeof(CHC_SATIPTINFO_T) * Num);
    if (*ppSatInfoArr)
    {
        memset(*ppSatInfoArr, 0, sizeof(CHC_SATIPTINFO_T) * Num);
        InitSatIptInfo(*ppSatInfoArr, Num);
        nsatinfo = Num;
    }
    return nsatinfo;
}

extern int VeriQCReleaseSatInfoArray(CHC_SATIPTINFO_T** ppSatInfoArr, int Num)
{
    if (!ppSatInfoArr || !*ppSatInfoArr)
    {
        return -1;
    }
    EndSatIptInfo(*ppSatInfoArr, Num);
    VERIQC_FREE(*ppSatInfoArr);

    return 0;
}
