#include "DataQC.h" 
#include <stdio.h> 
#define QC_VERSION    100   
static char g_QCVersionStr[258] = { 0 }; 
static char g_QCBranchStr[258] = { 0 }; 
int GetQCVersion(void) 
{ 
	return QC_VERSION; 
} 
char* GetQCVersionStr(void) 
{ 
	sprintf(g_QCVersionStr, "V1.0_%s %s-%s", "c2f1d93", __DATE__, __TIME__); 
	return g_QCVersionStr; 
} 
char* GetQCBranchStr(void) 
{ 
	sprintf(g_QCBranchStr, "Branch Name = %s", "master"); 
	return g_QCBranchStr; 
} 
