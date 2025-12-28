/**********************************************************************//**
          VeriQC
    ephemeris/almanac fitting and interpolation
*-
@file   Ephemeris.c
@author CHC
@date   2023/04/10
@brief  satellite ephemeris and clock functions
**************************************************************************/

#include "GNSSBase.h"
#include "Logs.h"
#include "Common/GNSSNavDataType.h"
#include "Common/GNSSConstants.h"
#include "Coordinate.h"

/* constants and macros ------------------------------------------------------*/

#define SQR(x)   ((x)*(x))
#define RE_GLO   6378136.0        /* radius of earth (m)            ref [2] */
#define MU_GPS   3.9860050E14     /* gravitational constant         ref [1] */
#define MU_GLO   3.9860044E14     /* gravitational constant         ref [2] */
#define MU_GAL   3.986004418E14   /* earth gravitational constant   ref [7] */
#define MU_CMP   3.986004418E14   /* earth gravitational constant   ref [9] */
#define J2_GLO   1.0826257E-3     /* 2nd zonal harmonic of geopot   ref [2] */

#define OMGE_GLO 7.292115E-5      /* earth angular velocity (rad/s) ref [2] */
#define OMGE_GAL 7.2921151467E-5  /* earth angular velocity (rad/s) ref [7] */
#define OMGE_CMP 7.292115E-5      /* earth angular velocity (rad/s) ref [9] */

#define SIN_5 -0.0871557427476582 /* sin(-5.0 deg) */
#define COS_5  0.9961946980917456 /* cos(-5.0 deg) */

#define ERREPH_GLO 5.0            /* error of glonass ephemeris (m) */
#define TSTEP   60.0// 240.0//60.0// -->120.0       /* integration step glonass ephemeris (s) */
#define RTOL_KEPLER 1E-14         /* relative tolerance for Kepler equation */

#define DEFURASSR 0.15            /* default accurary of ssr corr (m) */
#define MAXECORSSR 10.0           /* max orbit correction of ssr (m) */
#define MAXCCORSSR (1E-6*CLIGHT)  /* max clock correction of ssr (m) */
#define MAXAGESSR 90.0            /* max age of ssr orbit and clock (s) */
#define MAXAGESSR_HRCLK 10.0      /* max age of ssr high-rate clock (s) */
#define MAXAGEB2B 300.0           /* max age of B2b ssr orbit and clock (s) */
#define STD_BRDCCLK 30.0          /* error of broadcast clock (m) */
#define POLYORDER 11              //(order=POLYORDER-1)
#define POLYGAPUNIT 3000.0
#define MCSNRLMT 38
#define DMOD(x,y)    ((x)-(int)((double)(x)/(double)(y))*(y))        //mod for double precision
#define SEC_IN_WEEK 604800
#define SEC_IN_HLAF_WEEK 302400


/**********************************************************************//**
@brief convert glonass time(leapyear/daynum/sod) to VERIQC internal time struct

@param LeapYears       [In]  four-year interval number starting from 1996, based on 1
@param DayNum          [In]  Days number of year, based on 1
@param SecOfDay        [In]  second of day

@return QC time
@author CHC
@date 2023/11/08

@note
History:
**************************************************************************/
extern QC_TIME_T VeriQCGLOEphTime2Time(int LeapYears, int DayNum, double SecOfDay)
{
    QC_TIME_T time = GLOEphTime2Time(LeapYears, DayNum, SecOfDay);
    QC_TIME_T ctime;

    ctime.Time = time.Time;
    ctime.Sec = time.Sec;

    return ctime;
}

/**********************************************************************//**
@brief convert gps/gal/bds/qzs time(week/sow) to VERIQC internal time struct

@param Sys           [In]  system
@param Week          [In]  week number of input system
@param SOW           [In]  second of week of input system

@return VERIQC time
@author CHC
@date 2023/11/08

@note
History:
**************************************************************************/
extern QC_TIME_T VeriQCUNIEphTime2Time(int Sys, int Week, double SOW)
{
    QC_TIME_T time = UNIEphTime2Time(Sys, Week, SOW);
    QC_TIME_T ctime;

    ctime.Time = time.Time;
    ctime.Sec = time.Sec;

    return ctime;
}

/* variance by ura ephemeris (ref [1] 20.3.3.3.1.1) --------------------------*/
static double VarURAEph(int URA)
{
    const double ura_value[]={   
        2.4,3.4,4.85,6.85,9.65,13.65,24.0,48.0,96.0,192.0,384.0,768.0,1536.0,
        3072.0,6144.0
    };
    return URA<0||14<URA?6144.0:SQR(ura_value[URA]);
}

/* broadcast ephemeris to satellite clock bias ---------------------------------
* compute satellite clock bias with broadcast ephemeris (gps, galileo, qzss)
* args   : gtime_t Time     I   time by satellite clock (gpst)
*          eph_t *pEph       I   broadcast ephemeris
* return : satellite clock bias (s) without relativeity correction
* notes  : see ref [1],[7],[8]
*          satellite clock does not include relativity correction and tdg
*-----------------------------------------------------------------------------*/
extern double Eph2Clk(QC_TIME_T Time, const CHC_EPH_T *pEph)
{
    double t;
    int i;
    
    t=QCTimeDiff(&Time, &pEph->Toc);
    
    for (i=0;i<2;i++) {
        t-=pEph->Af0+pEph->Af1*t+pEph->Af2*t*t;
    }
    return pEph->Af0+pEph->Af1*t+pEph->Af2*t*t;
}
static void GetEphConstantValue(const CHC_EPH_T *pEph,CHC_SATIPTINFO_T *pSatIptInfo)
{
	int sys,prn=0;
	double mu,omge;
    switch ((sys=SatSys(pEph->Sat,&prn))) {
        case SYS_GAL: mu=MU_GAL; omge=OMGE_GAL; break;
        case SYS_BD3:
        case SYS_CMP: mu=MU_CMP; omge=OMGE_CMP; break;
        default:      mu=MU_GPS; omge=OMGE;     break;
    }
	pSatIptInfo->Sat = pEph->Sat;
	pSatIptInfo->NDot = pEph->NDot;
	pSatIptInfo->Mp=sqrt(mu/(pEph->A*pEph->A*pEph->A))+pEph->DeltaN;
	pSatIptInfo->OmgeToeSec=omge*pEph->ToeSec;
	pSatIptInfo->Se2=sqrt(1.0-pEph->E*pEph->E);
	pSatIptInfo->RelCorrConst=2.0*sqrt(mu*pEph->A)*pEph->E/SQR(CLIGHT);//relativity correction constant
	pSatIptInfo->Var=VarURAEph(pEph->SVA);
}
static void GEphClkPar(QC_TIME_T Time,const CHC_SATIPTINFO_T *pSatIptInfo,double *pDts)
{
	double t;

    t=QCTimeDiff(&Time, &pSatIptInfo->Toe);

	*pDts=-pSatIptInfo->Af0+pSatIptInfo->Af1*t;
}
//clk/relativity correction
static void EphClkRelPar(QC_TIME_T Time,int Sys,const CHC_SATIPTINFO_T *pSatIptInfo,double *pDts,double *pRelCorr)
{
	double tk,mu,omge;
	double M,E,sinE,Ek;
	int MAXITENUM = 10,iter=0,n;

    tk=QCTimeDiff(&Time, &pSatIptInfo->Toe);

    switch (Sys) {
        case SYS_GAL: mu=MU_GAL; omge=OMGE_GAL; break;
        case SYS_BD3:
        case SYS_CMP: mu=MU_CMP; omge=OMGE_CMP; break;
        default:      mu=MU_GPS; omge=OMGE;     break;
    }
	M = pSatIptInfo->M0 + (pSatIptInfo->Mp + 0.5*pSatIptInfo->NDot*tk)*tk;
    
    for (n=0,E=M,Ek=0.0;fabs(E-Ek)>RTOL_KEPLER;n++) {
        Ek=E; E-=(E-pSatIptInfo->E*sin(E)-M)/(1.0-pSatIptInfo->E*cos(E));
		iter++;
		if(iter>MAXITENUM)
		{
		   break;/**< modify 20160601 */
		}
    }
    sinE=sin(E);

	tk=QCTimeDiff(&Time, &pSatIptInfo->Toc);
    *pDts=pSatIptInfo->Af0+pSatIptInfo->Af1*tk+pSatIptInfo->Af2*tk*tk;
	if(pRelCorr)
	{
		/* relativity correction */
		*pRelCorr=pSatIptInfo->RelCorrConst*sinE;
		*pDts-=*pRelCorr;	    
	}
}
static void Eph2PosPar(QC_TIME_T Time, const CHC_EPH_T *pEph, double *pRs, double *pDts,
                    double *pVar,double *pRelCorr,const CHC_SATIPTINFO_T *pSatIptInfo)
{
    double tk,M,E,Ek,sinE,cosE,u,r,i,O,sin2u,cos2u,x,y,sinO,cosO,cosi,mu,omge;
    double xg,yg,zg,sino,coso,Ak;
    int n,sys,prn;
    int MAXITENUM = 10,iter=0;
	double omgetk,ycosi,ygCOS5,zgSIN5;

    if (pEph->A <= 0.0) {
        pRs[0] = pRs[1] = pRs[2] = 0.0;
        if (pDts) { *pDts = 0.0; }
        if (pVar) { *pVar = 0.0; }
        return;
    }
    tk=QCTimeDiff(&Time, &pEph->Toe);

    switch ((sys=SatSys(pEph->Sat,&prn))) {
        case SYS_GAL: mu=MU_GAL; omge=OMGE_GAL; break;
        case SYS_BD3:
        case SYS_CMP: mu=MU_CMP; omge=OMGE_CMP; break;
        default:      mu=MU_GPS; omge=OMGE;     break;
    }
	Ak = pEph->A + pEph->ADot*tk;
	M  = pEph->M0 + (pSatIptInfo->Mp + 0.5*pEph->NDot*tk)*tk;
    
    for (n=0,E=M,Ek=0.0;fabs(E-Ek)>RTOL_KEPLER;n++) {
        Ek=E; E-=(E-pEph->E*sin(E)-M)/(1.0-pEph->E*cos(E));
		iter++;
		if(iter>MAXITENUM)
		{
		   break;/**< modify 20160601 */
		}
    }
    sinE=sin(E); cosE=cos(E);
    
    u=atan2(pSatIptInfo->Se2*sinE,cosE-pEph->E)+pEph->OMG;
    r=Ak*(1.0-pEph->E*cosE);
    i=pEph->I0+pEph->IDot*tk;
    sin2u=sin(2.0*u); cos2u=cos(2.0*u);
    u+=pEph->Cus*sin2u+pEph->Cuc*cos2u;
    r+=pEph->Crs*sin2u+pEph->Crc*cos2u;
    i+=pEph->Cis*sin2u+pEph->Cic*cos2u;
    x=r*cos(u); y=r*sin(u); cosi=cos(i);
    
    /* beidou geo satellite (ref [9]) */
    if ((sys==SYS_CMP||sys==SYS_BD3)&&(prn<=5||prn>=59)) {
        O=pEph->OMG0+pEph->OMGd*tk-pSatIptInfo->OmgeToeSec;//the toes must be itself system time
        sinO=sin(O); cosO=cos(O);
		ycosi=y*cosi;
        xg=x*cosO-ycosi*sinO;
        yg=x*sinO+ycosi*cosO;
        zg=y*sin(i);
		omgetk=omge*tk;
        sino=sin(omgetk); coso=cos(omgetk);
		ygCOS5=yg*COS_5;
		zgSIN5=zg*SIN_5;
        pRs[0]= xg*coso+ygCOS5*sino+zgSIN5*sino;
        pRs[1]=-xg*sino+ygCOS5*coso+zgSIN5*coso;
        pRs[2]=-yg*SIN_5+zg*COS_5;
    }
    else {
        O=pEph->OMG0+(pEph->OMGd-omge)*tk-pSatIptInfo->OmgeToeSec;//the toes must be itself system time
        sinO=sin(O); cosO=cos(O);
		ycosi=y*cosi;
        pRs[0]=x*cosO-ycosi*sinO;
        pRs[1]=x*sinO+ycosi*cosO;
        pRs[2]=y*sin(i);
    }
	
	if(pDts)
	{
	  tk=QCTimeDiff(&Time, &pEph->Toc);
      *pDts=pEph->Af0+pEph->Af1*tk+pEph->Af2*tk*tk;
	  if(pRelCorr)
	  {
		/* relativity correction */
		*pRelCorr=pSatIptInfo->RelCorrConst*sinE;
		*pDts-=*pRelCorr;	    
	  }
	}

    /* position and clock error variance */
	if(pVar)
	{
	   *pVar=pSatIptInfo->Var;
	}   
}

/* broadcast ephemeris to satellite position and clock bias --------------------
* compute satellite position and clock bias with broadcast ephemeris (gps,
* galileo, qzss)
* args   : gtime_t Time     I   time (gpst)
*          eph_t *pEph       I   broadcast ephemeris
*          double *pRs       O   satellite position (ecef) {x,y,z} (m)
*          double *pDts      O   satellite clock bias (s)
*          double *pVar      O   satellite position and clock variance (m^2)
* return : none
* notes  : see ref [1],[7],[8]
*          satellite clock includes relativity correction without code bias
*          (tgd or bgd)
*-----------------------------------------------------------------------------*/
extern void Eph2Pos(QC_TIME_T Time, const CHC_EPH_T *pEph, double *pRs, double *pDts,
                    double *pVar,double *pRelCorr)
{
    double tk,M,E,Ek,sinE,cosE,u,r,i,O,sin2u,cos2u,x,y,sinO,cosO,cosi,mu,omge;
    double xg,yg,zg,sino,coso,Ak,delnk;
    int n,sys,prn;
    int MAXITENUM = 10,iter=0;

    if (pEph->A<=0.0) {
        pRs[0]=pRs[1]=pRs[2]=*pDts=*pVar=0.0;
        return;
    }
    tk=QCTimeDiff(&Time, &pEph->Toe);
    
    switch ((sys=SatSys(pEph->Sat,&prn))) {
        case SYS_GAL: mu=MU_GAL; omge=OMGE_GAL; break;
        case SYS_BD3:
        case SYS_CMP: mu=MU_CMP; omge=OMGE_CMP; break;
        default:      mu=MU_GPS; omge=OMGE;     break;
    }
	Ak = pEph->A + pEph->ADot*tk;
	delnk = pEph->DeltaN + 0.5*pEph->NDot*tk;
	M = pEph->M0 + (sqrt(mu / (pEph->A*pEph->A*pEph->A)) + delnk)*tk;
    
    for (n=0,E=M,Ek=0.0;fabs(E-Ek)>RTOL_KEPLER;n++) {
        Ek=E; E-=(E-pEph->E*sin(E)-M)/(1.0-pEph->E*cos(E));
		iter++;
		if(iter>MAXITENUM)
		{
		   break;/**< modify 20160601 */
		}
    }
    sinE=sin(E); cosE=cos(E);
    
    u=atan2(sqrt(1.0-pEph->E*pEph->E)*sinE,cosE-pEph->E)+pEph->OMG;
    r=Ak*(1.0-pEph->E*cosE);
    i=pEph->I0+pEph->IDot*tk;
    sin2u=sin(2.0*u); cos2u=cos(2.0*u);
    u+=pEph->Cus*sin2u+pEph->Cuc*cos2u;
    r+=pEph->Crs*sin2u+pEph->Crc*cos2u;
    i+=pEph->Cis*sin2u+pEph->Cic*cos2u;
    x=r*cos(u); y=r*sin(u); cosi=cos(i);
    
    /* beidou geo satellite (ref [9]) */
    if ((sys==SYS_CMP||sys==SYS_BD3)&&(prn<=5||prn>=59)) {
        O=pEph->OMG0+pEph->OMGd*tk-omge*pEph->ToeSec;//the toes must be itself system time
        sinO=sin(O); cosO=cos(O);
        xg=x*cosO-y*cosi*sinO;
        yg=x*sinO+y*cosi*cosO;
        zg=y*sin(i);
        sino=sin(omge*tk); coso=cos(omge*tk);
        pRs[0]= xg*coso+yg*sino*COS_5+zg*sino*SIN_5;
        pRs[1]=-xg*sino+yg*coso*COS_5+zg*coso*SIN_5;
        pRs[2]=-yg*SIN_5+zg*COS_5;
    }
    else {
        O=pEph->OMG0+(pEph->OMGd-omge)*tk-omge*pEph->ToeSec;//the toes must be itself system time
        sinO=sin(O); cosO=cos(O);
        pRs[0]=x*cosO-y*cosi*sinO;
        pRs[1]=x*sinO+y*cosi*cosO;
        pRs[2]=y*sin(i);
    }
	
	if(pDts)
	{
	  tk=QCTimeDiff(&Time, &pEph->Toc);
      *pDts=pEph->Af0+pEph->Af1*tk+pEph->Af2*tk*tk;
	  if(pRelCorr)
	  {
		/* relativity correction */
		*pRelCorr=2.0*sqrt(mu*pEph->A)*pEph->E*sinE/SQR(CLIGHT);
		*pDts-=*pRelCorr;	    
	  }
	}

    /* position and clock error variance */
	if(pVar)
	{
	   *pVar=VarURAEph(pEph->SVA);
	}   
}
/* glonass orbit differential equations --------------------------------------*/
static void Deq(const double *pX, double *pXDot, const double *pAcc)
{
    double a,b,c,r2=Dot(pX,pX,3),r3=r2*sqrt(r2),omg2=SQR(OMGE_GLO);
    
    if (r2<=0.0) {
        pXDot[0]=pXDot[1]=pXDot[2]=pXDot[3]=pXDot[4]=pXDot[5]=0.0;
        return;
    }
    /* ref [2] A.3.1.2 with bug fix for xdot[4],xdot[5] */
    a=1.5*J2_GLO*MU_GLO*SQR(RE_GLO)/r2/r3; /* 3/2*J2*mu*Ae^2/r^5 */
    b=5.0*pX[2]*pX[2]/r2;                    /* 5*z^2/r^2 */
    c=-MU_GLO/r3-a*(1.0-b);                /* -mu/r^3-a(1-b) */
    pXDot[0]=pX[3]; pXDot[1]=pX[4]; pXDot[2]=pX[5];
    pXDot[3]=(c+omg2)*pX[0]+2.0*OMGE_GLO*pX[4]+pAcc[0];
    pXDot[4]=(c+omg2)*pX[1]-2.0*OMGE_GLO*pX[3]+pAcc[1];
    pXDot[5]=(c-2.0*a)*pX[2]+pAcc[2];
}
/* glonass position and velocity by numerical integration --------------------*/
static void GLOOrbit(double Time, double *pX, const double *pAcc)
{
    double k1[6],k2[6],k3[6],k4[6],w[6];
    int i;
    
    Deq(pX,k1,pAcc); for (i=0;i<6;i++) w[i]=pX[i]+k1[i]*Time/2.0;
    Deq(w,k2,pAcc); for (i=0;i<6;i++) w[i]=pX[i]+k2[i]*Time/2.0;
    Deq(w,k3,pAcc); for (i=0;i<6;i++) w[i]=pX[i]+k3[i]*Time;
    Deq(w,k4,pAcc);
    for (i=0;i<6;i++) pX[i]+=(k1[i]+2.0*k2[i]+2.0*k3[i]+k4[i])*Time/6.0;
}

/* glonass ephemeris to satellite clock bias -----------------------------------
* compute satellite clock bias with glonass ephemeris
* args   : gtime_t Time     I   time by satellite clock (gpst)
*          geph_t *pGEph     I   glonass ephemeris
* return : satellite clock bias (s)
* notes  : see ref [2]
*-----------------------------------------------------------------------------*/
extern double GEph2Clk(QC_TIME_T Time, const CHC_GEPH_T *pGEph)
{
    double t;
    int i;
    
    t=QCTimeDiff(&Time, &pGEph->Toe);
    
    for (i=0;i<2;i++) {
        t-=-pGEph->Taun+pGEph->Gamn*t;
    }
    return -pGEph->Taun+pGEph->Gamn*t;
}
extern void GEph2PosOp(QC_TIME_T Time, const CHC_GEPH_T *GEph, double *pRs,double *pX,QC_TIME_T *pXTime)
{
    double t,tt;
    int i;
    double tadd=0.0;
    int MAXINTEGRAL=1200;
    double deltaT;

    t=QCTimeDiff(&Time, pXTime);
    deltaT = QCTimeDiff(&Time, &GEph->Toe);

    i=0;
    for (tt=t<0.0?-TSTEP:TSTEP;fabs(t)>1E-9;t-=tt) {
        if (fabs(t)<TSTEP) tt=t;
        GLOOrbit(tt,pX,GEph->Acc);
        tadd+=tt;
        i++;
        if(i>MAXINTEGRAL)
        {
            break;
        }
    }
    *pXTime=QCTimeAdd(pXTime,tadd);//x->outtime

    for (i=0;i<3;i++) pRs[i]=pX[i];

}
/* glonass ephemeris to satellite position and clock bias ----------------------
* compute satellite position and clock bias with glonass ephemeris
* args   : gtime_t Time      I   time (gpst)
*          geph_t *pGEph     I   glonass ephemeris
*          double *pRs       O   satellite position {x,y,z} (ecef) (m)
*          double *pDts      O   satellite clock bias (s)
*          double *pVar      O   satellite position and clock variance (m^2)
* return : none
* notes  : see ref [2]
*-----------------------------------------------------------------------------*/
extern void GEph2Pos(QC_TIME_T Time, const CHC_GEPH_T *pGEph, double *pRs, double *pDts,
                     double *pVar)
{
    double t,tt,x[6];
    int i;
    int MAXINTEGRAL=1200;
    double deltaT;

    t=QCTimeDiff(&Time,&pGEph->Toe);
    deltaT = t;

    if(pDts)
	{
	  *pDts=-pGEph->Taun+pGEph->Gamn*t;
	}
	
	if(pRs)
	{
		for (i=0;i<3;i++) {
			x[i  ]=pGEph->Pos[i];
			x[i+3]=pGEph->Vel[i];
		}
		i=0;
		for (tt=t<0.0?-TSTEP:TSTEP;fabs(t)>1E-9;t-=tt) {
			if (fabs(t)<TSTEP) tt=t;
			GLOOrbit(tt,x,pGEph->Acc);
			i++;
			if(i>MAXINTEGRAL)
			{
			   break;
			}
		}
		for (i=0;i<3;i++) pRs[i]=x[i];
	}

	if(pVar)
	{
	  *pVar=SQR(ERREPH_GLO);
	}
}

static double GetTimeMax(int Sys)
{
   double tmax=MAXDTOE+600.0;
   if(Sys==SYS_CMP||Sys==SYS_BD3)
   {
      tmax=MAXDTOE_BDS+600.0;
   }
   else if(Sys==SYS_GLO)
   {
      tmax=MAXDTOE_GLO+600.0;
   }
   return tmax;
}
static int ChkEphValid(CHC_EPH_T *pEph)
{
    double toe, A, E, xi, xnode, per, xmo, odot;
    int iweek = 0, prn = 0;
    int sys;
    QC_TIME_T et;

    et = pEph->Toe;
    sys = SatSys(pEph->Sat, &prn);
    if (sys == SYS_CMP || sys == SYS_BD3)
    {
        et = QCTimeAdd(&et, -14.0);
    }

    toe = QCTime2GPSTime(et, &iweek);
    if (iweek <= 0)
    {
        GNSS_LOG_TRACE("iweek<=0");
        return 0;
    }
 
    toe = ROUND_D((toe*1E6) / 1E6);
    A = pEph->A;
    E = pEph->E;
    xi = pEph->I0*R2D;
    xnode = pEph->OMG0*R2D;
    per = pEph->OMG*R2D;
    xmo = pEph->M0*R2D;

    odot = pEph->OMGd*R2D;

    if (sys == SYS_GPS && pEph->Sat <= MAXPRNGPS_TRUE) //GPS
    {
        if ((toe < 0.0) || (toe > 7 * 86400.0))
        {
            GNSS_LOG_TRACE("toe=%f", toe);
            return 0;
        }
        if (A<26.0E6 || A>27.0E6)
        {
            GNSS_LOG_TRACE("A=%f", A);
            return 0;
        }
        if (E<0.0 || E>0.1)
        {
            GNSS_LOG_TRACE("E%f", E);
            return 0;
        }
        if ((xi > 65.0 || xi<60.0) && (xi>58.0 || xi < 50))
        {
            GNSS_LOG_TRACE("xi=%f", xi);
            return 0;
        }
        if (fabs(xnode) < 1.0E-6)
        {
            GNSS_LOG_TRACE("xnode=%f", xnode);
            return 0;
        }
        if (fabs(per) < 1.0E-6)
        {
            GNSS_LOG_TRACE("per=%f", per);
            return 0;
        }
        if (fabs(xmo) < 1.0E-6)
        {
            GNSS_LOG_TRACE("xmo=%f", xmo);
            return 0;
        }
    }
    else if (sys == SYS_GAL) //GAL
    {
        if (toe < 0.0 || toe>7 * 86400.0 ||
            (fabs(DMOD(toe, 100.0) - 3.0) > 1.0E-5 && DMOD(toe + 1.0E-5, 100.0) > 2.0E-5))
        {
            GNSS_LOG_TRACE("toe=%f", toe);
            return 0;
        }
        if (A<29.0E6 || A>32.0E6) //
        {
            GNSS_LOG_TRACE("A=%f", A);
            return 0;
        }
        if (E<0.0 || E>0.1)
        {
            return 0;
        }
        if ((xi > 65.0 || xi<60.0) && (xi>58.0 || xi < 50))
        {
            return 0;
        }
        if (fabs(xnode) < 1.0E-6)
        {
            return 0;
        }
        if (fabs(xmo) < 1.0E-6)
        {
            return 0;
        }
    }
    else if (sys == SYS_CMP || sys == SYS_BD3) //BDS
    {
        if (toe < 0.0 || toe>7 * 86400.0 ||
            (fabs(DMOD(toe, 100.0) - 3.0) > 1.0E-5 && DMOD(toe + 1.0E-5, 100.0) > 2.0E-5))
        {
            return 0;
        }
        /* no more check for GEO */
        if (prn <= 5 || prn >= 59)
        {
            return 1;
        }

        if ((A<41.0E6 || A>43.0E6) && (A<27.0E6 || A>28.5E6)) /* two kinds of orbits */
        {
            return 0;
        }
        if (E<0.0 || E>0.1)
        {
            return 0;
        }
        if ((xi > 86.0) || (xi < 45))
        {
            return 0;
        }
        if (fabs(xnode) < 1.0E-6)
        {
            return 0;
        }
        if (fabs(per) < 1.0E-6)
        {
            return 0;
        }
        if (fabs(xmo) < 1.0E-6)
        {
            return 0;
        }
        if (odot<-6.0E-7 || odot>0) //
        {
            return 0;
        }
    }
    else if (sys == SYS_QZS) //QZSS
    {
        if (toe < 0.0 || toe>7 * 86400.0 ||
            (fabs(DMOD(toe, 100.0) - 3.0) > 1.0E-5 && DMOD(toe + 1.0E-5, 100.0) > 2.0E-5))
        {
            return 0;
        }
        if (A<41.0E6 || A>43.0E6)
        {
            return 0;
        }
        if (E<0.0 || E>0.1)
        {
            return 0;
        }
        if (xi>50.0 || xi < 0) //
        {
            return 0;
        }
        if (fabs(xnode) < 1.0E-6)
        {
            return 0;
        }
        if (fabs(per) < 1.0E-6)
        {
            return 0;
        }
        if (fabs(xmo) < 1.0E-6)
        {
            return 0;
        }
    }
    else
    {
        return 1;
    }

    return 1;
}
/* select ephememeris --------------------------------------------------------*/
static CHC_EPH_T *SelEph(QC_TIME_T Time, int Sat, int IODE, const CHC_NAV_T *pNav)
{
    double t,tmax,tmin,tt;
    int i,j=-1,sys=0,prn=0, exstat;
	sys=SatSys(Sat,&prn);
	tmax=GetTimeMax(sys);
	
    tmin=tmax+1.0;

    for (i=0;i<pNav->Num;i++) {
        if (pNav->pEph[i].Sat!=Sat) continue;
       
        if (IODE >= 0 && pNav->pEph[i].IODE != IODE)
        {
            GNSS_LOG_TRACE("sat=%3d, time=%f, i=%3d, iode=%f, nav->iode=%d",
                Sat, Time.Time + Time.Sec, i, IODE, pNav->pEph[i].IODE);
            continue;
        }
	
        if ((t = fabs(QCTimeDiff(&pNav->pEph[i].Toe, &Time)))>tmax) {
            continue;
        }
        if (IODE>=0) return pNav->pEph+i;
        if (t<=tmin) {j=i; tmin=t;} /* toe closest to time */
    }
    if (IODE>=0||j<0) {
        return NULL;
    }
    
	/** add 20170305 */
	tt=QCTimeDiff(&pNav->pEph[j].Toe, &pNav->pEph[j].Toc);
	if(fabs(tt)>1800.0)
	{
        GNSS_LOG_TRACE("j=%d, tt=%f > 1800.0", j, tt);
	   return NULL;
	}
    /** add 20200827 */
    exstat = ChkEphValid(pNav->pEph + j);
    if (exstat == 0)
    {
        GNSS_LOG_TRACE("sat=%d, ephvalid failed", Sat);
        return NULL;
    }
    return pNav->pEph+j;
}
/* select ephememeris --------------------------------------------------------*/
static CHC_EPH_T *SelEphBD3(QC_TIME_T Time, int Sat, int IODE, const CHC_NAV_T *pNav)
{
	double t, tmax, tmin, tt;
	int i, j = -1, sys = 0, prn = 0, exstat;
	sys = SatSys(Sat, &prn);
	tmax = GetTimeMax(sys);

	tmin = tmax + 1.0;

	for (i = 0; i<pNav->NumBD3; i++) {
		if (pNav->pEphBD3[i].Sat != Sat) continue;

		if (IODE >= 0 && pNav->pEphBD3[i].IODE != IODE) continue;

		if ((t = fabs(QCTimeDiff(&pNav->pEphBD3[i].Toe, &Time)))>tmax) {
			continue;
		}
		if (IODE >= 0) return pNav->pEphBD3 + i;
		if (t <= tmin) { j = i; tmin = t; } /* toe closest to time */
	}
	if (IODE >= 0 || j<0) {
		return NULL;
	}

	/** add 20170305 */
	tt = QCTimeDiff(&pNav->pEphBD3[j].Toe, &pNav->pEphBD3[j].Toc);
	if (fabs(tt)>1800.0)
	{
		return NULL;
	}
    /** add 20200827 */
	exstat = ChkEphValid(pNav->pEphBD3 + j);
	if (exstat == 0)
	{
		return NULL;
	}
	return pNav->pEphBD3 + j;
}
/* select glonass ephememeris ------------------------------------------------*/
static CHC_GEPH_T *SelGEph(QC_TIME_T Time, int Sat, int IODE, const CHC_NAV_T *pNav)
{
    double t,tmax,tmin;
    int i,j=-1;
    
	tmax=GetTimeMax(SYS_GLO);
	tmin=tmax+1.0;

    for (i=0;i<pNav->NumGLO;i++) {
        if (pNav->pGEph[i].Sat!=Sat) continue;
        if (IODE>=0&&pNav->pGEph[i].IODE!=IODE) continue;
        if ((t=fabs(QCTimeDiff(&pNav->pGEph[i].Toe, &Time)))>tmax) continue;
        if (IODE>=0) return pNav->pGEph+i;
        if (t<=tmin) {j=i; tmin=t;} /* toe closest to time */
    }
    if (IODE>=0||j<0) {
    	return NULL;
    }

    return pNav->pGEph+j;
}

/* satellite clock with broadcast ephemeris ----------------------------------*/
static int EphClk(QC_TIME_T Time, QC_TIME_T TimeEph, int Sat, const CHC_NAV_T* pNav,
    double* pDts, int BD3Flag, CHC_SATIPTINFO_T* pSatIptInfo)
{
    CHC_EPH_T* eph;
    CHC_GEPH_T* geph;
    //seph_t *seph;
    int sys, i;
    double tst, ted, toctt;
    U1 caninter = 0;

    sys = SatSys(Sat, NULL);

    if (pSatIptInfo)
    {
        if (pSatIptInfo->ValidFlag)
        {
            pSatIptInfo->XSUsing = 1;
            caninter = 1;
            tst = QCTimeDiff(&Time, &pSatIptInfo->pFtPNTTime[0]);
            ted = QCTimeDiff(&Time, &pSatIptInfo->pFtPNTTime[pSatIptInfo->ObsNum - 1]);
            if (tst * ted > 0.0)
            {
                caninter = 0;
                pSatIptInfo->XSUsing = 0;
            }
            else
            {
                LOGI("tst*ted<=0, sat=%d\n", Sat);
            }
        }

        if (caninter)
        {
            if (sys == SYS_GPS || sys == SYS_GAL || sys == SYS_CMP || sys == SYS_BD3)
            {
                toctt = QCTimeDiff(&Time, &pSatIptInfo->Toc);
                for (i = 0; i < 2; i++) {
                    toctt -= pSatIptInfo->Af0 + pSatIptInfo->Af1 * toctt + pSatIptInfo->Af2 * toctt * toctt;
                }
                *pDts = pSatIptInfo->Af0 + pSatIptInfo->Af1 * toctt + pSatIptInfo->Af2 * toctt * toctt;
            }
            else
            {
                toctt = QCTimeDiff(&Time, &pSatIptInfo->Toe);
                for (i = 0; i < 2; i++) {
                    toctt -= -pSatIptInfo->Af0 + pSatIptInfo->Af1 * toctt;
                }
                *pDts = -pSatIptInfo->Af0 + pSatIptInfo->Af1 * toctt;
            }
            pSatIptInfo->XSUsing = 0;
            return 1;
        }
        else
        {
            LOGI("caninter fail, sat=%d\n", Sat);
            return 0;
        }
    }

    if (sys==SYS_GPS||sys==SYS_GAL||sys==SYS_QZS||sys==SYS_CMP||sys==SYS_BD3) {
		if (BD3Flag)
		{
			if (!(eph = SelEphBD3(TimeEph, Sat, -1, pNav))) {

				return 0;
			}
		}
		else
		{
			if (!(eph = SelEph(TimeEph, Sat, -1, pNav))) {
                LOGI("seleph fail, sat=%d\n", Sat);
				return 0;
			}	
		}
		*pDts = Eph2Clk(Time, eph);
    }
    else if (sys==SYS_GLO) {
        if (!(geph=SelGEph(TimeEph,Sat,-1,pNav))) return 0;
        *pDts=GEph2Clk(Time,geph);
    }
    else return 0;
  
    return 1;
}

static double GetSatPosByPolyXs(const double X,double *pXs,int N)
{
	int i;
	double y=0.0;
	double t=1.0;
	double xt;
	xt = X/POLYGAPUNIT;
	for(i=0;i<N;i++)
	{
	   y+=t*pXs[i];
	   t*=xt;
	}

	return y;
}
static U1 ComputeInterXs(const double *pX,const double *pY,double *pXs,int NumPar,int NumObs)
{
	double *H=NULL,t,xt;
	int i,j,k,info;
	U1 stat=0;
	if(NumObs<NumPar) return 0;
	H=Mat(NumPar,NumObs);

	for(i=0;i<NumObs;i++)
	{
		k=i*NumPar;
		t=1.0;
		xt=pX[i]/POLYGAPUNIT;
	    for(j=0;j<NumPar;j++)
	    {
	      H[k+j]=t;
		  t*=xt;
	    }
	}
	info=Solve(MAT_MUL_TYPE_TN,H,pY,NumObs,1,pXs); /* F=Z'\E */
	stat=info==0?1:0;

	if(H){VERIQC_FREE(H);H=NULL;}
	
	return stat;
}


/*
gps/bds/gal poly info
return:1,success compute param;
soltype:0:forward,1:backward
time:curtime/starttime
*/
extern int GetEphPolyInfo(int SolType, QC_TIME_T Time,const CHC_EPH_T *pEph,CHC_SATIPTINFO_T *pSatIptInfo)
{
	int i,k,sys,mid;
	double tmax,tgap;
	QC_TIME_T ctime={0};
	double xt[POLYORDER],p[3][POLYORDER];//POLYORDER->nobs
	U1 stat=1;

    if (pSatIptInfo->XSUsing)
    {
        return 0;
    }
	
    pSatIptInfo->ValidFlag=0;

	sys=SatSys(pEph->Sat,NULL);

	tmax=GetTimeMax(sys);

    tgap = floor(tmax / (pSatIptInfo->ObsNum - 1));

	if(SolType==1)
	{
	   tgap=-tgap;//backward
	}

	GetEphConstantValue(pEph,pSatIptInfo);

	for(i=0;i<pSatIptInfo->ObsNum;i++)
	{
		ctime=QCTimeAdd(&Time,i*tgap);
		pSatIptInfo->pFtPNTTime[i]=ctime;
		Eph2PosPar(ctime,pEph,pSatIptInfo->pRs+i*3,NULL,NULL,NULL,pSatIptInfo);
	}   

   //poly
    mid = (pSatIptInfo->ObsNum - 1) / 2;
    for (i = 0; i < pSatIptInfo->ObsNum; i++)
    {
        xt[i] = QCTimeDiff(&pSatIptInfo->pFtPNTTime[i], &pSatIptInfo->pFtPNTTime[mid]);
		for(k=0;k<3;k++) p[k][i]=pSatIptInfo->pRs[i*3+k];
	}
	for(k=0;k<3;k++)
	{
		stat&=ComputeInterXs(xt,p[k],pSatIptInfo->pXs+k*pSatIptInfo->ParaNum,(int)pSatIptInfo->ParaNum,(int)pSatIptInfo->ObsNum);
	}
	if(stat)
	{
		pSatIptInfo->ValidFlag=1;
		pSatIptInfo->IODE=pEph->IODE;
		pSatIptInfo->IODC=pEph->IODC;
		pSatIptInfo->Toe=pEph->Toe;
		pSatIptInfo->Toc=pEph->Toc;
        pSatIptInfo->SVH = pEph->SVH == 0 ? 0 : 1;
		pSatIptInfo->M0=pEph->M0;
		pSatIptInfo->E=pEph->E;
		pSatIptInfo->Af0=pEph->Af0;
		pSatIptInfo->Af1=pEph->Af1;
		pSatIptInfo->Af2=pEph->Af2;
		pSatIptInfo->NDot = pEph->NDot;
		return 1;
	}
	return 0;
}
/*
glonass eph poly info
return:1,success compute param;0,error
soltype:0:forward,1:backward
time:curtime/starttime
*/
extern int GetGEphPolyInfo(int SolType, QC_TIME_T Time, const CHC_GEPH_T *pGEph, CHC_SATIPTINFO_T *pSatIptInfo)
{
	int i, k, sys, mid;
	double tmax, tgap, vare = 0.0, x[6] = { 0 };// , testrs[3] = { 0 };
	QC_TIME_T ctime = { 0 }, xtime = { 0 };
	double xt[POLYORDER], p[3][POLYORDER];//POLYORDER->nobs
	U1 stat = 1;

	pSatIptInfo->ValidFlag = 0;

	sys = SatSys(pGEph->Sat, NULL);

	tmax = GetTimeMax(sys);

	tgap = floor(tmax / (pSatIptInfo->ObsNum - 1));

	if (SolType == 1)
	{
		tgap = -tgap;//backward
	}

	for (k = 0; k<3; k++) {
		x[k] = pGEph->Pos[k];
		x[k + 3] = pGEph->Vel[k];
	}
	xtime = pGEph->Toe;
	for (i = 0; i<pSatIptInfo->ObsNum; i++)
	{
		ctime = QCTimeAdd(&Time, i*tgap);
		pSatIptInfo->pFtPNTTime[i] = ctime;
		GEph2PosOp(ctime, pGEph, pSatIptInfo->pRs + i * 3, x, &xtime);
	}

	//poly
    mid = (pSatIptInfo->ObsNum - 1) / 2;
    for (i = 0; i < pSatIptInfo->ObsNum; i++)
    {
        xt[i] = QCTimeDiff(&pSatIptInfo->pFtPNTTime[i], &pSatIptInfo->pFtPNTTime[mid]);
		for (k = 0; k<3; k++) p[k][i] = pSatIptInfo->pRs[i * 3 + k];
	}
	for (k = 0; k<3; k++)
	{
		stat &= ComputeInterXs(xt, p[k], pSatIptInfo->pXs + k*pSatIptInfo->ParaNum, (int)pSatIptInfo->ParaNum, (int)pSatIptInfo->ObsNum);
	}
	if (stat)
	{
		pSatIptInfo->ValidFlag = 1;
		pSatIptInfo->IODE = pGEph->IODE;
		pSatIptInfo->Toe = pGEph->Toe;
		GEph2Pos(ctime, pGEph, NULL, NULL, &vare);
		pSatIptInfo->Var = vare;
		pSatIptInfo->SVH = (U1)pGEph->SVH;
		pSatIptInfo->Af0 = pGEph->Taun;
		pSatIptInfo->Af1 = pGEph->Gamn;
		return 1;
	}
	return 0;
}

extern int SatInterpolationStat(int Sys, QC_TIME_T Time, const CHC_EPH_T *pEph, const CHC_GEPH_T *pGEph,const CHC_SATIPTINFO_T *pSatIptInfo)
{
	 int stat=0;
	 double ts,te;
	 double df0,df1,df2;
     if (Sys==SYS_GPS||Sys==SYS_GAL||Sys==SYS_CMP||Sys==SYS_BD3)
	 {
			if(pSatIptInfo->ValidFlag)
			{
				if(pSatIptInfo->IODE==pEph->IODE&&pSatIptInfo->SVH==pEph->SVH)
				{
					ts=QCTimeDiff(&pSatIptInfo->pFtPNTTime[0], &Time);
					te=QCTimeDiff(&pSatIptInfo->pFtPNTTime[pSatIptInfo->ObsNum-1], &Time);
					df0=fabs(pSatIptInfo->Af0-pEph->Af0);
					df1=fabs(pSatIptInfo->Af1-pEph->Af1);
					df2=fabs(pSatIptInfo->Af2-pEph->Af2);

                    if ((IsQCTimeSame(pSatIptInfo->Toe, pEph->Toe)) &&
                        (IsQCTimeSame(pSatIptInfo->Toc, pEph->Toc)) &&
                        (ts * te <= 0.0) &&
                        (df0 < 1E-12 && df1 < 1E-12 && df2 < 1E-12))
                    {
                        stat = 1;
                    }
				}
			}
	 }
	 else
	 {
			if(pSatIptInfo->ValidFlag)
			{
				if(pSatIptInfo->IODE==pGEph->IODE&&pSatIptInfo->SVH==pGEph->SVH)
				{
					ts=QCTimeDiff(&pSatIptInfo->pFtPNTTime[0], &Time);
					te=QCTimeDiff(&pSatIptInfo->pFtPNTTime[pSatIptInfo->ObsNum-1], &Time);
					df0=fabs(pSatIptInfo->Af0-pGEph->Taun);
					df1=fabs(pSatIptInfo->Af1-pGEph->Gamn);

                    if((IsQCTimeSame(pSatIptInfo->Toe, pGEph->Toe)) &&
                       (ts*te<=0.0) &&
					   (df0<1E-12 && df1<1E-12))
					{
						stat=1;
					}
				}
			}	 
	 }

	 return stat;
}

/*for update sat pos interpolation para
navsys:used navsys
soltype:0-forward;1-backward
*/
extern int UpdateInterSatPosInfo(int NavSys, int SolType, const CHC_OBSD_T *pObs, int Num,
    CHC_SATIPTINFO_T *pSatIptInfo, const CHC_NAV_T *pNav, int MaxCPUTimes, 
    unsigned char* pEphValidFlag, unsigned char *pEphActiveValidFlag, unsigned char* pAlmValidFlag)
{
    int i, sys,sat;
    CHC_EPH_T  *eph;
    CHC_GEPH_T *geph;
    QC_TIME_T teph, time;
    int parastat = 0,nmax;
    int computedns = 0, stat;
    unsigned  int stick, etick;
    int cputimes;

    if (Num<= 0 || !pSatIptInfo) return 0;

    teph = pObs[0].Time;/* select ephemeris/almanac by obs time */

    if (!SolType)
    {
        time = QCTimeAdd(&teph, -1.0*POLYSATPOSSHIFTSEC);//forward(consider transmit and clk bias time ,minus 1 second to contain current time)
    }
    else
    {
        time = QCTimeAdd(&teph, POLYSATPOSSHIFTSEC);//backward
    }
    stick = TickGet();

    nmax = MAXSAT;

    for (i = 0; i < nmax; i++)
    {
        etick = TickGet();
        cputimes = (int)etick - (int)stick;

        if (cputimes > MaxCPUTimes)
        {
            break;
        }

        sat = i + 1;
        sys = SatSys(sat, NULL);

        if (!(sys&NavSys)) continue;

        if (pSatIptInfo[sat - 1].Computing)  continue;//in computing
        if (pEphValidFlag) if (!pEphValidFlag[sat - 1]) continue;

        pSatIptInfo[sat - 1].Computing = 1;

        parastat = 0;
        if (sys == SYS_GPS || sys == SYS_GAL || sys == SYS_CMP || sys == SYS_BD3)
        {
            if (!(eph = SelEph(teph, sat, -1, pNav)))
            {
                pSatIptInfo[sat - 1].Computing = 0;
                if (pEphValidFlag) pEphValidFlag[sat - 1] = 0;
                if (pEphActiveValidFlag) pEphActiveValidFlag[sat - 1] = 1;
                continue;
            }
            parastat = SatInterpolationStat(sys, teph, eph, NULL, &pSatIptInfo[sat - 1]);

            if (!parastat)
            {
                stat = GetEphPolyInfo(SolType, time, eph, &pSatIptInfo[sat - 1]);//need to update para

                computedns += stat;
            }

            pSatIptInfo[sat - 1].Computing = 0;
            if (pEphValidFlag) pEphValidFlag[sat - 1] = 0;
            if (pEphActiveValidFlag) pEphActiveValidFlag[sat - 1] = 0;
        }
        else
        {
            if (!(geph = SelGEph(teph, sat, -1, pNav)))
            {
                pSatIptInfo[sat - 1].Computing = 0;
                if (pEphValidFlag) pEphValidFlag[sat - 1] = 0;
                if (pEphActiveValidFlag) pEphActiveValidFlag[sat - 1] = 1;
                continue;
            }
            parastat = SatInterpolationStat(sys, teph, NULL, geph, &pSatIptInfo[sat - 1]);

            if (!parastat)
            {
                stat = GetGEphPolyInfo(SolType, time, geph, &pSatIptInfo[sat - 1]);//need to update para
                computedns += stat;
            }

            pSatIptInfo[sat - 1].Computing = 0;
            if (pEphValidFlag) pEphValidFlag[sat - 1] = 0;
            if (pEphActiveValidFlag) pEphActiveValidFlag[sat - 1] = 0;
        }
    }

    return computedns;
}

extern int UpdateInterSatPosInfoBD3(int SolType, const CHC_OBSD_T *pObs, int Num,
	CHC_SATIPTINFO_T *pSatIptInfo, const CHC_NAV_T *pNav, int MaxCPUTimes,
    unsigned char* pEphValidFlag, unsigned char *pEphActiveValidFlag)
{
    int i, sat, prn = 0, sys;
	CHC_EPH_T  *eph;
	QC_TIME_T teph, time;
	int parastat = 0, nmax;
	int computedns = 0, stat;
	unsigned  int stick, etick;
	int cputimes;
    int ephIode;
    int iSat = CHC_NSATGPS + CHC_NSATGLO + CHC_NSATGAL + CHC_NSATQZS;

	if (Num<=0 || !pSatIptInfo) return 0;

    teph = pObs[0].Time;/* select ephemeris/almanac by obs time */

	if (!SolType)
	{
		time = QCTimeAdd(&teph, -1.0*POLYSATPOSSHIFTSEC);//forward(consider transmit and clk bias time ,minus 1 second to contain current time)
	}
	else
	{
		time = QCTimeAdd(&teph, POLYSATPOSSHIFTSEC);//backward
	}
	stick = TickGet();

	nmax = MAXSAT;

	for (i = iSat; i < iSat + CHC_NSATCMP; i++)
	{
		etick = TickGet();
		cputimes = (int)etick - (int)stick;

		if (cputimes > MaxCPUTimes)
		{
			break;
		}
        sat = i + 1; prn = i - iSat + 1;

        if (pSatIptInfo[prn - 1].Computing)  continue;//in computing

        pSatIptInfo[prn - 1].Computing = 1;
        ephIode = -1;
		parastat = 0;
  
        if (!(eph = SelEphBD3(teph, sat, ephIode, pNav)))
        {
            if (ephIode >= 0)
            {
                if (!pSatIptInfo[prn - 1].ValidFlag)
                {
                    eph = SelEphBD3(teph, sat, -1, pNav);
                }
            }
            if (!eph)
            {
                pSatIptInfo[prn - 1].Computing = 0;
                if (pEphValidFlag) pEphValidFlag[prn - 1] = 0;
                if (pEphActiveValidFlag) pEphActiveValidFlag[prn - 1] = 1;
                continue;
            }
        }

        sys = SatSys(sat, NULL);
        parastat = SatInterpolationStat(sys, teph, eph, NULL, &pSatIptInfo[prn - 1]);

        if (!parastat)
        {
            stat = GetEphPolyInfo(SolType, time, eph, &pSatIptInfo[prn - 1]);//need to update para

            computedns += stat;
        }

        pSatIptInfo[prn - 1].Computing = 0;
        if (pEphValidFlag) pEphValidFlag[prn - 1] = 0;
        if (pEphActiveValidFlag) pEphActiveValidFlag[prn - 1] = 0;
	}

	return computedns;
}


/* satellite position and clock by broadcast ephemeris -----------------------*/
static int EphPos(QC_TIME_T Time, QC_TIME_T EphTime, int Sat, const CHC_NAV_T *pNav,
                  int IODE, double *pRs, double *pDts, double *pRelCorr, double *pVar,
                  U1 *pSVH,CHC_SATIPTINFO_T *pSatIptInfo,int *pUsedIODE, int *pToeSec, int BD3Flag)
{
    CHC_EPH_T  *eph;
    CHC_GEPH_T *geph;
	QC_TIME_T timer={0};
    double toett;
	double tst,ted;
    double rst[3],dtst[1],tt=1E-3;
    int i,k,sys,mid;
    double relcorrt=0;
	U1 caninter=0;
    
    sys=SatSys(Sat,NULL);
    
    *pSVH=0;
    
	if(pSatIptInfo)
	{
		if(pSatIptInfo->ValidFlag)
		{
            pSatIptInfo->XSUsing = 1;
			caninter=1;
			tst=QCTimeDiff(&Time, &pSatIptInfo->pFtPNTTime[0]);
			ted=QCTimeDiff(&Time, &pSatIptInfo->pFtPNTTime[pSatIptInfo->ObsNum-1]);
			if(tst*ted>0.0)
			{
			    caninter=0;
                pSatIptInfo->XSUsing = 0;
			}
            else
            {

            }
		}
		  
		if(caninter)
		{
            mid = (pSatIptInfo->ObsNum - 1) / 2;
            toett = QCTimeDiff(&Time, &pSatIptInfo->pFtPNTTime[mid]);
			for(k=0;k<3;k++)
			{
			    pRs[k]=GetSatPosByPolyXs(toett,pSatIptInfo->pXs+k*pSatIptInfo->ParaNum,(int)pSatIptInfo->ObsNum);
			}
			if (sys==SYS_GPS||sys==SYS_GAL||sys==SYS_CMP||sys==SYS_BD3)
			{
               //get clk and relativity correction
			   EphClkRelPar(Time,sys, pSatIptInfo,pDts,pRelCorr);
			}
			else
			{
			   GEphClkPar(Time,pSatIptInfo,pDts);
			}
			//get var and svh
			*pVar=pSatIptInfo->Var;
			*pSVH=pSatIptInfo->SVH;
			if(pUsedIODE) *pUsedIODE=pSatIptInfo->IODE;
            if (pToeSec)
            {
                *pToeSec = (int)QCTime2GPSTime(pSatIptInfo->Toe, NULL);
            }
            timer = QCTimeAdd(&Time, tt);
            toett = QCTimeDiff(&timer, &pSatIptInfo->pFtPNTTime[mid]);
            for (k = 0; k < 3; k++)
            {
                rst[k] = GetSatPosByPolyXs(toett, pSatIptInfo->pXs + k * pSatIptInfo->ParaNum, (int)pSatIptInfo->ObsNum);
            }
            if (sys == SYS_GPS || sys == SYS_GAL || sys == SYS_CMP || sys == SYS_BD3)
            {
                EphClkRelPar(timer, sys, pSatIptInfo, dtst, &relcorrt);
            }
            else
            {
                GEphClkPar(timer, pSatIptInfo, dtst);
            }

            pSatIptInfo->XSUsing = 0;
		}
		else
		{
            GNSS_LOG_TRACE("caninter fail, sat=%d, ftp[0]=%f,ftp[n]=%f,vflg=%d\n", Sat,
                pSatIptInfo->pFtPNTTime[0].Time + pSatIptInfo->pFtPNTTime[0].Sec,
                pSatIptInfo->pFtPNTTime[pSatIptInfo->ObsNum - 1].Time + pSatIptInfo->pFtPNTTime[pSatIptInfo->ObsNum - 1].Sec,
                pSatIptInfo->ValidFlag);
		    return 0;
		}
	}
	else
	{
		if (sys==SYS_GPS||sys==SYS_GAL||sys==SYS_CMP||sys==SYS_BD3)
		{
			if (BD3Flag)
			{
				if (!(eph = SelEphBD3(EphTime, Sat, IODE, pNav))) return 0;
			}
			else
			{
				if (!(eph = SelEph(EphTime, Sat, IODE, pNav))) return 0;
			}
        
			if(pUsedIODE) *pUsedIODE=eph->IODE;

			Eph2Pos(Time,eph,pRs,pDts,pVar,pRelCorr);
            *pSVH = eph->SVH == 0 ? 0 : 1;
            if (pToeSec)
            {
                *pToeSec = (int)QCTime2GPSTime(eph->Toe, NULL);
            }
            timer = QCTimeAdd(&Time, tt);
            Eph2Pos(timer, eph, rst, dtst, pVar, &relcorrt);
		}
		else
		{
			if (!(geph=SelGEph(EphTime,Sat,IODE,pNav))) return 0;
		
			if(pUsedIODE) *pUsedIODE=geph->IODE;

			GEph2Pos(Time,geph,pRs,pDts,pVar);
			
            *pSVH = geph->SVH == 0 ? 0 : 1;
            if (pToeSec)
            {
                *pToeSec = (int)QCTime2GPSTime(geph->Toe, NULL);
            }
            timer = QCTimeAdd(&Time, tt);
            GEph2Pos(timer, geph, rst, dtst, pVar);
		}
	}

    /* satellite velocity and clock drift by differential approx */
    for (i = 0; i < 3; i++) pRs[i + 3] = (rst[i] - pRs[i]) / tt;
    pDts[1] = (dtst[0] - pDts[0]) / tt;
	
    return 1;
}

/* satellite position and clock ------------------------------------------------
* compute satellite position, velocity and clock
* args   : gtime_t Time      I   time (gpst)
*          gtime_t EphTime   I   time to select ephemeris (gpst)
*          int    Sat        I   satellite number
*          nav_t  *pNav      I   navigation data
*          int    EphOpt     I   ephemeris option (EPHOPT_???)
*          double *pRs       O   sat position and velocity (ecef)
*                               {x,y,z,vx,vy,vz} (m|m/s)
*          double *pDts      O   sat clock {bias,drift} (s|s/s)
*          double *pVar      O   sat position and clock error variance (m^2)
*          int    *pSVH      O   sat health flag (-1:correction not available)
* return : status (1:ok,0:error)
* notes  : satellite position is referenced to antenna phase center
*          satellite clock does not include code bias correction (tgd or bgd)
*-----------------------------------------------------------------------------*/
static int SatPos(QC_TIME_T Time, QC_TIME_T EphTime, int Sat, int EphOpt,
                  const CHC_NAV_T *pNav, double *pRs, double *pDts, double *pRelCorr, double *pVar,
                  U1 *pSVH,CHC_SATIPTINFO_T *pSatIptInfo,int *pIODE, int* pToeSec, int BD3Flag)
{
    *pSVH=0;
    
    switch (EphOpt) {
        case EPHOPT_BRDC  : return EphPos     (Time,EphTime,Sat,pNav,-1,pRs,pDts,pRelCorr,pVar,pSVH,pSatIptInfo,pIODE, pToeSec, BD3Flag);
		default:break;
	}
    *pSVH=-1;
    return 0;
}

int InitOneSatIptInfo(CHC_SATIPTINFO_T *pSatIptInfo)
{
	memset(pSatIptInfo,0,sizeof(CHC_SATIPTINFO_T));

	pSatIptInfo->pFtPNTTime=(QC_TIME_T *)VERIQC_MALLOC(sizeof(QC_TIME_T)*POLYORDER);
	pSatIptInfo->pRs=(double *)VERIQC_MALLOC(sizeof(double)*POLYORDER*3);
	pSatIptInfo->pXs=(double *)VERIQC_MALLOC(sizeof(double)*POLYORDER*3);
	if(!pSatIptInfo->pFtPNTTime||!pSatIptInfo->pRs||!pSatIptInfo->pXs)
	{
	    return 0;
	}
    
	pSatIptInfo->ParaNum=POLYORDER;
	pSatIptInfo->ObsNum=POLYORDER;

	return 1;
}
void EndOneSatIptInfo(CHC_SATIPTINFO_T *pSatIptInfo)
{
	if(pSatIptInfo->pFtPNTTime){VERIQC_FREE(pSatIptInfo->pFtPNTTime);pSatIptInfo->pFtPNTTime=NULL;}
	if(pSatIptInfo->pRs){VERIQC_FREE(pSatIptInfo->pRs);pSatIptInfo->pRs=NULL;}
	if(pSatIptInfo->pXs){VERIQC_FREE(pSatIptInfo->pXs);pSatIptInfo->pXs=NULL;}

    memset(pSatIptInfo,0,sizeof(CHC_SATIPTINFO_T));
}

int InitSatIptInfo(CHC_SATIPTINFO_T *pSatIptInfo, int Num)
{
	int i,stat;
	if(!pSatIptInfo) return 0;
	for(i=0;i<Num;i++)
	{
	  stat=InitOneSatIptInfo(&pSatIptInfo[i]);
	  if(stat!=1)
	  {
	     return -1;
	  }
	}

	return 1;
}
void EndSatIptInfo(CHC_SATIPTINFO_T *pSatIptInfo, int Num)
{
	int i;
	if(!pSatIptInfo) return;
	for(i=0;i<Num;i++)
	{
	  EndOneSatIptInfo(&pSatIptInfo[i]);
	}
}

//return:1-same;0-diff
static int ChkEphDiff(const CHC_SATIPTINFO_T * pSatIptInfo, int ToeSec, U1 SVH, int IODE)
{
    int stat = 1, toes;
    toes = (int)QCTime2GPSTime(pSatIptInfo->Toe, NULL);

    if (ToeSec != toes ||
        SVH != pSatIptInfo->SVH ||
        IODE != pSatIptInfo->IODE)
    {
        stat = 0;
    }

    return stat;
}

/* satellite positions and clocks ----------------------------------------------
* compute satellite positions, velocities and clocks
* args   : gtime_t EphTime   I   time to select ephemeris (gpst)
*          obsd_t *pObs      I   observation data
*          int    Num        I   number of observation data
*          nav_t  *pNav      I   navigation data
*          int    EphOpt     I   ephemeris option (EPHOPT_???)
*          double *pRs       O   satellite positions and velocities (ecef)
*          double *pDts      O   satellite clocks
*          double *pVar      O   sat position and clock error variances (m^2)
*          int    *pSVH      O   sat health flag (-1:correction not available)
* return : sat number
* notes  : pRs [(0:2)+i*6]= obs[i] sat position {x,y,z} (m)
*          pRs [(3:5)+i*6]= obs[i] sat velocity {vx,vy,vz} (m/s)
*          pDts[(0:1)+i*2]= obs[i] sat clock {bias,drift} (s|s/s)
*          pVar[i]        = obs[i] sat position and clock error variance (m^2)
*          pSVH[i]        = obs[i] sat health flag
*          if no navigation data, set 0 to pRs[], pDts[], pVar[] and pSVH[]
*          satellite position and clock are values at signal transmission time
*          satellite position is referenced to antenna phase center
*          satellite clock does not include code bias correction (tgd or bgd)
*          any pseudorange and broadcast ephemeris are always needed to get
*          signal transmission time
*-----------------------------------------------------------------------------*/
extern unsigned int SatPoss(CHC_SATIPTINFO_T *pSatIptInfo,QC_TIME_T EphTime, const CHC_OBSD_T *pObs, int Num, const CHC_NAV_T *pNav,
                    int EphOpt, double *pRs, double *pDts, double *pVar, U1 *pSVH, U1 *pType,int *pIODE,
                    int* pToeSec, const U1 *pExcludeSats, U1 IncrementUpdateMark,int BD3Flag, int16_t* SatNo, unsigned int *pNumGAL)
{
	QC_TIME_T signalEmitTime={0};
    double dt,pr,relcorr;
    int i,j,sys,prn=0,ephSame,index =0;
    unsigned int ncnt = 0;

    if (pNumGAL)
    {
        *pNumGAL = 0;
    }

    for (i=0;i<Num&&i<MAXOBS*2;i++) {

        sys = pObs[i].Sys;
        prn = pObs[i].PRN;
		if (BD3Flag)
		{
			if (!(sys&(SYS_CMP|SYS_BD3))) continue;
			index = prn - 1;
		}
		else
		{
			index = pObs[i].Sat - 1;
		}

		if (index < 0) continue;

        /* increment update */
        if (IncrementUpdateMark&&pSatIptInfo)
        {
            if (pSatIptInfo[index].ValidFlag)
            {
                ephSame = ChkEphDiff(&pSatIptInfo[index], pToeSec[i], pSVH[i], pIODE[i]);
                if (ephSame)
                {
                    continue;
                }
            }
            else
            {
                if (BD3Flag != pType[i])
                {
                    continue;
                }

				if (BD3Flag)
				{
                    continue;
				}
				else
				{
					for (j = 0; j < 6; j++) pRs[j + i * 6] = 0.0;
					for (j = 0; j < 2; j++) pDts[j + i * 2] = 0.0;
					pVar[i] = 0.0; pSVH[i] = 0;
				}
            }
        }

		if (BD3Flag)
		{
			if (fabs(pRs[0 + i * 6]) > 0.0001)
			{
                continue;
			}
		}
		else
		{
			for (j = 0; j < 6; j++) pRs[j + i * 6] = 0.0;
			for (j = 0; j < 2; j++) pDts[j + i * 2] = 0.0;
			pVar[i] = 0.0; pSVH[i] = 0;
		}

		if(pExcludeSats)
		{
			if(pExcludeSats[pObs[i].Sat-1])
			{
                GNSS_LOG_TRACE("excludesats, sat=%d\n", pObs[i].Sat);
			    continue;
			}
		}
        /* search any psuedorange */
        for (j = 0, pr = 0.0; j < NFREQ; j++)
        {
            if (pObs[i].ObsValidMask & BIT_MASK_CODE(j))
            {
                pr = pObs[i].P[pObs[i].Index[j]];
                break;
            }
        }
        
        if (j>=NFREQ) {
            LOGI("no valid P, sat=%d\n", pObs[i].Sat);
            continue;
        }
        /* transmission time by satellite clock */
        signalEmitTime = QCTimeAdd(&pObs[i].Time, -pr / CLIGHT);

        /* satellite clock bias by broadcast ephemeris */
        if (!EphClk(signalEmitTime, EphTime, pObs[i].Sat, pNav, &dt, BD3Flag, pSatIptInfo ? (&pSatIptInfo[index]) : NULL))
        {
            //gnss_log_trace("ephclk fail, sat=%d", obs[i].sat);
            //gnss_log_trace("sat=%03d,time=%lf, seleph failed", obs[i].sat, obs[i].time.time+obs[i].time.sec);
            continue;
        }
		signalEmitTime=QCTimeAdd(&signalEmitTime,-dt);

        /* satellite position and clock at transmission time */
        if (!SatPos(signalEmitTime,EphTime,pObs[i].Sat,EphOpt,pNav,pRs+i*6,pDts+i*2,&relcorr,pVar+i,
                    pSVH+i,pSatIptInfo?(&pSatIptInfo[index]):NULL,pIODE+i,pToeSec ? (pToeSec + i) : NULL, BD3Flag)) {
            GNSS_LOG_TRACE("satpos fail, sat=%d", pObs[i].Sat);
            continue;
        }
		else
		{
		}
        if (SatNo)
		{
			SatNo[i] = pObs[i].Sat;
		}
    	
        /* if no precise clock unavailable, use broadcast clock instead */
        if (pDts[i*2]==0.0) {
            if (!EphClk(signalEmitTime, EphTime, pObs[i].Sat, pNav, pDts + i * 2, BD3Flag, pSatIptInfo ? (&pSatIptInfo[index]) : NULL)) continue;
            pDts[1+i*2]=0.0;
			pVar[i]=SQR(STD_BRDCCLK);
        }

		if (BD3Flag)
		{
			pType[i] = 1;
		}
        else
        {
            pType[i] = 0;
        }
        ncnt++;
        if (pNumGAL)
        {
            if (sys == SYS_GAL)
            {
                (*pNumGAL)++;
            }
        }
    }
    return ncnt;
}

/*********************************************************************************//**
@brief  get sat approximate pos\vel\clock

@param pSatIptInfo [In]  satellite interpolation information 
@param Time        [In]  time of need
@param Sat         [In]  sat number
@param pNav        [In]  nav info
@param pSatRs      [Out] sat coor(m): 3 Pos + 3 Vel
@param pSatDts     [Out] sat clock(s): clock + clock drift
@param BD3Flag     [In]  BD3 flag

@return TRUE is clac success

@author CHC
@date   22 April 2024
*************************************************************************************/
extern BOOL GetSatApprPosVelClock(CHC_SATIPTINFO_T* pSatIptInfo, QC_TIME_T Time, int Sat, const CHC_NAV_T* pNav, double* pSatRs, double* pSatDts, int BD3Flag)
{
    double var = 0.0;
    double relCorr = 0.0;
    unsigned char svh;
    int iode = 0;
    QC_TIME_T gt;

    gt = QCTimeAdd(&Time, -0.07);  /**< sat single approximate travel time */

    return SatPos(gt, Time, Sat, EPHOPT_BRDC, pNav, pSatRs, pSatDts, &relCorr, &var,
                  &svh, pSatIptInfo, &iode, NULL, BD3Flag);
}

/*********************************************************************************//**
@brief  get BDS/GPS/GAL Eph for specified Toe

@param Sat         [In]  sat number
@param pNav        [In]  nav info
@param BD3Flag     [In]  BD3 flag
@param Toe         [In]  specify Eph Toe

@return Eph pointer or NULL

@author CHC
@date   2025/07/18
*************************************************************************************/
static CHC_EPH_T* SelEphByToe(int Sat, int Toe, const CHC_NAV_T* pNav, int BD3Flag)
{
    int iLoop;
    int EphToe;

    if (BD3Flag)
    {
        for (iLoop = 0; iLoop < pNav->NumBD3; iLoop++)
        {
            if (pNav->pEphBD3[iLoop].Sat != Sat)
            {
                continue;
            }

            EphToe = (int)QCTime2GPSTime(pNav->pEphBD3[iLoop].Toe, NULL);
            if ((Toe >= 0) && (EphToe != Toe))
            {
                continue;
            }

            if (Toe >= 0)
            {
                return pNav->pEphBD3 + iLoop;
            }
        }
    }
    else
    {
        for (iLoop = 0; iLoop < pNav->Num; iLoop++)
        {
            if (pNav->pEph[iLoop].Sat != Sat)
            {
                continue;
            }

            EphToe = (int)QCTime2GPSTime(pNav->pEph[iLoop].Toe, NULL);
            if ((Toe >= 0) && (EphToe != Toe))
            {
                continue;
            }

            if (Toe >= 0)
            {
                return pNav->pEph + iLoop;
            }
        }
    }

    return NULL;
}

/*********************************************************************************//**
@brief  get GLO Eph for specified Toe

@param Sat         [In]  sat number
@param pNav        [In]  nav info
@param Toe         [In]  specify Eph Toe

@return Eph pointer or NULL

@author CHC
@date   2025/07/18
*************************************************************************************/
static CHC_GEPH_T* SelGephByToe(int Sat, int Toe, const CHC_NAV_T* pNav)
{
    int iLoop;
    int EphToe;

    for (iLoop = 0; iLoop < pNav->NumGLO; iLoop++)
    {
        if (pNav->pGEph[iLoop].Sat != Sat)
        {
            continue;
        }

        EphToe = (int)QCTime2GPSTime(pNav->pGEph[iLoop].Toe, NULL);
        if ((Toe >= 0) && (EphToe != Toe))
        {
            continue;
        }

        if (Toe >= 0)
        {
            return pNav->pGEph + iLoop;
        }
    }

    return NULL;
}

/*********************************************************************************//**
@brief  get sat pos\vel\clock specify Eph Toe

@param pSatIptInfo [In]  satellite interpolation information
@param TransTime   [In]  signal transmission time
@param Time        [In]  time of need
@param Sat         [In]  sat number
@param pNav        [In]  nav info
@param pSatRs      [Out] sat coor(m): 3 Pos + 3 Vel
@param pSatDts     [Out] sat clock(s): clock + clock drift
@param BD3Flag     [In]  BD3 flag
@param Toe         [In]  specify Eph Toe

@return TRUE is clac success

@author CHC
@date   2025/07/17
*************************************************************************************/
extern BOOL GetSatPosVelClockSpecifyToe(CHC_SATIPTINFO_T* pSatIptInfo, QC_TIME_T TransTime, QC_TIME_T Time, int Sat, const CHC_NAV_T* pNav, double* pSatRs, double* pSatDts, int BD3Flag, int Toe)
{
    double Var = 0.0;
    double RelCorr = 0.0;
    double DeltaT;
    double RsTemp[3];
    double DtsTemp[1];
    double TT = 1E-3;
    double RelcorrTemp = 0;
    unsigned char SVH;
    int iLoop;
    int Sys;
    int IODE = 0;
    CHC_EPH_T* Eph;
    CHC_GEPH_T* GEph;
    QC_TIME_T Timer = { 0 };

    /** adjust transmission time */
    if (!EphClk(TransTime, Time, Sat, pNav, &DeltaT, 0, pSatIptInfo))
    {
        return FALSE;
    }
    TransTime = QCTimeAdd(&TransTime, -DeltaT);

    Sys = SatSys(Sat, NULL);

    /** select specified eph, calculate satellite position */
    if ((Sys == SYS_GPS) || (Sys == SYS_GAL) || (Sys == SYS_CMP) || (Sys == SYS_BD3))
    {
        if (!(Eph = SelEphByToe(Sat, Toe, pNav, BD3Flag)))
        {
            return FALSE;
        }

        IODE = Eph->IODE;

        Eph2Pos(TransTime, Eph, pSatRs, pSatDts, &Var, &RelCorr);
        SVH = Eph->SVH == 0 ? 0 : 1;

        Timer = QCTimeAdd(&TransTime, TT);
        Eph2Pos(Timer, Eph, RsTemp, DtsTemp, &Var, &RelcorrTemp);
    }
    else
    {
        if (!(GEph = SelGephByToe(Sat, Toe, pNav)))
        {
            return FALSE;
        }

        IODE = GEph->IODE;

        GEph2Pos(TransTime, GEph, pSatRs, pSatDts, &Var);
        SVH = (GEph->SVH == 0) ? 0 : 1;

        Timer = QCTimeAdd(&TransTime, TT);
        GEph2Pos(Timer, GEph, RsTemp, DtsTemp, &Var);
    }

    /** satellite velocity and clock drift by differential approx */
    for (iLoop = 0; iLoop < 3; iLoop++)
    {
        pSatRs[iLoop + 3] = (RsTemp[iLoop] - pSatRs[iLoop]) / TT;
    }
    pSatDts[1] = (DtsTemp[0] - pSatDts[0]) / TT;

    return TRUE;
}
