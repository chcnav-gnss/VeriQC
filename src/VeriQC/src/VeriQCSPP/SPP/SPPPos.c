
#include <stdlib.h>

#include "Logs.h"
#include "SPPBase.h"
#include "Coordinate.h"

#define SQR(x)      ((x)*(x))

#define MAXITR      60          /* max number of iteration for point pos */
#define CONMAXITR   5
#define VAR_IONO    25.0   
#define ERR_SAAS    0.3         /* saastamoinen model error std (m) */
#define VAR_CBIAS   0.09        //(0.3*0.3) (m)
#define VAR_CHECK   0.01

#define EXCLUDEVARAMP 1E10


#define MIXEFACT_GPS   1.0     /* error factor: GPS */
#define MIXEFACT_GAL   1.5625  //1.25*1.25   /* error factor:  Galileo*/                  
#define MIXEFACT_GLO   9.0//2.25    //1.5*1.5   /* error factor: GLONASS */                 
#define MIXEFACT_CMP   1.5625  //1.25*1.25   /* error factor: BeiDou */   

#define MAXSATTOROBUST 16
#define DESTITRYSYSNUM 15

static double s_GPSIFObsNoiseFact[] = {8.870,6.699};
static double s_BDSIFObsNoiseFact[] = {8.398,12.443,11.485,7.084,6.699};
static double s_GALIFObsNoiseFact[] = {7.888,6.699,7.256};

/*********************************************************************************//**
@brief get sat TGD from nav info

@param Sys     [In]  sys
@param Sat     [In]  sat No.
@param pNav    [In]  nav info
@param ObsType [In]  obs type
@param pTGD    [Out] TGD correct

@author CHC
@date   13 December 2023
*************************************************************************************/
static void GetNavTGD(int Sys, int Sat, const CHC_NAV_T* pNav, int ObsType, double* pTGD)
{
    int iSat;

    pTGD[0] = 0.0;
    pTGD[1] = 0.0;

    if ((SYS_BD3 == Sys) || (SYS_CMP == Sys))
    {
        if ((EQ_P4 == ObsType) ||
            (EQ_P5 == ObsType) ||
            (EQ_PLIF45 == ObsType))
        {
            for (iSat = 0; iSat < pNav->NumBD3; iSat++)
            {
                if (pNav->pEphBD3[iSat].Sat != Sat) continue;

                if (EQ_P4 == ObsType)
                {
                    pTGD[0] = pNav->pEphBD3[iSat].Tgd[0];
                }
                else if (EQ_P5 == ObsType)
                {
                    pTGD[0] = pNav->pEphBD3[iSat].Tgd[1];
                }
                else if (EQ_PLIF45 == ObsType)
                {
                    pTGD[0] = pNav->pEphBD3[iSat].Tgd[0];
                    pTGD[1] = pNav->pEphBD3[iSat].Tgd[1];
                }
                break;
            }
        }
        else
        {
            for (iSat = 0; iSat < pNav->Num; iSat++)
            {
                if (pNav->pEph[iSat].Sat != Sat) continue;
                pTGD[0] = pNav->pEph[iSat].Tgd[0];
                pTGD[1] = pNav->pEph[iSat].Tgd[1];
                break;
            }
            if ((EQ_PLIF34 == ObsType) ||
                (EQ_PLIF15 == ObsType))
            {
                for (iSat = 0; iSat < pNav->NumBD3; iSat++)
                {
                    if (pNav->pEphBD3[iSat].Sat != Sat) continue;

                    if (EQ_PLIF34 == ObsType) /* B3I-B1C */
                    {
                        pTGD[0] = 0.0;
                        pTGD[1] = pNav->pEphBD3[iSat].Tgd[0];
                    }
                    else if (EQ_PLIF15 == ObsType) /* B1I-B2a */
                    {
                        pTGD[1] = pNav->pEphBD3[iSat].Tgd[1];
                    }
                    break;
                }
            }
        }
    }
    else if (SYS_GLO == Sys)
    {
        for (iSat = 0; iSat < pNav->NumGLO; iSat++)
        {
            if (pNav->pGEph[iSat].Sat != Sat) continue;

            pTGD[0] = pNav->pGEph[iSat].DTaun;
            break;
        }
    }
    else
    {
        for (iSat = 0; iSat < pNav->Num; iSat++)
        {
            if (pNav->pEph[iSat].Sat != Sat) continue;
            pTGD[0] = pNav->pEph[iSat].Tgd[0];
            pTGD[1] = pNav->pEph[iSat].Tgd[1];
            break;
        }
    }

    pTGD[0] *= CLIGHT;
    pTGD[1] *= CLIGHT;
}


//lsq estimation
//m:obsnum
static int LSQEstSpt(const double *pA, const double *pTp, const double *pY, int M, double *pX, double *pQ, double *pSigma02,double *pRMS,double *pResidual, unsigned char SPT,const U1 *pValidFlag,double *pHTrans,double *pQTrans)
{
	int ix[LSQMAXPARAM]={0},k,i,j,stat=1;
	double x_[LSQMAXPARAM]={0};

	for(i=k=0;i<LSQMAXPARAM;i++)
	{
	   if(pValidFlag[i]) ix[k++]=i;
	}
	
	if(k<4) return 1;//error

	if(k<LSQMAXPARAM)
	{
	   memset(pHTrans,0,sizeof(double)*k*M);//m:obsnum;k:param num
	   for(i=0;i<M;i++)
	   {
	      for(j=0;j<k;j++)
		  {
		     pHTrans[j+k*i]=pA[ix[j]+i*LSQMAXPARAM];
		  }
	   }

	   stat = LSQPosFastSpt(pHTrans,pTp,pY,k,M,x_,pQTrans,pSigma02,pRMS,pResidual, NULL);
	   //restore
	   for(i=0;i<k;i++)
	   {
	       pX[ix[i]]=x_[i];
		   for(j=0;j<k;j++)
		   {
		      pQ[ix[i]*LSQMAXPARAM+ix[j]]=pQTrans[i*k+j];
		   }
	   }
	}
	else
	{
		memcpy(pHTrans,pA,sizeof(double)*k*M);
	    stat = LSQPosFastSpt(pA,pTp,pY,k,M,pX,pQ,pSigma02,pRMS,pResidual, NULL);
		memcpy(pQTrans,pQ,sizeof(double)*k*k);
	}

    return stat;
}
/* validate solution ---------------------------------------------------------*/
static int ValSolEx(const double *pAzEl, const int *pVSat, int Num,
					 const CHC_PRCOPT_T *pOpt, const double *pV, const double *pTp,int Nv, int Nx,
					 char *pMsg,double *pDop)
{
	double azels[MAXOBS*4],vv=0.0;//dop[4],
	int i,ns;
	int tnv;

    /* large gdop check */
    for (i = ns = 0; i<Num; i++) {
        if (!pVSat[i]) continue;
        azels[ns * 2] = pAzEl[i * 2];
        azels[1 + ns * 2] = pAzEl[1 + i * 2];
        ns++;
    }

    Dops(ns, azels, pOpt->EleMin, pDop);

    if (Nv <= Nx)
    {
        return 0;
    }
	/* chi-square validation of residuals */
	for(i=0;i<Nv;i++) 
	{
	   vv+=pV[i]*pTp[i]*pV[i];
	}
	tnv=Nv-Nx-1;
	if(tnv>29)
	{
	   tnv=29;
	}
    if (tnv < 0)
    {
        tnv = 0;
    }
	if (vv>g_Chisqr005[tnv]) {
		return 0;
	}

    if (pDop[0] <= 0.0 || pDop[0] > pOpt->MaxGDOP) 
    {
        return 0;
    }
	return 1;
}

/* pseudorange measurement error variance ------------------------------------*/
extern double VarErrSPPMix(const CHC_PRCOPT_T *pOpt, U1 ObsType, double Ele, int Sys)
{
    double fact = 1.0, varr, ionfreefact = 0.0;
    int index = -1, nsize = 0;
    double *noiseIF = NULL;

    switch (ObsType)
    {
    case EQ_PLIF12:index = 0; break;
    case EQ_PLIF13:index = 1; break;
    case EQ_PLIF14:index = ((Sys == SYS_CMP || Sys == SYS_BD3) ? -1 : 2); break;
    case EQ_PLIF15:index = 3; break;
    case EQ_PLIF45:index = 4; break;
    case EQ_PLIF34:index = ((Sys == SYS_CMP || Sys == SYS_BD3) ? 2 : -1); break;
    default:break;
    }

    switch (Sys)
    {
    case SYS_GPS:fact = MIXEFACT_GPS; noiseIF = s_GPSIFObsNoiseFact; nsize = sizeof(s_GPSIFObsNoiseFact) / sizeof(double); break;
    case SYS_BD3:
    case SYS_CMP:fact = MIXEFACT_CMP; noiseIF = s_BDSIFObsNoiseFact; nsize = sizeof(s_BDSIFObsNoiseFact) / sizeof(double); break;
    case SYS_GLO:fact = MIXEFACT_GLO; break;
    case SYS_GAL:fact = MIXEFACT_GAL; noiseIF = s_GALIFObsNoiseFact; nsize = sizeof(s_GALIFObsNoiseFact) / sizeof(double); break;
    default:break;
    }

    varr = SQR(pOpt->ErrorFactor[0])*(SQR(pOpt->ErrorFactor[1]) + SQR(pOpt->ErrorFactor[2]) /sin(Ele));

    if (noiseIF)
    {
        if (index >= 0 && index < nsize)
        {
            ionfreefact = noiseIF[index];
        }
    }
    if (ionfreefact>1.0)
    {
        varr *= ionfreefact;
    }

    varr *= fact;

    return varr;
}
/* pseudorange residuals -----------------------------------------------------*/
static int RessdPseu(const CHC_SSATLIST_T* pSsatlist, const MIX_PSEUOBS_T *pMixPseuObs, const CHC_NAV_T *pNav,
    const CHC_PRCOPT_T *pOpt, double *pRr, const int *pVSat, const double *pAzEl, double *pV, double *pVar)
{
    int refindex[SYSNUM];
    double r = 0.0, dion = 0, dtrp = 0.0, vmeas, vare, varchk, varnoise, varsnr, vion = 0.0, vtrp = 0.0, pos[3] = { 0 }, e[3] = { 0 }, P = 0.0;
    int i, k, g, nv = 0, sys, f;
    int prn = 0;
    MIX_PSEUOBSD_T mixpseuobsdi, mixpseuobsdj;
    int m, sm;
    double maxele = 0.0, vj, varj, lam[NFREQ]={0};
    double weekSecond;
    double trph_xs_r = 0.0, trpw_xs_r = 0.0;
    double sea_press_trop = 1013.25, sea_temp_trop = 15.0, humi_trop = 0.70;

    //selrefsat
    for (m = 0; m < SYSNUM; m++)
    {
        refindex[m] = -1;
        k = 0;
        maxele = 0.0;
        for (i = 0; i < pMixPseuObs->ObsNum; i++)
        {
            if (!pVSat[i])
            {
                continue;
            }
            mixpseuobsdi = pMixPseuObs->MixPseuObsd[i];
            sys = mixpseuobsdi.Sys;
            sm = mixpseuobsdi.SysIndex;
            if (m != sm)
            {
                continue;
            }
            g = 2 * i;
            if (pAzEl[g + 1] > maxele)
            {
                maxele = pAzEl[g + 1];
                refindex[m] = i;
                k++;
            }
        }

        if (k <= 1)
        {
            refindex[m] = -1;
        }
    }

    PosXYZ2LLH(pRr, pos);

    GetTropModelConstant(pos, sea_press_trop, sea_temp_trop, humi_trop, &trph_xs_r, &trpw_xs_r);

    weekSecond = QCTime2GPSTime(pMixPseuObs->Time, NULL);

    //form sdobs
    for (m = 0; m < SYSNUM; m++)
    {
        if (refindex[m] == -1)
        {
            continue;
        }
        mixpseuobsdj = pMixPseuObs->MixPseuObsd[refindex[m]];
        sys = mixpseuobsdj.Sys;// (mixpseuobsdj.sat, NULL);
        g = 2 * refindex[m];

        r = GeoDist(mixpseuobsdj.Rs, pRr, e);

        P = mixpseuobsdj.P;//psudorange 
                           //ionosphere error
        dion = 0.0;
        if (mixpseuobsdj.ObsType == EQ_P1 || mixpseuobsdj.ObsType == EQ_P2 || mixpseuobsdj.ObsType == EQ_P3 || mixpseuobsdj.ObsType == EQ_P4)
        {
            IonoCorrBroadcast(weekSecond, pNav, sys, pos, pAzEl + g, pSsatlist->pSsat + pSsatlist->Index[mixpseuobsdi.Sat - 1], &dion, &vion);
            
            if ((f=GetRawFreqByEquationType(mixpseuobsdj.ObsType)-NFREQ)!=0) {
                GetSysLam(sys,mixpseuobsdj.Sat,lam,pNav);
                dion*=(lam[f]>0.0?SQR(lam[f]/lam[0]):1.0);
            }
            if (fabs(dion)<1E-12)
            {
                vion = VAR_IONO;
            }
        }
        else //IF
        {
            vion = 0.0;
        }
    
        /* tropospheric corrections */
        dtrp = TropModelInputSeaLevelParaAdv(pMixPseuObs->Time, pos, pAzEl + g, trph_xs_r, trpw_xs_r);
        vtrp = SQR(ERR_SAAS / (sin(pAzEl[g + 1]) + 0.1));/* saastamoinen model */

        vmeas = mixpseuobsdj.VarMeasBias;
        vare = mixpseuobsdj.VarEphBias;
        varchk = mixpseuobsdj.VarChkBias;
        varsnr = mixpseuobsdj.VarSNRBias;
        varnoise = VarErrSPPMix(pOpt, mixpseuobsdj.ObsType, pAzEl[g + 1], sys);

        vj = P - (r - CLIGHT*mixpseuobsdj.Dts[0] + dion + dtrp);
        varj = varnoise + vmeas + vion + vtrp + varchk + varsnr + vare;

        for (i = 0; i < pMixPseuObs->ObsNum; i++)
        {
            if (!pVSat[i] || i == refindex[m])
            {
                continue;
            }

            mixpseuobsdi = pMixPseuObs->MixPseuObsd[i];

            sys = mixpseuobsdi.Sys;
            sm =  mixpseuobsdi.SysIndex;
            if (m != sm)
            {
                continue;
            }
            g = 2 * i;

            r = GeoDist(mixpseuobsdi.Rs, pRr, e);

            P = mixpseuobsdi.P;//psudorange 
                               //ionosphere error
            dion = 0.0;
            if (mixpseuobsdi.ObsType == EQ_P1 || mixpseuobsdi.ObsType == EQ_P2 || mixpseuobsdi.ObsType == EQ_P3 || mixpseuobsdi.ObsType == EQ_P4)
            {
                IonoCorrBroadcast(weekSecond, pNav, sys, pos, pAzEl + g, pSsatlist->pSsat + pSsatlist->Index[mixpseuobsdi.Sat - 1], &dion, &vion);

                if ((f=GetRawFreqByEquationType(mixpseuobsdi.ObsType)-NFREQ)!=0) {
                    GetSysLam(sys,mixpseuobsdi.Sat,lam,pNav);
                    dion*=(lam[f]>0.0?SQR(lam[f]/lam[0]):1.0);
                }
                if (fabs(dion)<1E-12)
                {
                    vion = VAR_IONO;
                }
            }
            else //IF
            {
                vion = 0.0;
            }

            /* tropospheric corrections */
            dtrp = TropModelInputSeaLevelParaAdv(pMixPseuObs->Time, pos, pAzEl + g, trph_xs_r, trpw_xs_r);
            vtrp = SQR(ERR_SAAS / (sin(pAzEl[g + 1]) + 0.1));/* saastamoinen model */

            vmeas = mixpseuobsdi.VarMeasBias;
            vare = mixpseuobsdi.VarEphBias;
            varchk = mixpseuobsdi.VarChkBias;
            varsnr = mixpseuobsdi.VarSNRBias;
            varnoise = VarErrSPPMix(pOpt, mixpseuobsdi.ObsType, pAzEl[g + 1], sys);


            pV[nv] = vj - (P - (r - CLIGHT*mixpseuobsdi.Dts[0] + dion + dtrp));

            /* error variance */
            pVar[nv] = varj + varnoise + vmeas + vion + vtrp + varchk + varsnr + vare;

            nv++;
        }
    }

    return nv;
}

static int FtrSPPByDopplerResult(const CHC_SSATLIST_T* pSsatlist, const MIX_PSEUOBS_T *pMixPseuObs, const CHC_PRCOPT_T *pOpt,
    const CHC_NAV_T *pNav, const int *pVSat, const double *pAzEl, CHC_SOL_T *pSol)
{
    double *v = NULL, *var = NULL;
    int n, nv, i;
    double tt;
    double rrd[3] = { 0 };
    double wvel[3] = { 0 }, dpos = 0.0;
    double sigma02c = 0.0, sigma02d = 0.0, wc, wd;

    if (pSol->Stat != SOLQ_SINGLE || pSol->VStat != SOLQ_SINGLE ||
        pSol->XSimSol.Stat != SOLQ_SINGLE || pSol->XSimSol.VStat != SOLQ_SINGLE)
    {
        return 0;
    }
    tt = QCTimeDiff(&pSol->Time, &pSol->XSimSol.SolTime);
    if (fabs(tt) > 180.0)
    {
        return 0;
    }
    for (i = 0; i < 3; i++)
    {
        wvel[i] = 0.5*(pSol->Rr[i + 3] + pSol->XSimSol.Rr[i + 3]);
        rrd[i] = pSol->XSimSol.Rr[i] + wvel[i] * tt;
        dpos += pow(pSol->Rr[i] - rrd[i], 2);
    }
    dpos = sqrt(dpos);

    if (dpos > 30.0)
    {
        return 0;
    }
    else if (dpos <= 1.0) 
    {
        for (i=0; i<3; i++) pSol->Rr[i]=rrd[i];
        return 0;
    }

    n = pMixPseuObs->ObsNum;

    v = Mat(n, 1); var = Mat(n, 1);
    if (!v || !var)
    {
        if (v) { VERIQC_FREE(v); v = NULL; }
        if (var) { VERIQC_FREE(var); var = NULL; }
        return 0;
    }

    nv = RessdPseu(pSsatlist, pMixPseuObs, pNav, pOpt, pSol->Rr, pVSat, pAzEl, v, var);
    for (i = 0; i < nv; i++)
    {
        if (var[i] <= 0.0)
        {
            continue;
        }
        sigma02c += v[i] * v[i] / var[i];
    }

    nv = RessdPseu(pSsatlist, pMixPseuObs, pNav, pOpt, rrd, pVSat, pAzEl, v, var);
    for (i = 0; i < nv; i++)
    {
        if (var[i] <= 0.0)
        {
            continue;
        }
        sigma02d += v[i] * v[i] / var[i];
    }

    if (sigma02c > 0.0&&sigma02d > 0.0)
    {
        wc = sigma02d / (sigma02c + sigma02d);
        wd = 1.0 - wc;
        if (dpos > 1.0)
        {
            if ((sigma02c < 0.2 * sigma02d))
            {
                wd = 0.0;
            }
            wd += 1.0 / dpos;
            if (wd > 1.0)
            {
                wd = 1.0;
            }
        }
        else
        {
            wd = 1.0;
        }
        wc = 1.0 - wd;
        for (i = 0; i < 3; i++)
        {
            pSol->Rr[i] = wc*pSol->Rr[i] + wd*rrd[i];
        }
    }

    if (v) { VERIQC_FREE(v); v = NULL; }
    if (var) { VERIQC_FREE(var); var = NULL; }
    return 1;
}

/* doppler residuals ---------------------------------------------------------*/
static int ResDopEx(const CHC_SSATLIST_T* pSsatlist, const CHC_OBSD_T *pObs, int Num, const double *pRs, const double *pDts,
				  const CHC_NAV_T *pNav, const double *pRr, const double *pX,
				  const double *pAzEl, const int *pVSat, double *pV, double *pH, U1 *pValidFlag,double *pW,double *pXs,int* pUsedSat)
{
	double lam,rate,pos[3],E[9],a[3],e[3],vs[3],cosel;
	int i,j,nv=0,k;
	int sys;
	int parmnum=LSQMAXPARAM;//vx,vy,vz,gps clk shift,glo,bds,gal
	double fact=1.0;

	PosXYZ2LLH(pRr,pos); PosXYZ2ENU(pos,E);

	for (i=0;i<Num&&i<MAXOBS;i++) {
        if (nv >= SPP_SATNUMLMT_INNER)
        {
            break;
        }
        sys = pObs[i].Sys;
		if(sys==SYS_NONE) continue;

        lam = pSsatlist->pSsat[pSsatlist->Index[pObs[i].Sat - 1]].Lam[0];
        if (!(pObs[i].ObsValidMask & BIT_MASK_DOP(0)))
        {
            continue;
        }

		if (fabs(lam)<1E-12||!pVSat[i]||Norm(pRs+3+i*6,3)<=0.0) {
			continue;
		}
		
		if(pXs[pObs[i].Sat-1]<1E-12)
		{
		    continue;//robust
		}
		k = GetClkIndex(sys);
		if (k < 0) continue;
        k += 3;
		switch(sys)
		{
		    case SYS_GPS:fact=1.0;break;
			case SYS_GLO:fact=0.2;break;
			case SYS_CMP:fact=0.8;break;
			case SYS_GAL:fact=0.8;break;
            case SYS_BD3:fact=0.8;break;
			default:break;
		}

		/* line-of-sight vector in ecef */
		cosel=cos(pAzEl[1+i*2]);
		a[0]=sin(pAzEl[i*2])*cosel;
		a[1]=cos(pAzEl[i*2])*cosel;
		a[2]=sin(pAzEl[1+i*2]);
		
		MatMul(MAT_MUL_TYPE_TN,3,1,3,1.0,E,a,0.0,e);

		/* satellite velocity relative to receiver in ecef */
		for (j=0;j<3;j++) vs[j]=pRs[j+3+i*6]-pX[j];

		/* range rate with earth rotation correction */
		rate= Dot(vs,e,3)+OMGE/CLIGHT*(pRs[4+i*6]*pRr[0]+pRs[1+i*6]*pX[0]-
			  pRs[3+i*6]*pRr[1]-pRs[i*6]*pX[1]);

		/* doppler residual */
		pV[nv]=-lam*pObs[i].D[pObs[i].Index[0]]-(rate+pX[k]-CLIGHT*pDts[1+i*2]);

		/* design matrix */
		for (j=0;j<3;j++)
		{
		   pH[j+nv*parmnum]=j<3?-e[j]:1.0;
		   if (pValidFlag) pValidFlag[j]=1;
		}
		//
		pUsedSat[nv]=pObs[i].Sat;
		pH[k+nv*parmnum]=1.0;
		pW[nv]=fact*a[2]*a[2];
		pW[nv]*=pXs[pObs[i].Sat-1];
		if (pValidFlag) pValidFlag[k]=1;
		nv++;
	}
	return nv;
}

static U1 ValVelEx(const double *pV, const double *pTp, int Nv, int Nx)
{
	double vv=0;
	int i,dof=0;
	dof=Nv-Nx;

	if(dof>=30) dof=30;
	if(dof<=0) return 0;

    /* chi-square validation of residuals */
    for (i=0;i<Nv;i++) {
        vv+=pV[i]*pTp[i]*pV[i];
    }

	if (dof>0&&vv>g_Chisqr005[dof-1])
	{
		return 0;
	}
	//20161020
	if(vv>3.0)
	{
	   return 0;
	}

	return 1;
}

/* estimate receiver velocity ------------------------------------------------*/
extern U1 EstVelEx(const CHC_OBSD_T *pObs, int Num, const double *pRs, const double *pDts,
				   const CHC_NAV_T *pNav, const CHC_PRCOPT_T *pOpt, CHC_SOL_T *pSol,
				   const double *pAzEl, const int *pVSat, CHC_SSATLIST_T *pSsatlist)
{
	double x[LSQMAXPARAM],dx[LSQMAXPARAM],Q[LSQMAXPARAM*LSQMAXPARAM],Q_[LSQMAXPARAM*LSQMAXPARAM]={0};
    int i = 0, j, ii, k, nv;
    int clkIndex;
	double *Pxs=NULL,*residual=NULL,*stdres=NULL,*tempres=NULL;
	U1 vflg[LSQMAXPARAM]={0};
	int usedSat[SPP_SATNUMLMT_INNER];
    double *ATQ = NULL, *ATQA=NULL, tempv, value = 0.0;
	double sigma02=0.0,rms=0.0,maxv,vstd,minv;
	double dxconv;
    double *v = NULL, *H = NULL, *H_ = NULL, *Pw = NULL;// , *PP;
	U1 endRobust=0;
	U1 maxRobust=6,forcedel=0,needrobust=0,dropn=0;
	U1 stat=0,info=0,conv=0,resmark=0;
    int realparamnum, firstnv = 0;
    ESTVEL_ELEMENT_T *estvel_element = NULL;
    CHC_SSAT_T *ssat = NULL;

    estvel_element = (ESTVEL_ELEMENT_T *)VERIQC_MALLOC(sizeof(ESTVEL_ELEMENT_T));
    memset(estvel_element, 0, sizeof(ESTVEL_ELEMENT_T));

    Pxs = estvel_element->Pxs;
    residual = estvel_element->Residual;
    stdres = estvel_element->STDRes;
    ATQ = estvel_element->ATQ;
    ATQA = estvel_element->ATQA;
    v = estvel_element->V;
    H = estvel_element->H;
    Pw = estvel_element->Pw;
    H_ = estvel_element->HTrans;

	memset(x,0,sizeof(double)*LSQMAXPARAM);

	for(i=0;i<Num;i++)
	{
		Pxs[pObs[i].Sat-1]=1.0;
	}
	
	while(!endRobust&&maxRobust>0)
	{
		for (i=0;i<MAXITR;i++) {

			memset(vflg,0,sizeof(U1)*LSQMAXPARAM);
			nv=ResDopEx(pSsatlist,pObs,Num,pRs,pDts,pNav,pSol->Rr,x,pAzEl,pVSat,v,H,vflg,Pw,Pxs,usedSat);
            if (i == 0 && maxRobust == 6)
            {
                firstnv = nv;
            }
			for(j=realparamnum=0;j<LSQMAXPARAM;j++) 
			{
				if(vflg[j]) realparamnum++;
			}
			if(nv<=realparamnum)
			{
				endRobust=1;
			    break;
			}
  
			memset(dx,0,sizeof(double)*LSQMAXPARAM);
			memset(Q,0,sizeof(double)*LSQMAXPARAM*LSQMAXPARAM);

			/* least square estimation */
			if ((info=LSQEstSpt(H,Pw,v,nv,dx,Q,&sigma02,&rms,residual,1,vflg,H_,Q_))) 
			{
				endRobust=1;
				
				break;
			}
			dxconv=0.0;
			for(j=0;j<LSQMAXPARAM;j++) 
			{
				if(vflg[j]) 
				{
					x[j]+=dx[j];
					dxconv += dx[j]*dx[j];
				}
			}

			dxconv=sqrt(dxconv);
			conv=0;
			if (dxconv<1E-6) 
			{
			    conv=1;
				stat=ValVelEx(residual,Pw,nv,realparamnum);
				if(stat)
				{
				   pSol->VStat=SOLQ_SINGLE;

                   for (j = 0; j < 3; j++)
                   {
                       pSol->Rr[j + 3] = x[j];
                       pSol->QVel[j] = (Q[j + j * LSQMAXPARAM] * sigma02);
                   }
                   pSol->QVel[3] = (Q[1] * sigma02);    /* cov xy */
                   pSol->QVel[4] = (Q[2 + LSQMAXPARAM] * sigma02); /* cov yz */
                   pSol->QVel[5] = (Q[2] * sigma02);    /* cov zx */

                   double pos[3] = { 0 };
                   double Qxyz[9] = { 0 };
                   double Qenu[9] = { 0 };
                   for (j = 0; j < 3; j++)
                   {
                       for (k = 0; k < 3; k++)
                       {
                           Qxyz[j * 3 + k] = Q[k + j * LSQMAXPARAM];
                       }
                   }
                   PosXYZ2LLH(pSol->Rr, pos);
                   CovENU(pos, Qxyz, Qenu);

                   for (clkIndex = 0; clkIndex < SYSNUM; clkIndex++)
                   {
                       if (vflg[3 + clkIndex])
                       {
                           pSol->DtrDot[clkIndex] = x[3 + clkIndex] / CLIGHT;
                       }
                   }

				   endRobust=1;
				  
				   break;
				}
			}

			if(conv&&!endRobust&&nv>=realparamnum+1)
			{
				if(rms>30.0)
				{ 
					forcedel=1;
				}
				//no ex obs to del
				if(nv<realparamnum+2) 
				{
					forcedel=0;
				}
				needrobust=0;
				if(!forcedel)
				{
					MatMul(MAT_MUL_TYPE_TN,nv,realparamnum,realparamnum,1.0,H_,Q_,0.0,ATQ);//ATQ=A'*Q
					MatMul(MAT_MUL_TYPE_NN,nv,nv,realparamnum,1.0,ATQ,H_,0.0,ATQA);//ATQA=ATQ*A
                    for (j = 0; j < nv; j++)
                    {
                        value = sigma02 / Pw[j] - sigma02 * ATQA[j + j * nv];
                        if (value < ZEROSVALUE) stdres[j] = 1.0;
                        else stdres[j] = residual[j] / (sqrt(value));
                    }

					for(j=0;j<nv;j++)
					{
						/* normalize residuals */
						vstd=fabs(stdres[j]);
	
                        Pxs[usedSat[j] - 1] *= GetPxsy(vstd, 1.5, 5.0,0);
						if(Pxs[usedSat[j]-1]<1.0)
						{
							needrobust=1;
						}
					}
					maxRobust--;
					if(maxRobust<=0) endRobust=1;
					if(needrobust)
					{
						dropn++;
					}
					else
					{
						forcedel=1;
					}
				}
				
				if(!endRobust)
				{
					if((dropn>2&&(rms>10.0||maxRobust<=2))||forcedel)
					{
						resmark=1;
						for(k=0;k<2;k++)
						{
						    if(!resmark) break;

						    tempres=stdres;
							if(k==1)
							{
							  tempres=residual;
							}

							maxv=fabs(tempres[0]);
							minv=maxv;
							j=0;
							resmark=0;

							for(ii=1;ii<nv;ii++)
							{
								tempv=fabs(tempres[ii]);
								if(maxv<tempv)
								{
									maxv=tempv;
									j=ii;
								}
								if(minv>tempv)
								{
								   minv=tempv;
								}
								if((maxv-minv<1E-6)||(maxv<minv*1.001))
								{
									resmark=1;
								}
							}
						}

						//ele,exclude the min ele
						if(resmark)
						{
							resmark=0;
							tempres=Pw;
							maxv=fabs(tempres[0]);
							minv=maxv;
							j=0;
							for(ii=1;ii<nv;ii++)
							{
								tempv=fabs(tempres[ii]);
								if(maxv<tempv)
								{
									maxv=tempv;
								}
								if(minv>tempv)
								{
									minv=tempv;
									j=ii;
								}
							}	
						}

						Pxs[usedSat[j]-1]=0.0;
					}
					break;
				}
			}
		}	

		if (i>=MAXITR) 
		{
			break;
		}
	}
    if (pSsatlist && pSol->VStat == SOLQ_SINGLE)/* only for rover */
    {
        for (i = 0; i<Num; i++)
        {
            Pxs[pObs[i].Sat - 1] = 1.0;
        }
        nv = ResDopEx(pSsatlist,pObs, Num, pRs, pDts, pNav, pSol->Rr, x, pAzEl, pVSat, v, H, NULL, Pw, Pxs, usedSat);
        for (i = 0; i < nv; i++)
        {
            if (pSsatlist->Index[usedSat[i] - 1] < 0)
            {
                continue;
            }
            ssat = &pSsatlist->pSsat[pSsatlist->Index[usedSat[i] - 1]];
            if (!pVSat[i])
            {
                ssat->PSRRate0Res = 0.0f;
                continue;
            }
            ssat->PSRRate0Res = (float)v[i];
        }
    }
    if (estvel_element) { VERIQC_FREE(estvel_element); estvel_element = NULL; }


	return stat;
}
static double GetErrRateLmt(int N)
{
    double rateLmt;

    if (N <= 0)
    {
        return 1.0;
    }

    if (N > 3)
    {
        rateLmt = 0.5;
    }
    else if (N == 3)
    {
        rateLmt = 0.60;
    }
    else
    {
        rateLmt = 0.999;
    }

    return rateLmt;
}
/*-------------------------------------------------------------------------------------------------*
* brief: Calculate the difference between the residual values of one satellite and those of other satellites 
*        Count the number of residuals less or greater than "errlmt" value
*--------------------------------------------------------------------------------------------------*/
int GetErrPseuSatBySPP(const CHC_SSATLIST_T *pSsatlist, const CHC_OBSD_T *pObs, int N, const double *pRs, const double *pDts,
    const CHC_NAV_T *pNav, const CHC_PRCOPT_T *pOpt, const double *pRr, U1 *pInvalid, double *pGRatio, double ErrLmt)
{
    int i, j, m, sys, sysi;
    double e[3], pos[3], r;
    double vres[MAXOBS*NFREQ];
    U1 vsats[MAXOBS*NFREQ];
    double dion = 0.0, vion = 0.0, dtss;
    double ionoTemp;
    double dtrp = 0.0, vtrp = 0.0;
    int nv, f,fx;
    double dv, rate, rateLmt;
    int gcnt = 0, scnt = 0, bcnt = 0;
    U1 vflg[MAXSAT];
    CHC_SSAT_T* pSsat;
    int iSat;
    double* pLam;
    int glbGcnt = 0;
    double weekSecond;
    double trph_xs_r = 0.0, trpw_xs_r = 0.0;
    double sea_press_trop = 1013.25, sea_temp_trop = 15.0, humi_trop = 0.70;

    if (N <= 0)
    {
        return 0;
    }
    for (i = 0; i < MAXSAT; i++)
    {
        vflg[i] = 1;
    }

    PosXYZ2LLH(pRr, pos);

    GetTropModelConstant(pos, sea_press_trop, sea_temp_trop, humi_trop, &trph_xs_r, &trpw_xs_r);

    weekSecond = QCTime2GPSTime(pObs[0].Time, NULL);

    for (m = 0; m < SYSNUM; m++)
    {
        sys = GetSystemIDByIndex(m);
        nv = 0;
        for (i = 0; i < N; i++)
        {
            sysi = pObs[i].Sys;
            if (sys != sysi)
            {
                continue;
            }

            r = GeoDist(pRs + i * 6, pRr, e);

            if (r <= 0.0) continue;//geometric distance

            dtss = CLIGHT*pDts[i * 2];

            iSat = pObs[i].Sat;
			pSsat = pSsatlist->pSsat + pSsatlist->Index[iSat - 1];
            IonoCorrBroadcast(weekSecond, pNav, sys, pos, pSsat->AzEl, pSsat, &dion, &vion);

            dtrp = TropModelInputSeaLevelParaAdv(pObs[i].Time, pos, pSsat->AzEl, trph_xs_r, trpw_xs_r);
            vtrp = SQR(ERR_SAAS / (sin(pSsat->AzEl[1]) + 0.1));/* saastamoinen model */

            pLam = pSsat->Lam;
            for (f = 0; f < NFREQ; f++)
            {
                fx = pObs[i].Index[f];
                if (pObs[i].ObsValidMask & BIT_MASK_CODE(f))
                {
                    ionoTemp = dion;
                    if (f)
                    {
                        ionoTemp *=(pLam[f]>0.0?SQR(pLam[f]/pLam[0]):1.0);
                    }
                    vres[nv] = pObs[i].P[fx] - (r - dtss + ionoTemp + dtrp);
                    vsats[nv] = pObs[i].Sat;
                    nv++;
                }
            }
        }

        if (nv <= 0)
        {
            continue;
        }

        for (i = 0; i < nv; i++)
        {
            gcnt = bcnt = 0;
            for (j = 0; j < nv; j++)
            {
                if (i == j)
                {
                    continue;
                }

                dv = vres[i] - vres[j];

                if (fabs(dv) < ErrLmt)
                {
                    gcnt++;
                }
                else
                {
                    bcnt++;
                }
            }

            scnt = gcnt + bcnt;

            if (scnt > 0)
            {
                rate = gcnt / (double)scnt;
                rateLmt = GetErrRateLmt(scnt);
                if (rate > rateLmt)
                {
                    vflg[vsats[i] - 1] = 0;
                }
            }
        }
    }

    for (i = 0; i < N; i++)
    {
        if (vflg[pObs[i].Sat - 1] == 0)
        {
            glbGcnt++;
        }
    }

    if (pGRatio)
    {
        *pGRatio = glbGcnt / (double)N;
    }


    if (pInvalid)
    {
        for (i = 0; i < MAXSAT; i++)
        {
            pInvalid[i] |= vflg[i];
        }
    }

    return glbGcnt;
}

/*********************************************************************************//**
@brief get pseu residual level

@param PseuVar [In] pseu var value

@return level

@author CHC
@date   15 December 2023
@note   1 level == 0.1m, Max Level: 250; Level == 251-254: reserved; Level == 255: SBAS error flag
*************************************************************************************/
static unsigned char GetPseuResidualLevel(double PseuVar)
{
    unsigned char level = 0;

    if (PseuVar <= 0.0)
    {
    }
    else if (PseuVar < 25.0)
    {
        level = (unsigned char)(PseuVar * 10.0);
    }
    else
    {
        level = 250;
    }

    return level;
}

int ResCodeExcar(const CHC_SSATLIST_T *pSsatlist, const MIX_PSEUOBS_T *pMixPseuObs, const CHC_PRCOPT_T *pOpt, const CHC_NAV_T *pNav,
    const double *pX, double *pV, double *pH, double *pVar, int *pVSat, double *pVarAmp, U1 *pValidFlag, double *pAzEl, int SpeciSys)
{
    double r = 0.0, dion = 0, dtrp = 0.0, vmeas, vare, varchk, varnoise, varsnr, vion = 0.0, vtrp = 0.0, rr[3] = { 0 }, pos[3] = { 0 }, dtr = 0.0, e[3] = { 0 }, P = 0.0, lam[NFREQ]={0};
	int i, j, k, g, nv = 0, sys, f;
	int prn = 0;
    int rowIndex = 0;
	MIX_PSEUOBSD_T mixpseuobsd;
	double trph_xs_r = 0.0, trpw_xs_r = 0.0;
	double sea_press_trop = 1013.25, sea_temp_trop = 15.0, humi_trop = 0.70;
    CHC_SSAT_T* pSsat;
    int iSatNo;
    double weekSecond;
	for (i = 0; i<3; i++)
	{
		rr[i] = pX[i];
	}

	PosXYZ2LLH(rr, pos);

	GetTropModelConstant(pos, sea_press_trop, sea_temp_trop, humi_trop, &trph_xs_r, &trpw_xs_r);

    weekSecond = QCTime2GPSTime(pMixPseuObs->Time, NULL);
	
	for (i = 0; i<pMixPseuObs->ObsNum; i++)
	{
		g = 2 * i;
		prn = 0; pVSat[i] = 0; pAzEl[g] = pAzEl[g + 1] = 0.0;
        mixpseuobsd = pMixPseuObs->MixPseuObsd[i];
        iSatNo = mixpseuobsd.Sat;
        pSsat = pSsatlist->pSsat + pSsatlist->Index[iSatNo - 1];
		/* reject duplicated observation data */
		if (i<pMixPseuObs->ObsNum - 1 && iSatNo == pMixPseuObs->MixPseuObsd[i + 1].Sat)
		{
			i++;
			continue;
		}

		sys =  mixpseuobsd.Sys;
		if (!sys)
		{
			continue;
		}

		if (!(sys&SpeciSys) || !(sys&pOpt->NavSys))
		{
			continue;
		}

        if (pVarAmp)
        {
            if (pVarAmp[i]>EXCLUDEVARAMP) continue;//exclude
        }
	
		r = GeoDist(mixpseuobsd.Rs, rr, e);

		if (r <= 0.0) continue;//geometric distance

		SatAzEl(pos, e, pAzEl + g);

		/*azimuth/elevation angle */
		if (pAzEl[g + 1]<pOpt->EleMin)
		{
			continue;
		}

		//sinel=sin(azel[g+1]);//ele
		P = mixpseuobsd.P;//psudorange 
						  //ionosphere error
		vion = dion = 0.0;

		if (pOpt->IonoOpt == IONOOPT_BRDC || pOpt->IonoOpt == IONOOPT_EST)
		{
			if (mixpseuobsd.ObsType == EQ_P1 || mixpseuobsd.ObsType == EQ_P2 || mixpseuobsd.ObsType == EQ_P3 || mixpseuobsd.ObsType == EQ_P4||
                mixpseuobsd.ObsType == EQ_P5 || mixpseuobsd.ObsType == EQ_P6 || mixpseuobsd.ObsType == EQ_P7)
			{
                IonoCorrBroadcast(weekSecond, pNav, sys, pos, pAzEl + g, pSsat, &dion, &vion);
                if ((f=GetRawFreqByEquationType(mixpseuobsd.ObsType)-NFREQ)!=0) {
                    GetSysLam(sys,mixpseuobsd.Sat,lam,pNav);
                    dion*=(lam[f]>0.0?SQR(lam[f]/lam[0]):1.0);
                }
				if (fabs(dion)<1E-12)
				{
					vion = VAR_IONO;
				}
			}
		}

		dtrp = vtrp = 0.0;

		if (pOpt->TropOpt == TROPOPT_SAAS || pOpt->TropOpt == TROPOPT_EST)
		{
            dtrp = TropModelInputSeaLevelParaAdv(pMixPseuObs->Time, pos, pAzEl + g, trph_xs_r, trpw_xs_r);
			vtrp = SQR(ERR_SAAS / (sin(pAzEl[g + 1]) + 0.1)); /**< saastamoinen model */
		}

		vmeas = mixpseuobsd.VarMeasBias;
		vare = mixpseuobsd.VarEphBias;
		varchk = mixpseuobsd.VarChkBias;
		varsnr = mixpseuobsd.VarSNRBias;
		varnoise = VarErrSPPMix(pOpt, mixpseuobsd.ObsType, pAzEl[g + 1], sys);

		k = GetClkIndex(sys);

		if (k<0) continue;
        rowIndex = nv * LSQMAXPARAM;
		/* design matrix */
		for (j = 0; j<3; j++)
		{
			pH[j + rowIndex] = -e[j];
			pValidFlag[j] = 1;
		}

		/* time system and receiver bias offset */
		dtr = pX[3 + k];
        pH[3 + rowIndex] = 0.0;
        pH[3 + 1 + rowIndex] = 0.0;
        pH[3 + 2 + rowIndex] = 0.0;
        pH[3 + 3 + rowIndex] = 0.0;
		pH[3 + k + rowIndex] = 1.0;
		pValidFlag[3 + k] = 1;

		/* pseudorange residual */
		pV[nv] = P - (r + dtr - CLIGHT*mixpseuobsd.Dts[0] + dion + dtrp);
		pVSat[i] = 1;
		/* error variance */
		pVar[nv] = varnoise + vmeas + vion + vtrp + varchk + varsnr + vare;
        if (pVarAmp)
        {
            pVar[nv] *= pVarAmp[i];
        }
		/*gnss_log_info("[%02d]sat=%03d,v=%5.2f,var=%5.2f,el=%5.2f,snr=%u", i,
			mixpseuobsd.sat, v[nv], var[nv], azel[2 * i + 1] * R2D, mixpseuobsd.SNRf1);*/
		nv++;
	}
	return nv;
}


static int GetToTryMark(const MIX_PSEUOBS_T *pMixPseuObs, const int *pVSat, int *pTrySysTbl, U1 *pToTryMark)
{
	int tn = 0, i;

	int nsatpersys[SYSNUM] = { 0 }, m, sysm;
	int sumns, realparam;
	U1 mark;

    pTrySysTbl[0] = SYS_GPS | SYS_CMP | SYS_BD3 | SYS_GAL;             /**< GPS+BDS+BDS3+GAL */
	pTrySysTbl[1] = SYS_GPS | SYS_CMP | SYS_BD3;
	pTrySysTbl[2] = SYS_GPS | SYS_GAL;
	pTrySysTbl[3] = SYS_GPS;
	pTrySysTbl[4] = SYS_CMP | SYS_BD3 | SYS_GAL;
	pTrySysTbl[5] = SYS_CMP | SYS_BD3;
	pTrySysTbl[6] = SYS_GAL;
	pTrySysTbl[7] = SYS_GPS | SYS_CMP | SYS_BD3 | SYS_GAL | SYS_GLO;
	pTrySysTbl[8] = SYS_GLO;
	pTrySysTbl[9] = SYS_GPS | SYS_CMP | SYS_BD3 | SYS_GLO;
	pTrySysTbl[10] = SYS_GPS | SYS_GAL | SYS_GLO;
	pTrySysTbl[11] = SYS_CMP | SYS_BD3 | SYS_GAL | SYS_GLO;
	pTrySysTbl[12] = SYS_GPS | SYS_GLO;
	pTrySysTbl[13] = SYS_CMP | SYS_BD3 | SYS_GLO;
	pTrySysTbl[14] = SYS_GAL | SYS_GLO;

    for (i = 0; i < pMixPseuObs->ObsNum; i++)
    {
        if (!pVSat[i]) continue;
        m = pMixPseuObs->MixPseuObsd[i].SysIndex;
        if (m >= 0 && m < SYSNUM)
        {
            nsatpersys[m]++;
        }
    }

	for (i = 0; i < DESTITRYSYSNUM; i++)
	{
		sumns = 0;
		mark = 0;
		realparam = 3;
		for (m = 0; m < SYSNUM; m++)
		{
			sysm = GetSystemIDByIndex(m);
			if (pTrySysTbl[i] & sysm)
			{
				if (nsatpersys[m] <= 0)
				{
					mark = 1;
					break;
				}
				else
				{
					sumns += nsatpersys[m];
					realparam++;
				}
			}
		}
        if (!mark)
        {
            
        }
        if (!mark && (sumns >= realparam + 1))
		{
			pToTryMark[i] = 1;
			tn++;
		}
        else pToTryMark[i] = 0;
	}

	return tn;
}

/* estimate receiver position ------------------------------------------------*/
static int EstPosExcar(const MIX_PSEUOBS_T *pMixPseuObs, const CHC_NAV_T* pNav,
	const CHC_PRCOPT_T *pOpt, CHC_SOL_T *pSol, int *pVSat, double *pAzEl, char *pMsg, float SPPGap, CHC_SSATLIST_T *pSsatlist)
{
	int realparamnum = 0;
    int i, j, g, k, h, info, stat = 0, nv, sizeH = 0, clkIndex;
	int n;
	double x[LSQMAXPARAM] = { 0 }, dx[LSQMAXPARAM] = { 0 };
	double Q[LSQMAXPARAM*LSQMAXPARAM], Q_[LSQMAXPARAM*LSQMAXPARAM] = { 0 };
	double *v = NULL, *H = NULL, *H_ = NULL, *var = NULL, *varamp = NULL,  *res = NULL, *stdres = NULL;
	U1 vflg[LSQMAXPARAM] = { 0 }, standardtat;
	double sigma02 = 0.0, rms = 0.0, dxconv, tt;
	CHC_ADJ_ROBUST_BUFFER_T *adjRobustBuffer = NULL;
	int *dsdindex = NULL, dsdindex1[MAXOBS], dsdindex2[MAXOBS];
	int trysystbl[DESTITRYSYSNUM];
	U1 totrymark[DESTITRYSYSNUM];
	int tot, tn, iter = 0, cn = 0, citer;
	U1 mstop = 0;
	double dxconvLmt;
    double dxconverr = 1.0E-4;
	int xtrycnt, vsattmp[SPP_SATNUMLMT_INNER];
    U1 memtype_adjRobustBuffer = 0;
    ESTPOS_EXCAR_ELEMENT_T *estpos_excar_element = NULL;
    CHC_SSAT_T *pSsat = NULL;

    double ave, std;
    double tempAve;

    uint64_t stick0 = 0, etick0 = 0, etick1 = 0, etick2 = 0, etick3 = 0, etick4 = 0, etick5 = 0;
#ifdef VERIQC_TICKGET_ENABLE
	uint64_t stick, etick;
	uint32_t scpucycle, ecpucycle;
	stick = tickgetus();
	scpucycle = cpucycle();
#endif

	n = pMixPseuObs->ObsNum;
    if (n > SPP_SATNUMLMT_INNER)
    {
        return 0;
    }
    sizeH = LSQMAXPARAM * n;
    estpos_excar_element = (ESTPOS_EXCAR_ELEMENT_T *)VERIQC_MALLOC(sizeof(ESTPOS_EXCAR_ELEMENT_T));
    memset(estpos_excar_element, 0, sizeof(ESTPOS_EXCAR_ELEMENT_T));

    v = estpos_excar_element->V;
    H = estpos_excar_element->H;
    H_ = estpos_excar_element->HTrans;
    var = estpos_excar_element->Var;
    varamp = estpos_excar_element->VarAmp;
    res = estpos_excar_element->Res;
    stdres = estpos_excar_element->STDRes;
    
	/* pseudorange residuals */
	nv = ResCodeExcar(pSsatlist, pMixPseuObs, pOpt, pNav, x, v, H, var, pVSat, varamp, vflg, pAzEl, pOpt->NavSys);

	tn = GetToTryMark(pMixPseuObs, pVSat, trysystbl, totrymark);

	if (tn <= 0)
	{
        if (estpos_excar_element) { VERIQC_FREE(estpos_excar_element); estpos_excar_element = NULL; }

		if (adjRobustBuffer)
		{
			EndAdjRobustBuffer(adjRobustBuffer);
            VERIQC_FREE(adjRobustBuffer);
			adjRobustBuffer = NULL;
		}

		return 0;
	}
    stick0 = TickGet();
	for (tot = 0; tot < DESTITRYSYSNUM; tot++)
	{
		if (mstop || iter >= MAXITR)
		{
			break;
		}
        //
		if (!totrymark[tot])
		{
			continue;
		}

		cn++;
		citer = 0;
		for (i = 0; i < LSQMAXPARAM; i++) x[i] = i<3 ? pSol->Rr[i] : 0.0;
		for (i = 0; i < pMixPseuObs->ObsNum; i++)
		{
			varamp[i] = 1.0;
		}
        
		while (iter< MAXITR)
		{
			iter++;
			citer++;
            etick4 = TickGet();
            etick5 = TickGet();
			memset(vflg, 0, sizeof(U1)*LSQMAXPARAM);
			//memset(H, 0, sizeof(double)*LSQMAXPARAM*n);
            etick2 = TickGet();
			/* pseudorange residuals */
			nv = ResCodeExcar(pSsatlist, pMixPseuObs, pOpt, pNav, x, v, H, var, pVSat, varamp, vflg, pAzEl, trysystbl[tot]);

			for (j = realparamnum = 0; j < LSQMAXPARAM; j++)
			{
				if (vflg[j]) realparamnum++;
			}
            if (nv <= realparamnum)
            {
                break;
            }

			for (j = 0; j < nv; j++) var[j] = 1.0 / var[j];
            etick3 = TickGet();
			memset(dx, 0, sizeof(double)*LSQMAXPARAM);
			memset(Q, 0, sizeof(double)*LSQMAXPARAM*LSQMAXPARAM);
            etick1 = TickGet();
			/* least square estimation */
			info = LSQEstSpt(H, var, v, nv, dx, Q, &sigma02, &rms, res, 1, vflg, H_, Q_);
			if (info)
			{
				break;
			}

			for (j = 0; j < LSQMAXPARAM; j++)
			{
				x[j] += dx[j];
			}

			dxconv = Dot(dx, dx, LSQMAXPARAM);
            etick0 = TickGet();
            GNSS_LOG_DEBUG("tot=%d,iter=%d,dxconv=%.2f,converr=%.2G, dtick,%d,-lsq,%d,-res,%d,-mem,%d,-memH,%d,memvfg,%d",
                tot, iter, dxconv, dxconverr, etick0 - stick0, etick0 - etick1, etick3 - etick2, etick1 - etick3, etick5 - etick4, etick2 - etick5);
			if (dxconv < dxconverr)
			{
                citer = 0;
				pSol->Type = 0;
				pSol->Time = pMixPseuObs->Time;

				for (j = 0; j < 6; j++) pSol->Rr[j] = j < 3 ? x[j] : 0.0;
				pSol->PosRms = 0.0;
				for (j = 0; j < 3; j++)
				{
					pSol->Qr[j] = Q[j + j*LSQMAXPARAM];
					pSol->PosRms += pSol->Qr[j];
				}
				if (pSol->PosRms > 0)
				{
					pSol->PosRms = sqrt(pSol->PosRms);
				}

				pSol->Qr[3] = Q[1];    /* cov xy */
				pSol->Qr[4] = Q[2 + LSQMAXPARAM]; /* cov yz */
				pSol->Qr[5] = Q[2];    /* cov zx */
				pSol->SatNum = (U1)nv;
				pSol->Age = 0.0;
				pSol->RMS = rms;
                pSol->ProcSatNum = nv;
                pSol->Iter = iter;
				stat = ValSolEx(pAzEl, pVSat, n, pOpt, res, var, nv, realparamnum, pMsg, pSol->Dop);

                if (stat && (nv > 10) && (pSol->PosRms > 10))
                {
                    ave = AvgSTD(v, nv, &std);
                    std *= 2.8;
                    for (j = 0; j < nv; j++)
                    {
                        tempAve = fabs(v[j]);
                        if ((tempAve > std) && (tempAve > 10.0))
                        {
                            stat = 0;
                            break;
                        }
                    }
                }

				if (stat)
				{
					pSol->Stat = SOLQ_SINGLE;

                    for (clkIndex = 0; clkIndex < SYSNUM; clkIndex++)
                    {
                        if (vflg[3 + clkIndex])
                        {
                            pSol->Dtr[clkIndex] = x[3 + clkIndex] / CLIGHT;
                        }
                    }

					tt = QCTimeDiff(&pSol->Time, &pSol->SPPSatUpdateTime);
					if (fabs(tt) >= SPPGap || nv > MAXSATTOROBUST)
					{
						pSol->SPPSatUpdateTime = pSol->Time;
						memset(pSol->SPPSat, 0, sizeof(U1)*MAXSAT);
						if (nv > MAXSATTOROBUST)
						{
							//drop sats when too much
							QuickSortD(res, nv, dsdindex2, 0,1);
							for (h = 0; h < nv - MAXSATTOROBUST; h++)
							{
								k = dsdindex2[h];
								for (j = g = 0; j < n; j++)
								{
									if (!pVSat[j]) continue;
									if (g == k)
									{
										pVSat[j] = 0;//drop
										break;
									}
									g++;
								}
							}
						}
						for (j = 0; j < n; j++)
						{
							if (!pVSat[j])
							{
								pSol->SPPSat[pMixPseuObs->MixPseuObsd[j].Sat - 1] = 1;
							}
						}
					}

					mstop = 1;
					break;
				}
                if (nv < realparamnum + 2)
                {
                    break;
                }
				//roubst
                if (!adjRobustBuffer)
                {
                    adjRobustBuffer = (CHC_ADJ_ROBUST_BUFFER_T *)VERIQC_MALLOC_FAST(sizeof(CHC_ADJ_ROBUST_BUFFER_T));
                    if (!adjRobustBuffer)
                    {
                        memtype_adjRobustBuffer = 0;
                        adjRobustBuffer = (CHC_ADJ_ROBUST_BUFFER_T *)VERIQC_MALLOC(sizeof(CHC_ADJ_ROBUST_BUFFER_T));
                    }
                    else
                    {
                        memtype_adjRobustBuffer = 1;
                    }
                    InitAdjRobustBuffer(adjRobustBuffer, n, LSQMAXPARAM);
                }
				
				/*standard residual*/
				ComputeAdjFactorSpt(adjRobustBuffer, H_, Q_, var, nv, realparamnum);
				standardtat = ComputeStandardResSpt(stdres, adjRobustBuffer, res, nv, sigma02, var);
				QuickSortD(res, nv, dsdindex2, 0,1);
				dsdindex = dsdindex2;
				k = dsdindex[0];
				if (standardtat)
				{
					QuickSortD(stdres, nv, dsdindex1, 0,1);
					dsdindex = dsdindex1;
					k = dsdindex[0];
					if (dsdindex2[0] != dsdindex1[0])
					{
						if (fabs(stdres[dsdindex1[0]]) < 1.5*fabs(stdres[dsdindex1[1]]) && fabs(stdres[dsdindex1[0]]) < 3.0)
						{
							if (fabs(res[dsdindex2[0]]) > 1.5*fabs(res[dsdindex2[1]]))
							{
								dsdindex = dsdindex2;
								k = dsdindex[0];/*the real index in v*/
							}
                            else
                            {
                                if (fabs(fabs(stdres[dsdindex1[0]]) - fabs(stdres[dsdindex1[1]])) < 1E-12)
                                {
                                    dsdindex = dsdindex2;
                                    k = dsdindex[0];
                                }
                            }
						}
					}
				}

				if (nv <= MAXSATTOROBUST)
				{
					for (j = g = 0; j < n; j++)
					{
						if (!pVSat[j]) continue;
						if (g == k)
						{
							varamp[j] = EXCLUDEVARAMP + 0.1;
							break;
						}
						g++;
					}
				}
				else
				{
					//delete sats
					for (h = 0; h < nv - MAXSATTOROBUST; h++)
					{
						k = dsdindex[h];
						for (j = g = 0; j < n; j++)
						{
							if (!pVSat[j]) continue;
							if (g == k)
							{
								varamp[j] = EXCLUDEVARAMP + 0.1;
								break;
							}
							g++;
						}
					}

				}
			}
			else if (citer>CONMAXITR)
			{
				xtrycnt = 0;
				for (h = tot + 1; h < DESTITRYSYSNUM; h++)
				{
					if (totrymark[h])
					{
						xtrycnt++;
					}
				}
				if (xtrycnt > 0)
				{
					dxconvLmt = (1.0 + 20.0 / (citer - CONMAXITR)) / xtrycnt;
					if (dxconv > dxconvLmt)
					{
						break;
					}
				}
			}
		}
	}
    if (pSsatlist && pSol->Stat == SOLQ_SINGLE)/* only for rover */
    {
        for (i = 0; i < SPP_SATNUMLMT_INNER; i++) varamp[i] = 1.0;
        nv = ResCodeExcar(pSsatlist, pMixPseuObs, pOpt, pNav, x, v, H, var, vsattmp, varamp, vflg, pAzEl, pOpt->NavSys);
        for (i = nv = 0; i < pMixPseuObs->ObsNum; i++)
        {
            if (vsattmp[i])
            {
                pSsat = &pSsatlist->pSsat[pSsatlist->Index[pMixPseuObs->MixPseuObsd[i].Sat - 1]];
                pSsat->AzEl[0] = pAzEl[2 * i];
                pSsat->AzEl[1] = pAzEl[2 * i + 1];
                pSsat->PSRRes = (float)v[nv++];
            }
        }
    }
    if (estpos_excar_element) { VERIQC_FREE(estpos_excar_element); estpos_excar_element = NULL; }

    if (adjRobustBuffer)
    {
        EndAdjRobustBuffer(adjRobustBuffer);
        if (memtype_adjRobustBuffer)
        {
            VERIQC_FREE_FAST(adjRobustBuffer);
        }
        else
        {
            VERIQC_FREE(adjRobustBuffer);
        }

        adjRobustBuffer = NULL;
    }
#ifdef VERIQC_TICKGET_ENABLE
	etick = tickgetus();
	ecpucycle = cpucycle();
	VERIQC_printf("T,%d,estpos_excar,st=%llu,et=%llu,dt=%lld,sc=%u,ec=%u,dc=%d,n=%d,i=%d,estpos_excar\r\n",
		taskget(),stick, etick, etick - stick, scpucycle, ecpucycle, ecpucycle - scpucycle, n, iter);
#endif
	return stat;
}

extern int EstPosPro(const MIX_PSEUOBS_T *pMixPseuObs, const CHC_NAV_T *pNav,
	const CHC_PRCOPT_T *pOpt, CHC_SOL_T *pSol, int *pVSat, double *pAzEl, char *pMsg, float SPPGap, CHC_SSATLIST_T *pSsatlist)
{
	return EstPosExcar(pMixPseuObs, pNav, pOpt, pSol, pVSat, pAzEl, pMsg, pOpt->SPPGap, pSsatlist);
}

extern double PRangeSF(int Sys,int Sat,int ObsType,double P, U1 Code, double *pVar,const CHC_NAV_T *pNav)
{
	double PC = 0.0, P1_C1, P2_C2, tgd1, tgd2;
	int f;
    double TGD[4];
	
	f=GetRawFreqByEquationType(ObsType)-NFREQ;

	if(f<0)
	{
	   return 0.0;
	}

    PC=P;

	P1_C1=pNav->CBias[Sat-1][1];
	P2_C2=pNav->CBias[Sat-1][2];
    //avoid error
    if (fabs(P1_C1) > 1E4) P1_C1 = 0.0;
    if (fabs(P2_C2) > 1E4) P2_C2 = 0.0;

	if (Code==CODE_L1C) 
	{
		PC+=P1_C1; /* C1->P1 */
	}
	else if(Code==CODE_L2C)
	{
	    PC+=P2_C2; /* C2->P2 */
	}

	//tgd
    GetNavTGD(Sys, Sat, pNav, ObsType, TGD);
	tgd1= TGD[0];
	tgd2= TGD[1];

    //avoid error
    if (fabs(tgd1) > 1E4) tgd1 = 0.0;
    if (fabs(tgd2) > 1E4) tgd2 = 0.0;

	if(ObsType==EQ_P1)
	{
		switch(Sys)
		{
		   case SYS_GPS:PC-=tgd1;break;
           case SYS_BD3:
		   case SYS_CMP:PC-=tgd1;break;
           case SYS_GLO:PC-=IF12_GLO_SIM_2*tgd1; break;
		   case SYS_GAL:PC-=tgd1;break;//E1
		   default:break;
		}
	}
	else if(ObsType==EQ_P2)
	{
		switch(Sys)
		{
		   case SYS_GPS:PC-=GAMMA_GPS_IF12*tgd1;break;
           case SYS_BD3:
		   case SYS_CMP:PC-=tgd2;break;
		   case SYS_GLO:PC+=IF12_GLO_SIM_1*tgd1; break;
		   case SYS_GAL:
			   {
				   if(fabs(tgd2)>0.0)
				   { 
					  PC-=GAMMA_GAL_IFE1E5b*tgd2;break;//tgd1/tgd2 a little different
				   }
				   else if(fabs(tgd1)>0.0)
				   {
					  PC-=GAMMA_GAL_IFE1E5b*tgd1;break;//tgd1/tgd2 a little different
				   }		   
			   }

		   default:break;
		}
	}
	else if(ObsType==EQ_P3)
	{
		switch(Sys)
		{
		   case SYS_GPS:break;
           case SYS_BD3:
		   case SYS_CMP:break;
		   case SYS_GLO:break;
		   case SYS_GAL:PC-=GAMMA_GAL_IFE1E5a*tgd1;break;//E5a
		   default:break;
		}
	}
	else
	{
        switch (Sys)
        {
        case SYS_GPS:break;
        case SYS_BD3:
        case SYS_CMP:PC -= tgd1; break;
        case SYS_GLO:break;
        case SYS_GAL:break;
        default:break;
        }
	}
	
	*pVar=VAR_CBIAS;

	return PC;
}

/*************************************************//**
@ brief : IF psedorange combination
@ param : char SysIndex               [I]     system index
@         int Sys,					  [I]     system type: SYS_???
@         int Sat,					  [I]     satitle No
@		  const double *pP,           [I]     psedurange
@         const u1 *pCode			  [I]     code 
@		  double *pVar				  [I]     variance
@         const nav_t*     pNav       [I]     navigation message
@         const short* pObsFreqIndex, [I]     Index list
@         int* pFirstFreq             [I0]    FirstFreqIndex in P\L\SNR\code list
@         int* pSecondFreq            [I0]    secondFreqIndex in P\L\SNR\code list
@ return: 0-error;1-success
@ note  : 
*********************************************************/
extern double PRangeIF(char SysIndex, int Sys, int Sat, int ObsType, const double *pP, const U1 *pCode, double *pVar, const CHC_NAV_T *pNav,
	const short* pObsFreqIndex, int* pFirstFreq, int* pSecondFreq)
{
	double PC = 0.0, P1, P2, P1_C1, P2_C2;
	int f;
	double xsPIF_P, xsPIF_N;
    double TGD[4];
	double tgd1, tgd2;
	int firstFreqIndex = 0;
	int secondFreqIndex = 0;
	U1 Code1 = 0;
	U1 Code2 = 0;
    if (Sys == SYS_GLO)
    {
        if (EQ_PLIF12 == ObsType)
        {
            firstFreqIndex = 0;
            secondFreqIndex = 1;
        }
        else
        {
            return 0.0;
        }
    }
    else
    {
        f = GetIFFreqByEquationType(SysIndex, ObsType, &firstFreqIndex, &secondFreqIndex) - NFREQ_IF;

        if (f < 0)
        {
            return 0.0;
        }
    }

	firstFreqIndex = pObsFreqIndex[firstFreqIndex];
	secondFreqIndex = pObsFreqIndex[secondFreqIndex];
	if (pFirstFreq != NULL)
	{
		*pFirstFreq = firstFreqIndex;
	}
	else
	{
	}
	if (pSecondFreq != NULL)
	{
		*pSecondFreq = secondFreqIndex;
	}
	else
	{
	}

	if (firstFreqIndex < 0 || secondFreqIndex < 0)
	{
		return 0.0;/**< target freq isn't tracked */
	}
	
	Code1 = pCode[firstFreqIndex];
	Code2 = pCode[secondFreqIndex];
	P1 = pP[firstFreqIndex];
	P2 = pP[secondFreqIndex];
    P1_C1=pNav->CBias[Sat-1][1];
	P2_C2=pNav->CBias[Sat-1][2];
	/** firstFreqIndex */
	if (Code1==CODE_L1C)
	{
		P1+=P1_C1; /**< C1->P1 */
	}
	else if(Code1==CODE_L2C)
	{
	    P1+=P2_C2; /**< C2->P2 */
	}

	/** secondFreqIndex */
	if (Code2==CODE_L1C)
	{
		P2+=P1_C1; /**< C1->P1 */
	}

	else if(Code2==CODE_L2C)
	{
	    P2+=P2_C2; /**< C2->P2 */
	}

    GetNavTGD(Sys, Sat, pNav, ObsType, TGD);
    tgd1 = TGD[0];
    tgd2 = TGD[1];

    if (Sys == SYS_GLO)
    {
        xsPIF_P = IF12_GLO_SIM_1;
        xsPIF_N = IF12_GLO_SIM_2;
        //P1 -= IF12_GLO_SIM_2 * tgd1;
        //P2 += IF12_GLO_SIM_1 * tgd1;

        PC = xsPIF_P * P1 + xsPIF_N * P2;
        *pVar = VAR_CBIAS;
        return PC;
    }

	if(Sys==SYS_CMP||Sys==SYS_BD3)
	{
	   switch (ObsType)
	   {
	   case EQ_PLIF12: /**< B1I-B2I */
		   /** OLD version: PC -= (GAMMA_CMP_IF12*tgd1 - tgd2) / (GAMMA_CMP_IF12 - 1); same as :TDG corr firstly, IF-Combination secondly */
		   P1 -= tgd1;
		   P2 -= tgd2;
		   break;
	   case EQ_PLIF13:/**< B1I-B3I */
		   /** OLD version: PC -= tgd1*IF13_CMP_1;  same as :TDG corr firstly, IF-Combination secondly */
		   P1-= tgd1;
		   break;
	   case EQ_PLIF34:/**< B3I-B1C */
		   /** PC -= tgd1*IF34_CMP_3; above reason */
		   P2 -= tgd2;
		   break;
	   case EQ_PLIF15:/**< B1I-B2a */
		   P1 -= tgd1;
		   P2 -= tgd2;
		   break;
	   case EQ_PLIF45:/**< B1C-B2a */
		   P1 -= tgd1;
		   P2 -= tgd2;
		   break;
	   default:
		   break;
	   }
	}
	else if(Sys==SYS_GPS)
	{
		switch (ObsType)
		{
		case EQ_PLIF12:/**< L1-L2 */
			break;
		case EQ_PLIF13:/**< L1-L5 */
			P1 -= tgd1;
            //P2 -= tgd1; /**< need to ISC */
			break;
		default:
			break;
		}
	}
	else if(Sys==SYS_GAL)
	{
		switch (ObsType)
		{
		case EQ_PLIF12:/**< E1-E5b */
			//P2 -= tgd2;  /**< no need to corr */
			break;
		case EQ_PLIF13:/**< E1-E5a */
			//P2 -= tgd1;  /**< no need to corr */
			break;
		case EQ_PLIF14:/**< E1-E5a+b */
			break;
		default:
			break;
		}
	}

    xsPIF_P = GetCoeffPIF1(SysIndex, f);
    xsPIF_N = GetCoeffPIF2(SysIndex, f);

	/** pseudorange IF Combination */
	PC = xsPIF_P*P1 + xsPIF_N*P2;
	*pVar=VAR_CBIAS;

	return PC;
}

static int GetPRange(MIX_PSEUOBSD_T* pMixPseuObs, int Sys, int Sat, char SysInd, int ObsType, int FreqIndex, const short* pObsFreqIndex,
    const double *pP, const U1 *pCode, const U1 *pSNR, double Vare, double VarChk, const CHC_NAV_T *pNav)
{
    double var = 0.0;
    int secondFreq = -1;

    if (ObsType == EQ_P1 || ObsType == EQ_P2 || ObsType == EQ_P3 || ObsType == EQ_P4 || ObsType == EQ_P5 || ObsType == EQ_P6 || ObsType == EQ_P7)
    {
        pMixPseuObs->P = PRangeSF(Sys, Sat, ObsType, pP[FreqIndex], pCode[FreqIndex], &var, pNav);
        pMixPseuObs->SNRf1 = (U1)(pSNR[FreqIndex]  + 0.5);
    }
    else if (ObsType == EQ_PLIF12 || ObsType == EQ_PLIF13 || ObsType == EQ_PLIF14 || ObsType == EQ_PLIF15 || ObsType == EQ_PLIF45 || ObsType == EQ_PLIF34)
    {
		pMixPseuObs->P = PRangeIF(SysInd, Sys, Sat, ObsType, pP, pCode, &var, pNav, pObsFreqIndex, NULL, &secondFreq);
        pMixPseuObs->SNRf1 = (U1)(pSNR[FreqIndex]  + 0.5);
        if (secondFreq > 0)
        {
            pMixPseuObs->SNRf2 = (U1)(pSNR[secondFreq] + 0.5);
        }
    }

    pMixPseuObs->Sat = Sat;
    pMixPseuObs->Sys = Sys;
    pMixPseuObs->SysIndex = SysInd;
    pMixPseuObs->ObsType = ObsType;
    pMixPseuObs->VarEphBias = Vare;  //epherr
    pMixPseuObs->VarChkBias = VarChk;//check err
    pMixPseuObs->VarMeasBias = var;
    pMixPseuObs->VarSNRBias = 0.0;
    return 1;
}

/*********************************************************************************//**
@brief get obs type to SPP by first freq index 

@param pObs           [In] one obs data
@param FirstFreqIndex [In] first freq index 
@param pOpt           [In] process option

@return obs type

@author CHC
@date   9 November 2023
*************************************************************************************/
static int GetObsTypeByFirstFreqIndex(const CHC_OBSD_T* pObs, int FirstFreqIndex, const CHC_PRCOPT_T* pOpt)
{
    int obsType = -1, obsTypeIF;
    int freqIndex, iFreqIndex, iFreqIF;
    int firstIF, secondIF;
    int firstIndex, secondIndex;
    int sys = pObs->Sys;

    //if ((CHC_IONOOPT_IFLC == pOpt->ionoopt) && (sys != SYS_GLO))
    if(sys != SYS_GLO)
    {
        freqIndex = -1;
        for (iFreqIndex = FirstFreqIndex + 1; iFreqIndex < NFREQ; iFreqIndex++)
        {
            if (pObs->ObsValidMask & BIT_MASK_CODE(iFreqIndex)) /**< select second valid P */
            {
                freqIndex = iFreqIndex;
                break;
            }
        }
        if (freqIndex > 0)
        { 
            for (iFreqIF = 0; iFreqIF < NFREQ_IF; iFreqIF++)
            {
                obsTypeIF = GetIFEquationType(pObs->SysIndex, (iFreqIF + NFREQ_IF), &firstIF, &secondIF);
                if (obsTypeIF > 0)
                {
                    if (((firstIF == FirstFreqIndex) && (secondIF == freqIndex)) ||
                        ((firstIF == freqIndex) && (secondIF == FirstFreqIndex)))
                    {
                        obsType = obsTypeIF;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        /** GLO P1 and P2 */
        if ((0 == FirstFreqIndex) && (pObs->ObsValidMask & BIT_MASK_CODE(1)))
        {
            /** not to use */
            //obsType = EQ_PLIF12;
            //firstIF = 0;
            //secondIF = 1;
        }
    }

    if (obsType >= 0)
    {
        /** not to dual freq corr when no TGD */
        if (((SYS_CMP == sys) || (SYS_BD3 == sys) || (SYS_GAL == sys)) &&
            ((EQ_PLIF14 == obsType) || (EQ_PLIF15 == obsType) || (EQ_PLIF45 == obsType) || (EQ_PLIF34 == obsType)))
        {
            obsType = -1;
        }
    }

    if (obsType >= 0)
    {
        firstIndex = pObs->Index[firstIF];
        secondIndex = pObs->Index[secondIF];
        if ((fabs(pObs->P[firstIndex] - pObs->P[secondIndex]) > 50) ||
            (fabs(pObs->SNR[firstIndex] - pObs->SNR[secondIndex]) > 15))
        {
            obsType = -1;
        }
    }

    if (obsType < 0)
    {
        obsType = GetRawEquationType(FirstFreqIndex + NFREQ);
    }
    return obsType;
}

/********************************************************************************************
@brief   Select Obs For SPP.
@param   pObs[In]            obs data
@param   ObsNum[In]          number of obs
@param   pOpt[In]            process option
@param   pRS[In]             satellite position/velocity
@param   pDts[In]            satellite clock/shift
@param   pVare[In]           satellite ephemeris error
@param   pSvh[In]            satellite svh flag
@param   pNav[In]            navigation message
@param   pSPPObs[Out]        selected obs data
@param   bSpecifySat[In]     use specified sats flag
@param   pSpecifySatList[In] specified sats list
@param   MaxSatNum[In]       max sat number for sbas/spp
@return  0-success;1-error
*********************************************************************************************/
extern int ExtractObsToSPP(const CHC_OBSD_T* pObs, int ObsNum, const CHC_PRCOPT_T* pOpt, const double* pRs, const double* pDts, const double* pVare,
    const CHC_NAV_T* pNav, MIX_PSEUOBS_T* pSPPObs, U1 bSpecifySat, const U1* pSpecifySatList, int MaxSatNum)
{
    double* obsQualityList;
#ifdef BDS23
    double sysQuality[SYSNUM] = { 15,13,0,11,13 };/* GPS,BDS2,R,GAL,BDS3 */
#else
    double sysQuality[SYSNUM] = { 15,13,0,11 };/* GCRE */
#endif
    int* sortIndex;
    int iObsIndex, iFreqIndex, iLoop, jLoop;
    int iSat, iSys;
    signed char iSnr;
    int rsIndex;
    int obsType;
    int freqIndex;
    int nUsedSat = 0;
    signed char* usedSatIndexList;
    signed char* usedSatFreqList;
    const CHC_OBSD_T* piObs;
    MIX_PSEUOBSD_T* pOneObs;
    double maxVare = 0.0;
    const double snrLmt = 30.0;
    const int snrLmtNs = 8;
    int snrGreatLmtNs = 0;
    signed char* bSnrGreatLmtList;
    VERIQC_MEM_MANAGER_T memManager;
    size_t mTotal = 0;

    BOOL bOnlyUseSingle = FALSE;
    BOOL bOnlyUseDual = FALSE;
    BOOL bEphDualBD2 = TRUE;
    unsigned int dualNum = 0;
    unsigned int singleNum = 0;
    unsigned char* pFreqObsTypeList;

    int firstFreqIndex = 0;
    int secondFreqIndex = 0;

    const unsigned int onlyUseDualNumLimit = 15;

    if (ObsNum <= 0)
    {
        return 0;
    }

    if (MaxSatNum > SPP_SATNUMLMT_INNER)
    {
        MaxSatNum = SPP_SATNUMLMT_INNER;
    }
    VeriQCMemManagerInit(&memManager);
    mTotal = ObsNum * (sizeof(double) + sizeof(int) + 3 * sizeof(signed char) + sizeof(unsigned char));
    VeriQCAddSize(&memManager, mTotal, 0);
    if (VeriQCAllocateWhole(&memManager) < 0)
    {
        return 0;
    }
    obsQualityList = (double*)VeriQCRequestMemory(&memManager, ObsNum * sizeof(double), 0);
    sortIndex = (int*)VeriQCRequestMemory(&memManager, ObsNum * sizeof(int), 0);
    usedSatIndexList = (signed char*)VeriQCRequestMemory(&memManager, ObsNum * sizeof(signed char), 0);
    usedSatFreqList = (signed char*)VeriQCRequestMemory(&memManager, ObsNum * sizeof(signed char), 0);
    bSnrGreatLmtList = (signed char*)VeriQCRequestMemory(&memManager, ObsNum * sizeof(signed char), 0);
    pFreqObsTypeList = (unsigned char*)VeriQCRequestMemory(&memManager, ObsNum * sizeof(unsigned char), 0);
    memset(bSnrGreatLmtList, 0, ObsNum * sizeof(signed char));

    if ((pNav->NumBD2) < 3 && (pNav->NumBD3 > 1))
    {
        bEphDualBD2 = FALSE;
    }

    for (iObsIndex = 0; iObsIndex < ObsNum; iObsIndex++)
    {
        pFreqObsTypeList[iObsIndex] = 0;
        usedSatFreqList[iObsIndex] = -1;

        piObs = pObs + iObsIndex;
        if (piObs->SysIndex < 0)
        {
            continue;
        }

        iSat = piObs->Sat;
        if (bSpecifySat && pSpecifySatList[iSat - 1])
        {
            continue;
        }
        if (!(piObs->Sys & pOpt->NavSys))
        {
            continue;
        }

        rsIndex = iObsIndex * 6;
        if (fabs(pRs[rsIndex]) < ZEROSVALUE || fabs(pRs[rsIndex + 1]) < ZEROSVALUE || fabs(pRs[rsIndex + 2]) < ZEROSVALUE)
        {
            continue;
        }

        freqIndex = -1;
        for (iFreqIndex = 0; iFreqIndex < NFREQ; iFreqIndex++)
        {
            if (piObs->ObsValidMask & BIT_MASK_CODE(iFreqIndex))
            {
                freqIndex = iFreqIndex;
                if ((!bEphDualBD2) &&
                    ((SYS_CMP == piObs->Sys) || ((SYS_BD3 == piObs->Sys))))
                {
                    obsType = GetRawEquationType(freqIndex + NFREQ);
                }
                else
                {
                    obsType = GetObsTypeByFirstFreqIndex(piObs, freqIndex, pOpt);
                }
                //obsType = GetRawEquationType(freqIndex + NFREQ);
                usedSatFreqList[iObsIndex] = (char) freqIndex;
                break;
            }
        }
        if (freqIndex >= 0)
        {
            pFreqObsTypeList[iObsIndex] = (unsigned char)obsType;
            if (obsType >= EQ_PLIF12)
            {
                dualNum++;
            }
            else
            {
                singleNum++;
            }
        }
    }

    if (dualNum > onlyUseDualNumLimit)
    {
        bOnlyUseDual = TRUE;
    }
    else if (singleNum > 5 * dualNum)
    {
        bOnlyUseSingle = TRUE;
    }

    pSPPObs->Time = pObs->Time;
    for (iObsIndex = 0; iObsIndex < ObsNum; iObsIndex++)
    {
        if (0 == pFreqObsTypeList[iObsIndex])
        {
            continue;
        }
        if (bOnlyUseDual)
        {
            if (pFreqObsTypeList[iObsIndex] < EQ_PLIF12)
            {
                continue;
            }
        }

        piObs = pObs + iObsIndex;
        freqIndex = usedSatFreqList[iObsIndex];
        if (bOnlyUseSingle)
        {
            if (pFreqObsTypeList[iObsIndex] >= EQ_PLIF12)
            {
                pFreqObsTypeList[iObsIndex] = GetRawEquationType(freqIndex + NFREQ);
            }
        }

        if (pVare[iObsIndex] > maxVare)
        {
            maxVare = pVare[iObsIndex];
        }

        iSnr = (signed char)(piObs->SNR[piObs->Index[freqIndex]] + 0.5);

        if (iSnr > snrLmt)
        {
            bSnrGreatLmtList[nUsedSat] = TRUE;
            snrGreatLmtNs++;
        }
        obsQualityList[nUsedSat] = iSnr + sysQuality[piObs->SysIndex];
        usedSatIndexList[nUsedSat] = (signed char)iObsIndex;
        usedSatFreqList[nUsedSat] = (signed char)freqIndex;
        pFreqObsTypeList[nUsedSat] = pFreqObsTypeList[iObsIndex];
        nUsedSat++;
    }
    if (snrGreatLmtNs > snrLmtNs && snrGreatLmtNs != nUsedSat) /* select snrLmtNs by SNR */
    {
        for (iLoop = 0; iLoop < nUsedSat; iLoop++)
        {
            if (FALSE == bSnrGreatLmtList[iLoop])
            {
                for (jLoop = iLoop; jLoop < nUsedSat - 1; jLoop++)
                {
                    obsQualityList[jLoop] = obsQualityList[jLoop + 1];
                    usedSatIndexList[jLoop] = usedSatIndexList[jLoop + 1];
                    usedSatFreqList[jLoop] = usedSatFreqList[jLoop + 1];
                    bSnrGreatLmtList[jLoop] = bSnrGreatLmtList[jLoop + 1];
                    pFreqObsTypeList[jLoop] = pFreqObsTypeList[jLoop + 1];
                }
                iLoop--;
                nUsedSat--;
            }
        }
    }
    if (nUsedSat > MaxSatNum)/* select sat to spp */
    {
        QuickSortD(obsQualityList, nUsedSat, sortIndex, 1, 1);
        for (iObsIndex = nUsedSat - MaxSatNum - 1; iObsIndex >= 0; iObsIndex--)
        {
            usedSatIndexList[sortIndex[iObsIndex]] = -1;
        }
    }

    for (iLoop = 0; iLoop < nUsedSat; iLoop++)
    {
        iObsIndex = usedSatIndexList[iLoop];
        if (iObsIndex < 0)
        {
            continue;
        }

        piObs = pObs + iObsIndex;
        iSat = piObs->Sat;
        iSys = piObs->Sys;
        rsIndex = iObsIndex * 6;
        freqIndex = piObs->Index[usedSatFreqList[iLoop]];
        pOneObs = pSPPObs->MixPseuObsd + pSPPObs->ObsNum;

        obsType = (int) pFreqObsTypeList[iLoop];

        GetPRange(pOneObs, iSys, iSat, piObs->SysIndex, obsType, freqIndex, piObs->Index,
            piObs->P, piObs->Code, piObs->SNR, pVare[iObsIndex], VAR_CHECK, pNav);
        //if (pVare[iObsIndex] > maxVare)
        //{
        //    maxVare = pVare[iObsIndex];
        //}
        pOneObs->Rs[0] = pRs[rsIndex];
        pOneObs->Rs[1] = pRs[rsIndex + 1];
        pOneObs->Rs[2] = pRs[rsIndex + 2];
        pOneObs->Dts[0] = pDts[iObsIndex * 2];
        pOneObs->Dts[1] = pDts[iObsIndex * 2 + 1];

        pSPPObs->ObsNum++;
    }
    if (maxVare > 0.0)/* vare standardization(<4.0) */
    {
        maxVare *= 0.25;
        for (iLoop = 0; iLoop < pSPPObs->ObsNum; iLoop++)
        {
            pSPPObs->MixPseuObsd[iLoop].VarEphBias /= maxVare;
        }
    }
    VeriQCMemManagerFree(&memManager);

    return 1;
}


static double RobustSPPByGRatio(const CHC_SSATLIST_T *pSsatlist, CHC_SOL_T *pSol, const CHC_OBSD_T *pObs, int N, const double *pRs, const double *pDts, const CHC_NAV_T *pNav, const CHC_PRCOPT_T *pOpt)
{
    double gratio_cur = 0.0, gratio_inf = 0.0, gratio_fin = 0.0, tt;
    int gcnt_cur = 0, gcnt_inf = 0, dgcntLmt = 1;
    double dpos, errLmt = 10.0, simrr[6] = { 0 }, dgratio, wgtsim, wgtc, vfact;
    unsigned int iPosVel;
    U1 curstat = 0, finstat = 0;
    double curPosVar = 0.0, prePosVar = 0.0;
    double curVelVar = 0.0, preVelVar = 0.0;
    BOOL bPosVarCheck = TRUE;
    double ratioMin = 0.5, ratioMax = 0.9;

    if (pSol->Stat == SOLQ_SINGLE)
    {
        gcnt_cur = GetErrPseuSatBySPP(pSsatlist, pObs, N, pRs, pDts, pNav, pOpt, pSol->Rr, NULL, &gratio_cur, errLmt);
        gratio_fin = gratio_cur;
        curstat = 1;
    }

    tt = QCTimeDiff(&pSol->Time, &pSol->XSimSol.SolTime);
    if (fabs(tt) > 180.0)
    {
        return gratio_fin;
    }

    if (pSol->XSimSol.Stat == SOLQ_SINGLE)
    {
        dpos = 0.0;

        for (iPosVel = 0; iPosVel < 3; iPosVel++)
        {
            simrr[iPosVel] = pSol->XSimSol.Rr[iPosVel];
        }
        vfact = 1.0;
        if (pSol->XSimSol.VStat != SOLQ_NONE)
        {
            for (iPosVel = 3; iPosVel < 6; iPosVel++)
            {
                simrr[iPosVel] = pSol->XSimSol.Rr[iPosVel];
            }
            vfact = 0.5;
        }
        if (pSol->VStat == SOLQ_SINGLE)
        {
            for (iPosVel = 0; iPosVel < 3; iPosVel++)
            {
                curVelVar += pSol->QVel[iPosVel];
            }
            if (curVelVar < 100)
            {
                for (iPosVel = 3; iPosVel < 6; iPosVel++)
                {
                    simrr[iPosVel] = vfact * (simrr[iPosVel] + pSol->Rr[iPosVel]);
                }
            }
        }

        if (curstat == 1)
        {
            for (iPosVel = 0; iPosVel < 3; iPosVel++)
            {
                simrr[iPosVel] = pSol->XSimSol.Rr[iPosVel] + simrr[iPosVel + 3] * tt;
                dpos += pow(simrr[iPosVel] - pSol->Rr[iPosVel], 2);

                curPosVar += pSol->Qr[iPosVel];
                prePosVar += pSol->XSimSol.Qr[iPosVel];
            }

            if (curPosVar < 0.5 * prePosVar)
            {
                bPosVarCheck = FALSE;
            }
            else if (prePosVar > 250)
            {
                if (curPosVar < prePosVar)
                {
                    bPosVarCheck = FALSE;
                }
            }

            dpos = sqrt(dpos);

            if ((dpos > 5.0) && bPosVarCheck)
            {
                dgcntLmt = (int)(errLmt - dpos);
                if (dgcntLmt < -3)
                {
                    dgcntLmt = -3;
                }
                
                //
                gcnt_inf = GetErrPseuSatBySPP(pSsatlist, pObs, N, pRs, pDts, pNav, pOpt, simrr, NULL, &gratio_inf, errLmt);
                
                if ((gcnt_inf >= 3) || ((N <= 5) && (curPosVar > 50 * prePosVar)))
                {

                    if ((fabs(gratio_cur - gratio_inf) < 0.1) &&
                        (((gratio_cur < ratioMin) && (gratio_inf < ratioMin) && (curPosVar < prePosVar)) ||
                         ((gratio_cur > ratioMax) && (gratio_inf > ratioMax) && (N > 10))))
                    {
                        wgtc = 1.0;
                    }
                    else
                    {
                        wgtc = 1.0 / dpos + (gcnt_cur - gcnt_inf) * 0.2 + (gratio_cur - gratio_inf) * 0.8;
                        if ((gratio_cur > 0.8) && (gratio_inf > 0.8) && (dpos > 15) && (wgtc < 0.2))
                        {
                            wgtc += 0.5;
                        }
                        else if ((curPosVar > 3 * prePosVar) && (wgtc > 0.5) && (dpos > 50.0))
                        {
                            wgtc = 0;
                        }
                    }
                    if (wgtc > 1.0)
                    {
                        wgtc = 1.0;
                    }
                    else if (wgtc < 0.0)
                    {
                        wgtc = 0.0;
                    }

                    wgtsim = 1.0 - wgtc;

                    for (iPosVel = 0; iPosVel < 3; iPosVel++)
                    {
                        simrr[iPosVel] = wgtc*pSol->Rr[iPosVel] + wgtsim*simrr[iPosVel];
                    }

                    gratio_fin = gratio_inf;
                    finstat = 1;
                }
                else if ((N >= 10) && ((gratio_cur < 0.3) && (gratio_inf < 0.3) && (dpos > 100)) && (fabs(tt) < 10.0))
                {
                    if (curPosVar > prePosVar)
                    {
                        wgtc = 0.0;
                    }
                    else
                    {
                        wgtc = 1.0;
                    }
                    wgtsim = 1.0 - wgtc;
                    for (iPosVel = 0; iPosVel < 3; iPosVel++)
                    {
                        simrr[iPosVel] = wgtc * pSol->Rr[iPosVel] + wgtsim * simrr[iPosVel];
                    }
				
                    gratio_fin = gratio_inf;
                    finstat = 1;
                }
            }
        }
        else
        {
            gcnt_inf = GetErrPseuSatBySPP(pSsatlist, pObs, N, pRs, pDts, pNav, pOpt, simrr, NULL, &gratio_inf, errLmt);
            if (gcnt_inf >= 3)
            {
                for (iPosVel = 0; iPosVel < 3; iPosVel++)
                {
                    prePosVar += pSol->XSimSol.Qr[iPosVel];
                    preVelVar += pSol->XSimSol.QVel[iPosVel];
                }
                dgratio = gratio_inf - pSol->XSimSol.SPPRatio;
                if ((dgratio>-0.20) && (gratio_inf>0.20) && (prePosVar < 100) && (preVelVar < 30))
                {
                    finstat = 1;
                    gratio_fin = gratio_inf;
                }
            }
        }
    }
    
    if (finstat)
    {
        for (iPosVel = 0; iPosVel < 6; iPosVel++)
        {
            pSol->Rr[iPosVel] = simrr[iPosVel];
            pSol->Qr[iPosVel] = pSol->XSimSol.Qr[iPosVel];
            pSol->QVel[iPosVel] =  pSol->XSimSol.QVel[iPosVel];
        }
        pSol->Stat = pSol->XSimSol.Stat;
        pSol->VStat = pSol->XSimSol.VStat;
    }

    return gratio_fin;
}

extern int PNTPosExNew(const CHC_OBSD_T *pObs, int Num, const CHC_NAV_T *pNav, const CHC_PRCOPT_T *pOpt, 
                         CHC_SOL_T *pSol, char *pMsg, const double *pRs, const double *pDts,const double *pVar,
                         CHC_SSATLIST_T *pSsatlist)
{
    int stat = 0, * vsat = NULL, * vsat_ = NULL;
	int i,j;
    PNTPOS_ELEMENT_T *pntpos_element = NULL;
    double *azel = NULL, *azel_ = NULL;
    MIX_PSEUOBS_T* mixpseuobs = NULL;
	U1 sppspecifysat=0;
	double tt;
    double gratio_fin;

#ifdef VERIQC_TICKGET_ENABLE
	uint64_t stick, etick;
	uint32_t scpucycle, ecpucycle;
	stick = tickgetus();
	scpucycle = cpucycle();
#endif

	pSol->Stat=SOLQ_NONE;
 	pSol->VStat=SOLQ_NONE;

	if (Num<=0) {strcpy(pMsg,"no observation data"); return 0;}

	pSol->Time=pObs[0].Time; pMsg[0]='\0';

    pntpos_element = (PNTPOS_ELEMENT_T *)VERIQC_MALLOC(sizeof(PNTPOS_ELEMENT_T));
    memset(pntpos_element, 0, sizeof(PNTPOS_ELEMENT_T));

    vsat = pntpos_element->VSat;
    vsat_ = pntpos_element->VSatSort;
    azel = pntpos_element->AzEl;
    azel_ = pntpos_element->AzElSort;
    mixpseuobs = &(pntpos_element->MixPseuObs);

    tt = QCTimeDiff(&pSol->Time, &pSol->SPPSatUpdateTime);
    if (fabs(tt) < pOpt->SPPGap)
    {
        sppspecifysat = 1;
    }
	
    ExtractObsToSPP(pObs, Num, pOpt, pRs, pDts, pVar, pNav, mixpseuobs, sppspecifysat, pSol->SPPSat, SPP_SATNUMLMT_INNER);

	if (mixpseuobs->ObsNum<=0) 
	{
        if (pntpos_element) { VERIQC_FREE(pntpos_element); pntpos_element = NULL; }
		return 0;
	}
#ifdef VERIQC_TICKGET_ENABLE
    stick = tickget();
#endif
    stat = EstPosPro(mixpseuobs, pNav, pOpt, pSol, vsat, azel, pMsg, pOpt->SPPGap, pSsatlist);
#ifdef VERIQC_TICKGET_ENABLE
    etick = tickget();
    gnss_log_debug("estpos_pro use %d ms", etick - stick);
#endif
	if (stat) 
	{
		//azel/vsat
		for(i=0;i<Num;i++)
		{
		   for(j=0;j<mixpseuobs->ObsNum;j++)
		   {
		       if(pObs[i].Sat==mixpseuobs->MixPseuObsd[j].Sat)
			   {
				   vsat_[i]=vsat[j];
				   azel_[2*i]=azel[2*j];
				   azel_[2*i+1]=azel[2*j+1];
			       break;
			   }
		   }
		}

		if(STATION_ROVER==pObs[0].AntID&&pOpt->CroverDopplerVel)
		{
#ifdef VERIQC_TICKGET_ENABLE
            stick = tickget();
#endif
            EstVelEx(pObs, Num, pRs, pDts, pNav, pOpt, pSol, azel_, vsat_, pSsatlist);
#ifdef VERIQC_TICKGET_ENABLE
            etick = tickget();
            gnss_log_debug("estvel_ex use %d ms", etick - stick);
#endif
		}

        FtrSPPByDopplerResult(pSsatlist, mixpseuobs, pOpt, pNav, vsat, azel, pSol);
	}

    if (SOLQ_NONE != pSol->Stat)
    {
        UpdateSatAzEl(pSsatlist, pObs, Num, pRs, pSol->Rr);
    }

    gratio_fin = RobustSPPByGRatio(pSsatlist, pSol, pObs, Num, pRs, pDts, pNav, pOpt);

    if (pSol->Stat == SOLQ_SINGLE)
    {
        //xsimsol
        pSol->XSimSol.SolTime = pSol->Time;
        for (i = 0; i < 6; i++)
        {
            pSol->XSimSol.Rr[i] = pSol->Rr[i];
            pSol->XSimSol.Qr[i] = pSol->Qr[i];
            pSol->XSimSol.QVel[i] = pSol->QVel[i];
        }
        pSol->XSimSol.Stat = pSol->Stat;
        pSol->XSimSol.VStat = pSol->VStat;
        pSol->XSimSol.SPPRatio = (float)gratio_fin;
    }

    if (pntpos_element) { VERIQC_FREE(pntpos_element); pntpos_element = NULL; }

#ifdef VERIQC_TICKGET_ENABLE
	etick = tickgetus();
	ecpucycle = cpucycle();
	VERIQC_printf("T,%d,pntpos_ex_new,st=%llu,et=%llu,dt=%lld,sc=%u,ec=%u,dc=%d,pntpos_ex_new\r\n",
		taskget(),stick, etick, etick - stick, scpucycle, ecpucycle, ecpucycle - scpucycle);
#endif

	return stat;
}

/********************************************************************************************
@brief   update satellite azimuth and elevation in ssatlist

@param   pSsatList     [In/Out] satellite list information
@param   pObs          [In] obs structure pointer
@param   ObsNum        [In] number of obs
@param   pRs           [In] satellite position and velocity(corresponding with pObs order)
@param   pRr           [In] receiver position

@author  CHC

@return  number of updated satellite
*********************************************************************************************/
extern int UpdateSatAzEl(CHC_SSATLIST_T* pSsatList, const CHC_OBSD_T* pObs, int ObsNum, const double* pRs, const double* pRr)
{
    CHC_SSAT_T* pSsat;
    int iObs, iSat;
    int rsIndex;
    double e[3], normE;
    double E[9], enu[3];
    double pos[3];
    int updateNum = 0;

    if (fabs(pRr[0]) < ZEROSVALUE || fabs(pRr[1]) < ZEROSVALUE || fabs(pRr[2]) < ZEROSVALUE)
    {
        return updateNum;
    }
    PosXYZ2LLH(pRr, pos);
    PosXYZ2ENU(pos, E);
    for (iObs = 0; iObs < ObsNum; iObs++)
    {
        iSat = pObs[iObs].Sat;
        pSsat = &(pSsatList->pSsat[pSsatList->Index[iSat - 1]]);
        if (pSsat->AzEl[0] > 0.0)
        {
            continue;
        }
        rsIndex = iObs * 6;
        if (fabs(pRs[rsIndex]) < ZEROSVALUE || fabs(pRs[rsIndex + 1]) < ZEROSVALUE || fabs(pRs[rsIndex + 2]) < ZEROSVALUE)
        {
            pSsat->SatStatus = SAT_STATUS_NOEPH;
            continue;
        }
        e[0] = pRs[rsIndex] - pRr[0];
        e[1] = pRs[rsIndex + 1] - pRr[1];
        e[2] = pRs[rsIndex + 2] - pRr[2];
        normE = sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
        e[0] /= normE;
        e[1] /= normE;
        e[2] /= normE;

        enu[0] = E[0] * e[0] + E[3] * e[1] + E[6] * e[2];
        enu[1] = E[1] * e[0] + E[4] * e[1] + E[7] * e[2];
        enu[2] = E[2] * e[0] + E[5] * e[1] + E[8] * e[2];
        if (fabs(enu[0]) < ZEROSVALUE && fabs(enu[1]) < ZEROSVALUE)
        {
            pSsat->AzEl[0] = 1E-12;
        }
        else
        {
            pSsat->AzEl[0] = atan2(enu[0], enu[1]);
        }
        if (pSsat->AzEl[0] < 0.0) pSsat->AzEl[0] += 2 * PI;
        pSsat->AzEl[1] = asin(enu[2]);

        updateNum++;
    }

    return updateNum;
}