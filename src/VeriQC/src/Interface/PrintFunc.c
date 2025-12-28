/**********************************************************************//**
		   VeriQC

	Interface Module
*-
@file   PrintFunc.c
@author CHC
@date   2024/06/18
@brief  file output related functions

**************************************************************************/

#include <float.h>
#include "PrintFunc.h"
#include "CommonBase.h"

#define MAX_HTML_STR_LEN 1024*30

char g_TemplateHtmlParh[512] = "";
char* g_SequenceUnit = ",";
char* g_SequenceLine = "+-------------------------------------------------------------------------------------+";

#ifndef WIN32
static const short g_MapChar2WcharChinese[280 * 3] =
{
	  -58, -75,39057,
	 -75, -29,28857,
	 -74, -44,23545,
	 -45, -90,24212,
	 -42, -54,36136,
	 -68, -20,26816,
	 -41, -36,24635,
	 -67, -31,32467,
	 -46, -58,31227,
	 -74, -81,21160,
	 -43, -66,31449,
	 -69,  -7,22522,
	 -41, -68,20934,
	 -43, -66,31449,
	 -78, -30,27979,
	 -54, -44,35797,
	 -80, -26,29256,
	 -42, -54,36136,
	 -68, -20,26816,
	 -79, -88,25253,
	 -72, -26,21578,
	 -58, -75,39057,
	 -75, -29,28857,
	 -74, -44,23545,
	 -45, -90,24212,
	 -71, -40,20851,
	 -49, -75,31995,
	 -49, -75,31995,
	 -51, -77,32479,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -42, -54,36136,
	 -68, -20,26816,
	 -41, -36,24635,
	 -67, -31,32467,
	 -42, -54,36136,
	 -68, -20,26816,
	 -60, -93,27169,
	 -48, -51,22411,
	 -50, -34,26080,
	 -46, -58,31227,
	 -74, -81,21160,
	 -43, -66,31449,
	 -50, -60,25991,
	 -68,  -2,20214,
	 -69,  -7,22522,
	 -41, -68,20934,
	 -43, -66,31449,
	 -50, -60,25991,
	 -68,  -2,20214,
	 -46, -58,31227,
	 -74, -81,21160,
	 -43, -66,31449,
	 -46, -58,31227,
	 -74, -81,21160,
	 -43, -66,31449,
	 -50, -60,25991,
	 -68,  -2,20214,
	 -58, -67,24179,
	 -66,  -7,22343,
	 -41,  -8,22352,
	 -79, -22,26631,
	 -65, -86,24320,
	 -54, -68,22987,
	 -71, -37,35266,
	 -78, -30,27979,
	 -54, -79,26102,
	 -68, -28,38388,
	 -67, -31,32467,
	 -54,  -8,26463,
	 -71, -37,35266,
	 -78, -30,27979,
	 -54, -79,26102,
	 -68, -28,38388,
	 -54, -79,26102,
	 -68, -28,38388,
	 -68, -28,38388,
	 -72, -12,38548,
	 -71, -37,35266,
	 -78, -30,27979,
	 -64,  -6,21382,
	 -44, -86,20803,
	 -54,  -3,25968,
	 -45, -48,26377,
	 -48, -89,25928,
	 -71, -37,35266,
	 -78, -30,27979,
	 -54,  -3,25968,
	 -64, -19,29702,
	 -62, -37,35770,
	 -71, -37,35266,
	 -78, -30,27979,
	 -54,  -3,25968,
	 -64,  -5,21033,
	 -45, -61,29992,
	 -62, -54,29575,
	 -42, -36,21608,
	 -52,  -8,36339,
	 -79, -56,27604,
	 -70, -49,21512,
	 -72, -15,26684,
	 -65, -55,21487,
	 -45, -61,29992,
	 -62, -54,29575,
	 -73, -42,20998,
	 -49, -75,31995,
	 -51, -77,32479,
	 -49, -75,31995,
	 -51, -77,32479,
	 -41, -36,24635,
	 -42, -36,21608,
	 -52,  -8,36339,
	 -54,  -3,25968,
	 -52, -67,25506,
	 -78, -30,27979,
	 -54,  -3,25968,
	 -42, -36,21608,
	 -52,  -8,36339,
	 -79, -56,27604,
	 -65, -55,21487,
	 -45, -61,29992,
	 -62, -54,29575,
	 -70, -51,21644,
	 -42, -36,21608,
	 -52,  -8,36339,
	 -79, -56,27604,
	 -73, -42,20998,
	 -50, -64,21355,
	 -48, -57,26143,
	 -50, -64,21355,
	 -48, -57,26143,
	 -65, -55,21487,
	 -60, -36,33021,
	 -71, -37,35266,
	 -78, -30,27979,
	 -42, -75,20540,
	 -54, -75,23454,
	 -68, -54,38469,
	 -71, -37,35266,
	 -78, -30,27979,
	 -42, -75,20540,
	 -64,  -5,21033,
	 -45, -61,29992,
	 -62, -54,29575,
	 -42, -36,21608,
	 -52,  -8,36339,
	 -54,  -3,25968,
	 -52, -67,25506,
	 -78, -30,27979,
	 -54,  -3,25968,
	 -42, -36,21608,
	 -52,  -8,36339,
	 -79, -56,27604,
	 -58, -67,24179,
	 -66,  -7,22343,
	 -44, -40,36733,
	 -44, -21,22122,
	 -79, -56,27604,
	 -70, -49,21512,
	 -72, -15,26684,
	 -49, -75,31995,
	 -51, -77,32479,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -67,24179,
	 -66,  -7,22343,
	 -44, -40,36733,
	 -44, -21,22122,
	 -79, -56,27604,
	 -93, -70,65306,
	 -78, -69,19981,
	 -51, -84,21516,
	 -72, -33,39640,
	 -74, -56,24230,
	 -67, -57,35282,
	 -73, -74,33539,
	 -50, -89,22260,
	 -49, -75,31995,
	 -51, -77,32479,
	 -58, -75,39057,
	 -74, -50,27573,
	 -50, -79,20266,
	 -66, -32,36317,
	 -74, -32,22810,
	 -62, -73,36335,
	 -66, -74,24452,
	 -93, -70,65306,
	 -73, -42,20998,
	 -49, -75,31995,
	 -51, -77,32479,
	 -73, -42,20998,
	 -58, -75,39057,
	 -74, -50,27573,
	 -78, -69,19981,
	 -70, -49,21512,
	 -72, -15,26684,
	 -49, -75,31995,
	 -51, -77,32479,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -50, -79,20266,
	 -66, -32,36317,
	 -74, -32,22810,
	 -62, -73,36335,
	 -66, -74,24452,
	 -93, -70,65306,
	 -73, -42,20998,
	 -50, -64,21355,
	 -48, -57,26143,
	 -73, -42,20998,
	 -58, -75,39057,
	 -74, -50,27573,
	 -50, -64,21355,
	 -48, -57,26143,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573,
	 -58, -75,39057,
	 -74, -50,27573
};
static unsigned short GetChineseChar(char Char1, char Char2)
{
	int ichar1, ichar2;
	unsigned short ret = g_MapChar2WcharChinese[2];
	for (ichar1 = 0; ichar1 < 280; ichar1++)
	{
		if ((g_MapChar2WcharChinese[ichar1 * 3 + 0] == (int)Char1)&&
			(g_MapChar2WcharChinese[ichar1 * 3 + 1] == (int)Char2))
		{
			ret = g_MapChar2WcharChinese[ichar1 * 3 + 2];
			break;
		}
	}
	return ret;
}
#endif
/*------------------------------------------------------------*
* GPS	    L1、L2C、L5、 L1C、    L2P
* BDS	    B1、B2、 B3、 B1C、    B2a、B1A、B3A、B2b、B2a+B2b
* GLONASS	G1、G1a、G2、 G2a、    G3
* Galileo	E1、E5A、E5B、E5(A+B)、E6
* QZSS	    L1、L2、 L5、 L5S、    L6、L1C(P)、L1S
* SBAS	    L1、L5
*-------------------------------------------------------------*/
static int8_t g_SigType[QC_MAX_NUM_SYS][QC_NUM_FREQ][8]={
	{"L1","L2C","L5","L1C","L2P",""},/*GPS*/
	{"L1","L5",  "",     "",     "",""},/*SBAS*/
	{"G1","G2","", "",   "",""},/*GLONASS*/
	{"E1","E5a","E5b","E5(a+b)","E6",""},/*Galileo*/
	{"L1","L2","L5","L1C(P)","L6","-"},/*QZSS*/
	{"B1","B2","B3","B1C","B2a","B2b"}/*BDS*/
};

extern BOOL SetExePath(char* pExePath)
{
	char outDir[256] = { 0 };
	if (pExePath != NULL)
	{
		GetOutDirStr(pExePath, outDir);
		memset(g_TemplateHtmlParh, 0, sizeof(char) * 512);
		sprintf(g_TemplateHtmlParh, "%stemplate.html", outDir);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	
}

static char* QCGetSysStr(int Id)
{
	char* pHead = NULL;
	switch (Id)
	{
	case QC_GNSS_SYSID_GPS :pHead = " GPS"; break;
	case QC_GNSS_SYSID_SBAS:pHead = " SBS"; break;
	case QC_GNSS_SYSID_GLN :pHead = " GLO"; break;
	case QC_GNSS_SYSID_GAL :pHead = " GAL"; break;
	case QC_GNSS_SYSID_QZS :pHead = " QZS"; break;
	case QC_GNSS_SYSID_BDS :pHead = " BDS"; break;
	default:
		break;
	}
	return pHead;
}

void PrintObsFileInfoHtml(FILE* fpOut, QC_RESULT_T* pQcRes, QC_OPT_T* pOpt, int AntID)
{
	float* MAX_mp[QC_MAX_NUM_SYS] = { pOpt->GMpMAX,pOpt->SMpMAX,pOpt->RMpMAX,pOpt->EMpMAX,pOpt->JMpMAX,pOpt->CMpMAX };
	int* MIN_snr[QC_MAX_NUM_SYS] = { pOpt->GSnrMIN,pOpt->SSnrMIN,pOpt->RSnrMIN,pOpt->ESnrMIN,pOpt->JSnrMIN,pOpt->CSnrMIN };
	int iSys, iFre, iSat, iDeg;
	BOOL invail_sys = FALSE;
	char ts_str[20] = { 0 };
	char te_str[20] = { 0 };
	char prn_str[4] = { 0 }, * File_str[2] = { "QC Result ","QC Result " };
	char* buff_t = (char*)VERIQC_CALLOC(MAX_HTML_STR_LEN, sizeof(char));
	char buff[3000] = { 0 };
	int len = 0, GoodNum = 0, BadNum = 0, sys, prn;
	int m_SlipNum[QC_MAX_NUM_SYS] = { 0 }, m_DetectNum[QC_MAX_NUM_SYS] = { 0 };

	len += sprintf(buff_t + len, "<table class=\"Table\">\n<h1 class=\"Title_1\"><a id=\"T%d\">%s</a></h1>\n", AntID + 1, File_str[AntID]);
	len += sprintf(buff_t + len, "<tr>");
	len += sprintf(buff_t + len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "File:");//first col
	if (AntID == 0)
	{
		len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%s</td>", pOpt->RoverFile);//second col
	}
	else if (AntID == 1)
	{
		len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%s</td>", pOpt->BaseFile);//second col
	}
	len += sprintf(buff_t + len, "</tr>\n");
	len += sprintf(buff_t + len, "<tr>");
	len += sprintf(buff_t + len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "Average Pos(ECEF):");//first col
	len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%10.3f,%10.3f,%10.3f</td>", pQcRes->PosXYZ[AntID][0], pQcRes->PosXYZ[AntID][1], pQcRes->PosXYZ[AntID][2]);//second col
	len += sprintf(buff_t + len, "</tr>\n");
	QCTime2String(&pQcRes->StartTime[AntID], ts_str, 0);
	QCTime2String(&pQcRes->EndTime[AntID], te_str, 0);
	len += sprintf(buff_t + len, "<tr>");
	len += sprintf(buff_t + len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "Start Time:");//
	len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%s</td>", ts_str);
	len += sprintf(buff_t + len, "</tr>\n");
	len += sprintf(buff_t + len, "<tr>");
	len += sprintf(buff_t + len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "End Time:");//first col
	len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%s</td>", te_str);
	len += sprintf(buff_t + len, "</tr>\n");
	len += sprintf(buff_t + len, "<tr>");
	len += sprintf(buff_t + len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "Interval(s):");//first col
	len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%.1f</td>", pQcRes->Interval[AntID]);
	len += sprintf(buff_t + len, "</tr>\n");
	len += sprintf(buff_t + len, "<tr>");
	len += sprintf(buff_t + len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "Epoch Number:");//first col
	len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%d</td>", pQcRes->MaxEpochNum[AntID]);
	len += sprintf(buff_t + len, "</tr>\n");
	len += sprintf(buff_t + len, "<tr>");
	len += sprintf(buff_t + len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "Complete Obs Number:");//
	len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%d</td>", pQcRes->CompleteNum[AntID]);
	len += sprintf(buff_t + len, "</tr>\n");
	len += sprintf(buff_t + len, "<tr>");
	len += sprintf(buff_t + len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "Possible Obs Number:");//
	len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%d</td>", pQcRes->PossibleNum[AntID]);
	len += sprintf(buff_t + len, "</tr>\n");
	len += sprintf(buff_t + len, "<tr>");
	len += sprintf(buff_t + len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "Use Ratio(%):");//first col
	len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%.1f</td>", 100.0*pQcRes->UseRateTotal[AntID]);//second col
	len += sprintf(buff_t + len, "</tr>\n");
	len += sprintf(buff_t + len, "<tr>");
	len += sprintf(buff_t + len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "Cycle Slip Ratio:");//first col
	if (pQcRes->OSR[AntID] < pOpt->CJR)
	{	
		len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%.1f %s</td>", pQcRes->OSR[AntID],"FAIL");//second col	
	}
	else
	{
		len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%.1f %s</td>", pQcRes->OSR[AntID],"PASS");//second col
	}
	len += sprintf(buff_t + len, "</tr>\n");
	len += sprintf(buff_t + len, "<tr>");
	len += sprintf(buff_t + len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "Elevation Mask(Deg):");//first col
	len += sprintf(buff_t + len, "<td class=\"ColumnTwoTotalTwo\">%d</td>", pOpt->EleDeg);//second col
	len += sprintf(buff_t + len, "</tr>\n");
	len += sprintf(buff_t + len, "</table>\n");
	UpdateString2File(fpOut, buff_t, &len);
	/** 1 #Use Rate(%) */
	len += sprintf(buff_t + len, "<table class=\"Table\">\n<h2 class=\"Title_1\">%s</h2>\n", "Use Ratio:(%)");
	len += sprintf(buff_t + len, "<tr class=\"TableHeader\"><th class=\"NotAZIColum\">%s</th>\n", "System");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "Use Ratio(%)");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "Cycle Slip Number");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "Detect Number");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th></tr>\n", "Cycle Slip Ratio");
	for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
	{
		invail_sys = FALSE;
		if (pQcRes->SatPossibleNum[AntID][iSat] > 0)
		{
			invail_sys = TRUE;
		}
		if (!invail_sys)
		{
			continue;
		}
		sys = GetQCSatSys(iSat + 1, &prn);
		iSys = QCSys2Id(sys) - 1;
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (pQcRes->DetectNum[AntID][iSat][iFre] < 0.1)
			{

			}
			else
			{
				m_SlipNum[iSys] += pQcRes->SlipNum[AntID][iSat][iFre];
				m_DetectNum[iSys] += pQcRes->DetectNum[AntID][iSat][iFre];
			}
		}
		
	}
	for (iSys = 0; iSys < QC_MAX_NUM_SYS; iSys++)
	{
		if (pQcRes->UseRateSys[AntID][iSys] >= 0)
		{
			invail_sys = TRUE;
		}
		if (!invail_sys)
		{
			continue;
		}
		len += sprintf(buff_t + len, "<tr><td class = \"ColumnFirstTotalTwo\">%s:</td>\n", QCGetSysStr(iSys + 1));
		len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%5.1f</td>\n", pQcRes->UseRateSys[AntID][iSys] * 100.0);
		len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%d</td>\n", m_SlipNum[iSys]);
		len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%d</td>\n", m_DetectNum[iSys]);

		if (m_DetectNum[iSys] > 0 && m_SlipNum[iSys] > 0)
		{
			len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%5.1f</td>\n", 1.0 * m_DetectNum[iSys] / m_SlipNum[iSys]);
		}
		else if (m_DetectNum[iSys] > 0 && m_SlipNum[iSys] < 0.1)
		{
			len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%.1f</td>\n", 99999.0);
		}
		else if (m_DetectNum[iSys] < 0.1)
		{
			len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%.1f</td>\n", 99999.0);
		}
		len += sprintf(buff_t + len, "</tr>\n");
		invail_sys = FALSE;
	}
	len += sprintf(buff_t + len, "</table>\n");
	UpdateString2File(fpOut, buff_t, &len);
	len += sprintf(buff_t + len, "<table class=\"Table\">\n<h2 class=\"Title_1\">%s</h2>\n", "Use ratio and Cycle Slip Ratio:");
	len += sprintf(buff_t + len, "<tr class=\"TableHeader\"><th class=\"NotAZIColum\">%s</th>\n", "Satellite");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "Possible Obs");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "Complete Obs");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "Use Ratio");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "CS<br>G:L1/R:G1/E:E1/<br>J:L1/C:B1");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "CS<br>G:L2C/R:G2/E:E5a<br>J:L2/C:B2");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "CS<br>G:L5/E:E5b/<br>J:L5/C:B3");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "CS<br>G:L1C/E:E5(a+b)/<br>J:L1C(P)/C:B1C");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "CS<br>G:L2P/E:E6/<br>J:L6/C:B2a");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "CS<br>C:B2b");
	/** len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "Detect Number"); */
	/** len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th></tr>\n", "Cycle Slip Ratio"); */
	UpdateString2File(fpOut, buff_t, &len);
	for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
	{
		invail_sys = FALSE;
		if (pQcRes->SatPossibleNum[AntID][iSat] > 0)
		{
			invail_sys = TRUE;
		}
		if (!invail_sys)
		{
			continue;
		}
		UpdateString2File(fpOut, buff_t, &len);
		QCSatNo2Str(iSat + 1, prn_str);
		len += sprintf(buff_t + len, "<tr><td class = \"ColumnFirstTotalTwo\">%s:</td>\n", prn_str);
		len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%d</td>\n", pQcRes->SatPossibleNum[AntID][iSat]);
		len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%d</td>\n", pQcRes->SatCompleteNum[AntID][iSat]);
		len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%.1f</td>\n", pQcRes->UseRateSat[AntID][iSat]);
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%d</td>\n", pQcRes->SlipNum[AntID][iSat][iFre]);
			/** len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%d</td>\n", QcRes->DetectNum[ant_id][iSat][iFre]); */
		}

		/**
		if (QcRes->DetectNum[ant_id][iSat] > 0 && QcRes->SlipNum[ant_id][iSat] > 0)
		{
			len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%5.1f</td>\n", 1.0 * QcRes->DetectNum[ant_id][iSat] / QcRes->SlipNum[ant_id][iSat]);
		}
		else if (QcRes->DetectNum[ant_id][iSat] > 0 && QcRes->SlipNum[ant_id][iSat] < 0.1)
		{
			len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%.1f</td>\n", 99999.0);
		}
		else if (QcRes->DetectNum[ant_id][iSat] < 0.1)
		{
			len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%.1f</td>\n", 99999.0);
		}*/
		len += sprintf(buff_t + len, "</tr>\n");
	}
	len += sprintf(buff_t + len, "</table>\n");
	UpdateString2File(fpOut, buff_t, &len);
	/** 2 #AVE SNR(dBHz) */
	//len += sprintf(buff_t + len, "<table class=\"Table\">\n<h2 class=\"Title_1\">%s</h2>\n", "Average CN0(dBHz)");
	len += sprintf(buff_t + len, "<tr class=\"TableHeader\"><th class=\"NotAZIColum\">%s</th>\n", "System");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L1/R:G1/E:E1/<br>J:L1/C:B1");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L2C/R:G2/E:E5a<br>J:L2/C:B2");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L5/E:E5b/<br>J:L5/C:B3");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L1C/E:E5(a+b)/<br>J:L1C(P)/C:B1C");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L2P/E:E6/<br>J:L6/C:B2a");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "C:B2b");
	for (iSys = 0; iSys < QC_MAX_NUM_SYS; iSys++)
	{
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (pQcRes->SNR[AntID][iSys][iFre] > 0)
			{
				invail_sys = TRUE;
			}
		}
		if (!invail_sys)
		{
			continue;
		}
		len += sprintf(buff_t + len, "<tr><td class = \"ColumnFirstTotalTwo\">%s:</td>\n", QCGetSysStr(iSys + 1));
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (pQcRes->SNR[AntID][iSys][iFre] > 0)
			{
				len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%5.1f</td>\n", pQcRes->SNR[AntID][iSys][iFre]);
				if (pQcRes->SNR[AntID][iSys][iFre] >= MIN_snr[iSys][iFre])
				{
					GoodNum++;
				}
				else
				{
					BadNum++;
				}
			}
			else
			{
				len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%s</td>\n", "--");
			}
		}
		len += sprintf(buff_t + len, "</tr>");
		invail_sys = FALSE;
	}
	invail_sys = FALSE;
	len += sprintf(buff_t + len, "</table>\n");
	memset(buff, 0, sizeof(char) * 3000);
	memcpy(buff, buff_t, sizeof(char)* len);
	if (BadNum > 0)
	{
		len = sprintf(buff_t, "<table class=\"Table\">\n<h2 class=\"Title_1\">%s</h2>\n%s", "Average CN0(dBHz):FAIL", buff);
	}
	else
	{
		len = sprintf(buff_t, "<table class=\"Table\">\n<h2 class=\"Title_1\">%s</h2>\n%s", "Average CN0(dBHz):PASS", buff);
	}

	UpdateString2File(fpOut, buff_t, &len);
	len += sprintf(buff_t + len, "<table class=\"Table\">\n<h2 class=\"Title_1\">%s</h2>\n", "Average CN0: Different Ele Range(dBHz)");
	len += sprintf(buff_t + len, "<tr class=\"TableHeader\"><th class=\"NotAZIColum\">%s</th>", "System");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "Frequency");
	for (iDeg = 0; iDeg < pQcRes->IntervNum; iDeg++)
	{
		if (iDeg == 0)
		{
			len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">[%.1f,%.1f)</th>\n", 0.0, pQcRes->DegThreshold[iDeg]);
		}
		else
		{
			len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">[%.1f,%.1f)</th>\n", pQcRes->DegThreshold[iDeg - 1], pQcRes->DegThreshold[iDeg]);
		}
	}
	len += sprintf(buff_t + len, "</tr>\n");
	PrintSNRDegHtml(buff_t, &len, AntID, pQcRes);
	len += sprintf(buff_t + len, "</table>\n");
	UpdateString2File(fpOut, buff_t, &len);
	BadNum = 0; GoodNum = 0;
	/** 3 #RMS MP(m) */
	//len += sprintf(buff_t + len, "<table class=\"Table\">\n<h2 class=\"Title_1\">%s</h2>\n", "PSR Multipath RMS: Subsystems and frequency bands.(m)");
	len += sprintf(buff_t + len, "<tr class=\"TableHeader\"><th class=\"NotAZIColum\">%s</th>\n", "System");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L1/R:G1/E:E1/<br>J:L1/C:B1");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L2C/R:G2/E:E5a<br>J:L2/C:B2");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L5/E:E5b/<br>J:L5/C:B3");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L1C/E:E5(a+b)/<br>J:L1C(P)/C:B1C");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L2P/E:E6/<br>J:L6/C:B2a");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "C:B2b");
	for (iSys = 0; iSys < QC_MAX_NUM_SYS; iSys++)
	{
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (pQcRes->MPTotel[AntID][iSys][iFre] > 0)
			{
				invail_sys = TRUE;
			}
		}
		if (!invail_sys)
		{
			continue;
		}

		len += sprintf(buff_t + len, "<tr><td class = \"ColumnFirstTotalTwo\">%s</td>\n", QCGetSysStr(iSys + 1));
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (pQcRes->MPTotel[AntID][iSys][iFre] > 0)
			{
				len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%5.3f</td>\n", pQcRes->MPTotel[AntID][iSys][iFre]);
				if (pQcRes->MPTotel[AntID][iSys][iFre] <= MAX_mp[iSys][iFre])
				{
					GoodNum++;
				}
				else
				{
					BadNum++;
				}
			}
			else
			{
				len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%s</td>\n", "--");
			}
		}
		invail_sys = FALSE;
		len += sprintf(buff_t + len, "</tr>\n");
	}
	len += sprintf(buff_t + len, "</table>\n");
	memset(buff, 0, sizeof(char)* 3000);
	memcpy(buff, buff_t, sizeof(char)* len);
	if (BadNum > 0)
	{
		len = sprintf(buff_t, "<table class=\"Table\">\n<h2 class=\"Title_1\">%s</h2>\n%s", "Pseudorange Multipath RMS:(m):FAIL", buff);
	}
	else
	{
		len = sprintf(buff_t, "<table class=\"Table\">\n<h2 class=\"Title_1\">%s</h2>\n%s", "Pseudorange Multipath RMS:(m):PASS", buff);
	}
	UpdateString2File(fpOut, buff_t, &len);
	len += sprintf(buff_t + len, "<table class=\"Table\">\n<h2 class=\"Title_1\">%s</h2>\n", "Pseudorange Multipath RMS:(m)");
	len += sprintf(buff_t + len, "<tr class=\"TableHeader\"><th class=\"NotAZIColum\">%s</th>\n", "Satellite");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L1/R:G1/E:E1/<br>J:L1/C:B1");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L2C/R:G2/E:E5a<br>J:L2/C:B2");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L5/E:E5b/<br>J:L5/C:B3");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L1C/E:E5(a+b)/<br>J:L1C(P)/C:B1C");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "G:L2P/E:E6/<br>J:L6/C:B2a");
	len += sprintf(buff_t + len, "<th class=\"NotAZIColum\">%s</th>\n", "C:B2b");
	UpdateString2File(fpOut, buff_t, &len);
	for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
	{
		invail_sys = FALSE;
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (pQcRes->MPNum[AntID][iSat][iFre] > 0)
			{
				invail_sys = TRUE;
				break;
			}
		}
		if (!invail_sys)
		{
			continue;
		}
		QCSatNo2Str(iSat + 1, prn_str);
		UpdateString2File(fpOut, buff_t, &len);
		len += sprintf(buff_t + len, "<tr><td class = \"ColumnFirstTotalTwo\">%s:</td>\n", prn_str);
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (pQcRes->MPNum[AntID][iSat][iFre] < 0.1)
			{
				len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%s</td>\n", "--");
			}
			else
			{
				len += sprintf(buff_t + len, "<td class = \"ColumnTwoTotalTwo\">%5.3f</td>\n", QC_SQRT(1.0 * pQcRes->MP[AntID][iSat][iFre] / pQcRes->MPNum[AntID][iSat][iFre]));
			}

		}
		len += sprintf(buff_t + len, "</tr>");
	}
	len += sprintf(buff_t + len, "</table>");
	UpdateString2File(fpOut, buff_t, &len);
	VERIQC_FREE(buff_t);
	buff_t = NULL;
}

int PrintSNRDegHtml(char* pBuffer,int* pBufferLen, int AntID, QC_RESULT_T* pQCRes)
{
	int iDeg, iSys, iFre;
	int numFre = 0;
	int len = *pBufferLen;
	
	for (iSys = 0; iSys < QC_MAX_NUM_SYS; iSys++)
	{
		if (iSys == QC_GNSS_SYSID_SBAS - 1)
		{
			continue;
		}

		numFre = QCId2SysFreNum(iSys + 1);

		for (iFre = 0; iFre < numFre; iFre++)
		{
			if (iFre != floor(numFre / 2))
			{
				len += sprintf(pBuffer + len, "<tr><td class = \"ColumnFirstTotalTwo\"> %s</td>", "");
			}
			else
			{
				len += sprintf(pBuffer + len, "<tr><td class = \"ColumnFirstTotalTwo\"> %s</td>", QCGetSysStr(iSys + 1));
			}
			
			len += sprintf(pBuffer + len, "<td class = \"ColumnTwoTotalTwo\"> %s</td>", g_SigType[iSys][iFre]);
			
			for (iDeg = 0; iDeg < pQCRes->IntervNum; iDeg++)
			{
				if (pQCRes->SNRDeg[AntID][iSys][iDeg][iFre] > 0)
				{
					len += sprintf(pBuffer + len, "<td class = \"ColumnTwoTotalTwo\"> %.1f</td>", pQCRes->SNRDeg[AntID][iSys][iDeg][iFre]);
				}
				else
				{
					len += sprintf(pBuffer + len, "<td class = \"ColumnTwoTotalTwo\"> %s</td>", "--");
				}
			}

			len += sprintf(pBuffer + len, "</tr>\n");
		}
	}
	*pBufferLen = len;
	return len;
}

void PrintEpochCS(QC_TIME_T* pTime, FILE* fpOutFile, QC_RESULT_T* QcRes, int AntID, LOG_HEADTYPE_E eLogType)
{
	char msg[1049] = { 0 };
	int iSat, invail = FALSE, iSys, sys, prn;
	int len = 0;
	double GpsSec;
	int GpsWeek;
	int iFre = 0;
	if (!fpOutFile)
	{
		return;
	}
	for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
	{
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (QcRes->SlipFlag[AntID][iSat][iFre] > 0)
			{
				invail = TRUE;
				break;
			}
		}
		if (invail == TRUE)
		{
			break;
		}
	}
	if (!invail)
	{
		return;
	}
	GpsSec = QCTime2GPSTime(*pTime, &GpsWeek);
#if 0
	QC_Time2Str(pTime, tstr, 1);
	if (eLogType == E_LOG_HEADTYPE_1)
	{
		len+=sprintf(msg+len, "%s", tstr);
	}
	else if (eLogType == E_LOG_HEADTYPE_2)
	{
		len += sprintf(msg + len, "%4d%s%8.1f", GpsWeek, g_seq_unit, GpsSec);
	}
#else
	len += sprintf(msg + len, "%4d%s%8.1f", GpsWeek, g_SequenceUnit, GpsSec);
#endif
	fprintf(fpOutFile, "%s\n", msg);
	memset(msg, 0, sizeof(char) * len);
	len = 0;
	for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
	{
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (QcRes->SlipFlag[AntID][iSat][iFre] > 0)
			{
				sys = GetQCSatSys(iSat + 1, &prn);
				iSys = QCSys2Id(sys) - 1;
				if (iSys < 0)
				{
					continue;
				}
				QCSatNo2Str(iSat + 1, msg);
				msg[3] = '\0';
				fprintf(fpOutFile, "%s,%d\n", msg, iFre);
			}
		}
	}
	fflush(fpOutFile);
}

/**********************************************************************//**
@brief print satellite data list log

@param pTime       [In]      Epoch time
@param pOutFile    [In]      satellite data list file pointer
@param pObs        [In]      Epoch satellite observation data
@param pSlipFlag   [In]      Cycle skip flag
@param eLogType    [In]      log format type
@param bCheckLLI   [In]      If ADR check LLI
@param EleMask     [In]      Elevation cut off, rad

@author CHC
@date 2024/04/26
@note
History:\n
**************************************************************************/
void PrintEpochDataList(QC_TIME_T* pTime, FILE* pOutFile, QC_OBS_T* pObs, int (*pSlipFlag)[QC_NUM_FREQ], LOG_HEADTYPE_E eLogType, BOOL bCheckLLI, double EleMask)
{
	char msg[1049] = { 0 };
	const static unsigned char s_NullBuffer[QC_NUM_FREQ + 3] = ",,,,,,,,,";
	int iSat, iFre, invail = FALSE, iSys, sys, prn;
	int len = 0;
	double GpsSec;
	int GpsWeek;
	if (!pOutFile)
	{
		return;
	}

#if 0
	char tstr[32] = { 0 };
	char sPrn[4] = { 0 };
	QC_Time2Str(pTime, tstr, 1);
#endif

	GpsSec = QCTime2GPSTime(*pTime, &GpsWeek);

	for (iSat = 0; iSat < pObs->ObsNum; iSat++)
	{
		invail = FALSE;
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if ((pObs->Data[iSat].L[iFre] != 0) || (pObs->Data[iSat].P[iFre] != 0))
			{
				invail = TRUE;
				break;
			}
		}
		if (!invail)
		{
			continue;
		}
		sys = GetQCSatSys(pObs->Data[iSat].Sat, &prn);
		iSys = QCSys2Id(sys) - 1;
		if (iSys < 0)
		{
			continue;
		}

#if 0
		QC_Satno2str(iSat + 1, sPrn);
		if (eLogType == E_LOG_HEADTYPE_1)
		{
			len += sprintf(msg + len, "%s%s%s%s", tstr, g_seq_unit, sPrn, g_seq_unit);
		}
		else if (eLogType == E_LOG_HEADTYPE_2)
		{
			len += sprintf(msg + len, "%4d%s%8.1f%s%d%s%02d%s", GpsWeek, g_seq_unit, GpsSec, g_seq_unit, iSys + 1, g_seq_unit, prn, g_seq_unit);
		}
#else
		len += sprintf(msg + len, "%4d%s%8.1f%s%d%s%02d%s", GpsWeek, g_SequenceUnit, GpsSec, g_SequenceUnit, iSys + 1, g_SequenceUnit, prn, g_SequenceUnit);
#endif

		/** print EL */
		len += sprintf(msg + len, "%4.1f%s", (pObs->Data[iSat].Azel[1] * R2D), g_SequenceUnit);
		/** print L */
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if ((fabs(pObs->Data[iSat].L[iFre]) >= DBL_EPSILON) && (len >= 0))
			{
				if ((bCheckLLI == TRUE) && (pObs->Data[iSat].LLI[iFre] != 0))
				{
					msg[len++] = '0';
					msg[len++] = g_SequenceUnit[0];
				} 
				else
				{
					msg[len++] = '1';
					msg[len++] = g_SequenceUnit[0];
				}
			}
			else if (len >= 0)
			{
				msg[len++] = '0';
				msg[len++] = g_SequenceUnit[0];
			}
		}
		/** print P */
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (fabs(pObs->Data[iSat].P[iFre]) >= DBL_EPSILON)
			{
				msg[len++] = '1';
				msg[len++] = g_SequenceUnit[0];
			}
			else
			{
				msg[len++] = '0';
				msg[len++] = g_SequenceUnit[0];
			}
		}

		/** NULL(print refsat) */
		len += sprintf(msg + len, "%s", s_NullBuffer);

		/** print SNR */
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (pObs->Data[iSat].SNR[iFre] > 0)
			{
				len += sprintf(msg + len, "%4.1f%s", pObs->Data[iSat].SNR[iFre], g_SequenceUnit);
			}
			else
			{
				msg[len++] = g_SequenceUnit[0];
			}
		}

		/** NULL(print base SNR) */
		len += sprintf(msg + len, "%s", s_NullBuffer);

		/** doppler */
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (fabs(pObs->Data[iSat].D[iFre]) >= FLT_EPSILON)
			{
				len += sprintf(msg + len, "%.3f%s", pObs->Data[iSat].D[iFre], g_SequenceUnit);
			}
			else
			{
				msg[len++] = g_SequenceUnit[0];
			}
		}

		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (pSlipFlag[pObs->Data[iSat].Sat - 1][iFre] == 0)
			{
				msg[len++] = '0';
				msg[len++] = g_SequenceUnit[0];
			}
			else
			{
				msg[len++] = '1';
				msg[len++] = g_SequenceUnit[0];
			}
		}

		/** print azimuth */
		len += sprintf(msg + len, "%4.1f%s", (pObs->Data[iSat].Azel[0] * R2D), g_SequenceUnit);

		msg[len++] = '\n';
		fwrite(msg, len, 1, pOutFile);
		len = 0;
	}
}

void PrintEpochMP(QC_TIME_T* pTime, FILE* fpOutFile, QC_RESULT_T* pQCRes, int AntID, LOG_HEADTYPE_E eLogType)
{
	char msg[1049] = { 0 };
	int iSat, iFre, iSys, sys, prn;
	int len = 0;
	double GpsSec;
	int GpsWeek;

	if (!fpOutFile)
	{
		return;
	}
	GpsSec = QCTime2GPSTime(*pTime, &GpsWeek);

#if 0
	char tstr[32] = { 0 };
	char sPrn[4] = { 0 };

	QC_Time2Str(pTime, tstr, 1);
#endif
	
	for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
	{
		if (pQCRes->CurMPValidMask[AntID][iSat])
		{
			sys = GetQCSatSys(iSat + 1, &prn);
			iSys = QCSys2Id(sys) - 1;
			if (iSys < 0)
			{
				continue;
			}

#if 0
			QC_Satno2str(iSat + 1, sPrn);
			if (eLogType == E_LOG_HEADTYPE_1)
			{
				len += sprintf(msg + len, "%s%s%s%s", tstr, g_seq_unit, sPrn, g_seq_unit);
			}
			else if (eLogType == E_LOG_HEADTYPE_2)
			{
				len += sprintf(msg + len, "%4d%s%8.1f%s%d%s%02d%s", GpsWeek, g_seq_unit, GpsSec, g_seq_unit, iSys + 1, g_seq_unit, prn, g_seq_unit);
			}
#else
			len += sprintf(msg + len, "%4d%s%8.1f%s%d%s%02d%s", GpsWeek, g_SequenceUnit, GpsSec, g_SequenceUnit, iSys + 1, g_SequenceUnit, prn, g_SequenceUnit);
#endif

			for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
			{
				if (pQCRes->CurMP[AntID][iSat][iFre] != -9999)
				{
					len += sprintf(msg + len, "%11.8f%s", pQCRes->CurMP[AntID][iSat][iFre], g_SequenceUnit);
				}
				else if (len >= 0)
				{
					msg[len++] = g_SequenceUnit[0];
				}
			}
			msg[len++] = '\n';
			fwrite(msg, len, 1, fpOutFile);
			len = 0;
		}
	}
}

void Ucs2ToUtf8(unsigned char* pUcs2, unsigned char* pUtf8) {
	unsigned int ch;
	ch = pUcs2[0] + (pUcs2[1] << 8);
	if (ch < 0x80) {
		pUtf8[0] = ch;
		pUtf8[1] = '\0';
	}
	else if (ch < 0x800) {
		pUtf8[0] = 0xC0 | (ch >> 6);
		pUtf8[1] = 0x80 | (ch & 0x3F);
		pUtf8[2] = '\0';
}
	else {
		pUtf8[0] = 0xE0 | (ch >> 12);
		pUtf8[1] = 0x80 | ((ch >> 6) & 0x3F);
		pUtf8[2] = 0x80 | (ch & 0x3F);
		pUtf8[3] = '\0';
	}
}

void Char2WChar(const char* pChar, wchar_t* pWChar)
{
#if WIN32
	int len = MultiByteToWideChar(CP_ACP, 0, pChar, (int)strlen(pChar), NULL, 0);
	if (len > 0)
	{
		memset(pWChar, 0, sizeof(wchar_t) * ((size_t)len + 1));
		MultiByteToWideChar(0, 0, pChar, (int)strlen(pChar), pWChar, len);
		pWChar[len] = '\0';
	}
#else
	int iChar = 0;
	int iChar0 = 0;
	int iCharW = 0;
	int len = (int)strlen(pChar);
	if(len>0)
	{
		memset(pWChar, 0, sizeof(wchar_t) * ((size_t)len + 1));
		for (iChar = 0; iChar < len; iChar++)
		{
			iChar0 = iChar + 1;
			if ((pChar[iChar] < 0 || pChar[iChar] > 127) &&
				(pChar[iChar0] < 0 || pChar[iChar0] >127))
			{
				pWChar[iCharW++] = (wchar_t)((pChar[iChar] << 8) + pChar[iChar0]);
				iChar++;
			}
			else
			{	
				pWChar[iCharW++] = (wchar_t)pChar[iChar];
			}
			
		}
		pWChar[iCharW] = '\0';
	}
#endif
}

/* Update string to file */
void UpdateString2File(FILE* fpOut, char* pBuffer, int* pBuffLen)
{
	if (fpOut == NULL)
	{
		return;
	}
#ifdef WIN32
	wchar_t* buff_ch = (wchar_t*)VERIQC_CALLOC(MAX_HTML_STR_LEN, sizeof(wchar_t));
	Char2WChar(pBuffer, buff_ch);
#ifdef __aarch
	fprintf(fpOut, "%s", buff_ch);
#else
	fwprintf(fpOut, L"%s", buff_ch);
#endif
	fflush(fpOut);
	memset(pBuffer, 0, sizeof(*pBuffLen));
	
	VERIQC_FREE(buff_ch);
#else
	fwrite(pBuffer, sizeof(char), *pBuffLen, fpOut);
#endif

	*pBuffLen = 0;
}

void PrintFreTabelHtml(char* pBuffer, int* pBuffLen)
{
	int len = *pBuffLen, iFre,iSys;
	// g_SigType[QC_MAX_NUM_SYS][QC_NUM_FREQ][8]
	len += sprintf(pBuffer + len, "<table class=\"Table\"><h1 class=\"Title_0\"><a id=%s>%s</a></h1>\n", "T0", "Frequency Correspondence");
	len += sprintf(pBuffer + len, "<tr class=\"TableHeader\"><th class=\"NotAZIColum\">%s</th>\n", "System");
	len += sprintf(pBuffer + len, "<th class=\"NotAZIColum\">%s</th>\n", "Freq 1");
	len += sprintf(pBuffer + len, "<th class=\"NotAZIColum\">%s</th>\n", "Freq 2");
	len += sprintf(pBuffer + len, "<th class=\"NotAZIColum\">%s</th>\n", "Freq 3");
	len += sprintf(pBuffer + len, "<th class=\"NotAZIColum\">%s</th>\n", "Freq 4");
	len += sprintf(pBuffer + len, "<th class=\"NotAZIColum\">%s</th>\n", "Freq 5");
	len += sprintf(pBuffer + len, "<th class=\"NotAZIColum\">%s</th>\n", "Freq 6") ;
	len += sprintf(pBuffer + len, "<th class=\"NotAZIColum\">%s</th>\n", "Freq 7");
	len += sprintf(pBuffer + len, "<th class=\"NotAZIColum\">%s</th>\n", "Freq 8");
	len += sprintf(pBuffer + len, "<th class=\"NotAZIColum\">%s</th></tr>\n", "Freq 9");
	/* GPS */
	for (iSys=0; iSys< QC_MAX_NUM_SYS; iSys++)
	{
		len += sprintf(pBuffer + len, "<tr><td class = \"ColumnFirstTotalTwo\">%s:</td>\n\n", QCGetSysStr(iSys + 1));
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			len += sprintf(pBuffer + len, "<td class = \"ColumnTwoTotalTwo\">%s</td>\n", g_SigType[iSys][iFre]);
		}
		len += sprintf(pBuffer + len, "</tr>");
	}
	*pBuffLen = len;
}

void WriteQCStatisticsHtml(QC_RESULT_T* pRes, QC_OPT_T* pOpt)
{
	char outfile[256] = { 0 };
	FILE* fp_out = NULL;
	int buff_len = 0;
	QC_TIME_T DDstartTime = { 0 };
	QC_TIME_T DDendTime = { 0 };
	float Interval = 0.0;
	char tstr[30] = { 0 };
	char tstr1[30] = { 0 };
	char outfilename_r[MAX_FILE_NAME_LENGTH] = { 0 };
	char outfilename_b[MAX_FILE_NAME_LENGTH] = { 0 };
	char pureFilename[MAX_FILE_NAME_LENGTH] = { 0 };
	char* buff = (char*)VERIQC_CALLOC(MAX_HTML_STR_LEN, sizeof(char));//[MAX_HTML_STR_LEN] = { 0 };
	wchar_t* buff_ch = (wchar_t*)VERIQC_CALLOC(MAX_HTML_STR_LEN, sizeof(wchar_t));

	memset(buff_ch, 0, MAX_HTML_STR_LEN * sizeof(wchar_t));
	
	/** pGnssCfg option -> pQcReader */
	GetOutFileStr(pOpt->OutDir, pOpt->RoverFile, outfilename_r);
	GetOutFileStr(pOpt->OutDir, pOpt->BaseFile, outfilename_b);
	if (strlen(outfilename_r) > 1)
	{

	}
	else if (strlen(outfilename_b) > 1)
	{
		strcpy(outfilename_r, outfilename_b);
	}
	else
	{
		PrintfQC("\n<GNSSQC error>The file path is incorrect.\n");
		return;
	}
	
	sprintf(outfile, "%s%s", outfilename_r, "_QC.html");

	
#ifdef WIN32
	if (!(fp_out = fopen(outfile, "w,ccs=UTF-8")))
#else
	if (!(fp_out = fopen(outfile, "w")))
#endif
	{
		PrintfQC("\n<GNSSQC error>Failed to create the result file.\n");
		return;
	}

	/** html style */

	FILE* fp = fopen(g_TemplateHtmlParh, "r");
	if (!fp)
	{
		PrintfQC("\n<GNSSQC error>Template file is missing:%s\n", g_TemplateHtmlParh);
		return;
	}

	while (!feof(fp) && !ferror(fp))
	{
		strcpy(buff, "\n");
		
		fgets(buff, sizeof(buff), fp);

#ifdef WIN32
		Char2WChar(buff, buff_ch);
#ifdef __aarch
		fprintf(fp_out, "%s", buff_ch);
#else
		fwprintf(fp_out, L"%s", buff_ch);
#endif
		fflush(fp_out);
#else
		fwrite(buff, sizeof(char), strlen(buff), fp_out);
#endif
		if (strstr(buff, "</head>"))
		{
			break;
		}
	}
	
	if (fp != NULL)
	{
		fclose(fp); 
		fp = NULL;
	}

	buff_len += sprintf(buff + buff_len, "%s", "\n<body><div class=\"Container\">\n<div class=\"Left\">\n");
	buff_len += sprintf(buff + buff_len, "<p class=\"ContentsTitle_1\"><a href=%s>%s</a></p>\n","\"#T0\"","QC Summary");
	buff_len += sprintf(buff + buff_len, "<p class=\"ContentsTitle_1\"><a href=%s>%s</a></p>\n", "\"#T1\"", "QC Result");
	UpdateString2File(fp_out, buff, &buff_len);

	buff_len += sprintf(buff + buff_len, "</div><div class=\"Right\">\n<p class=\"Title\">%s</p>", "VeriQC Report");
	
	buff_len += sprintf(buff + buff_len, "<table class=\"Table\">\n<h1 class=\"Title_0\"><a id=%s>%s</a></h1>\n", "\"T0\"", "QC Summary");
	buff_len += sprintf(buff + buff_len, "<tr>");
	buff_len += sprintf(buff + buff_len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "QC Model:");//first col
	buff_len += sprintf(buff + buff_len, "<td class=\"ColumnTwoTotalTwo\">%s</td>", "None");//second col
	buff_len += sprintf(buff + buff_len, "</tr>\n");

	buff_len += sprintf(buff + buff_len, "<tr>");
	buff_len += sprintf(buff + buff_len, "<td class=\"ColumnFirstTotalTwo\">%s</td>", "Observation File:");//first col
	buff_len += sprintf(buff + buff_len, "<td class=\"ColumnTwoTotalTwo\">%s</td>", pOpt->RoverFile);//second col
	buff_len += sprintf(buff + buff_len, "</tr>\n");

	UpdateString2File(fp_out, buff, &buff_len);
	/* CHCDATA Functions */
	if (pRes->MaxEpochNum[0] > 0)
	{
		PrintObsFileInfoHtml(fp_out, pRes, pOpt, 0);
	}
	buff_len += sprintf(buff + buff_len, "%s", "</div></div></body></html>\n");
	UpdateString2File(fp_out, buff, &buff_len);

	VERIQC_FREE(buff_ch);
	buff_ch = NULL;

	VERIQC_FREE(buff);
	buff = NULL;
	fclose(fp_out);
}
