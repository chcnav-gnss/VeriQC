#include "Logs.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>


#include "ProcInterface.h"
#include "VeriQCSPP.h"

#define VERIQC_LOG_PATH_LEN     512         /**< max length of log path */
#define VERIQC_LOG_BUFF_LEN     1024        /**< max length of log message */
typedef struct _VERIQC_LOG_T
{
    VERIQC_OUTPUT_FUNC SaveLogFunc;    /**< save log to file function */
    VERIQC_OUTPUT_FUNC StdoutFunc;     /**< print to stdout function */
    unsigned char LevelStdout;         /**< log to stdout level limit of labels */
    unsigned char LevelFile;           /**< log to file level limit of labels */
    int Week;                          /**< log week */
    double Time;                       /**< log second time */
    double Epoch[6];                   /**< epoch{year,month,day,hour,min,sec} */
    int IsOpen;                        /**< log opened or not */
    int IsFatal;                       /**< log have fatal or not */
    VERIQC_LOG_STFMT_E StreamFormat;   /**< format to show time */
}VERIQC_LOG_T;

static VERIQC_LOG_T s_Log[VERIQC_MAX_LABEL] = {0};     /**< default log struct */

/**
 * @brief set log time of label
 * @author CHC
 * @date 2020/04/14
 * @param[in]   Label   input label
 * @param[in]   pTime   input time
 * @return status(0: OK)
 */
extern int VeriQCLogSetTime(int Label, const QC_TIME_T *pTime)
{
    if(!s_Log[Label].IsOpen) VeriQCLogOpen(Label, NULL, NULL);

    s_Log[Label].Time = QCTime2GPSTime(*pTime, &s_Log[Label].Week);
    QCTime2Epoch(*pTime, s_Log[Label].Epoch);
    return 0;
}

/**
 * @brief get log time of label
 * @author CHC
 * @date 2020/05/07
 * @return time of ins log(double)
 */
extern double VeriQCLogGetTime(int Label)
{
    return s_Log[Label].Time;
}

/**
 * @brief set show time format for label
 * @author CHC
 * @date 2022/01/20
 * @param[in]   Label           log label(optional: 0~VERIQC_MAX_LABEL)
 * @param[in]   StreamFormat    show time format
 * @return status(0: OK)
 */
extern int VeriQCLogSetStreamFormat(int Label, VERIQC_LOG_STFMT_E StreamFormat)
{
    if(!s_Log[Label].IsOpen) VeriQCLogOpen(Label, NULL, NULL);

    s_Log[Label].StreamFormat = StreamFormat;
    return 0;
}

/**
 * @brief set log level limit of both stdout and file, log level large 
 *  large than level limit will be disabled.
 * @author CHC
 * @date 2020/04/14
 * @param[in]   Label   label to limit (optional: 0~VERIQC_MAX_LABEL)
 * @param[in]   Level   level of label to limit(optional: 0~6)
 * @return status(0: OK)
 */
extern int VeriQCLogSetLevel(int Label, int Level)
{
    int ret1 = VeriQCLogSetLevelFile(Label, Level);
    int ret2 = VeriQCLogSetLevelStdout(Label, Level);
    return ret1 + ret2;
}

/**
 * @brief set log level limit of  file, log level large large than level limit
 *      will be disabled.
 * @author CHC
 * @date 2020/04/14
 * @param[in]   Label   label to limit (optional: 0~INS_MAXLABEL)
 * @param[in]   Level   level of label to limit(optional: 0~6)
 * @return status(0: OK)
 */
extern int VeriQCLogSetLevelFile(int Label, int Level)
{
    if(!s_Log[Label].IsOpen) VeriQCLogOpen(Label, NULL, NULL);
    if(Label >= VERIQC_MAX_LABEL) return -1;
    s_Log[Label].LevelFile = (unsigned char)Level;
    return 0;
}

/**
 * @brief set log level limit of  stdout, log level large large than level limit
 *      will be disabled.
 * @author CHC
 * @date 2020/04/14
 * @param[in]   Label   label to limit (optional: 0~INS_MAXLABEL)
 * @param[in]   Level   level of label to limit(optional: 0~6)
 * @return status(0: OK)
 */
extern int VeriQCLogSetLevelStdout(int Label, int Level)
{
    if(!s_Log[Label].IsOpen) VeriQCLogOpen(Label, NULL, NULL);
    if(Label >= VERIQC_MAX_LABEL) return -1;
    s_Log[Label].LevelStdout = (unsigned char)Level;
    return 0;
}

/**
 * @brief log message of a label to stdout and file according to level limit of 
        labels, format is same as standard library printf() function
        this function will add a time tag automatically
 * @author CHC
 * @date 2020/04/14
 * @param[in]   Label   label of log message
 * @param[in]   Level   level of log message
 * @param[in]   pFormat format of log message(same as printf())
 * @return status(0: OK)
 * @note single message length is limited to INS_MAX_LEN_LOG(1024 default), make
 *  sure short than the limit.
 */
extern int VeriQCLogPrintfp(int Label, int Level, const char *pFormat, ...)
{
    if(!s_Log[Label].IsOpen) VeriQCLogOpen(Label, NULL, NULL);
    va_list ap;
    VERIQC_LOG_T *plog = &s_Log[Label];

    char buff[VERIQC_LOG_BUFF_LEN];
    int len = 0;


    if((Level <= plog->LevelStdout && plog->StdoutFunc != NULL) || 
       (Level <= plog->LevelFile && plog->SaveLogFunc != NULL))
    {
        switch(plog->StreamFormat)
        {
        case VERIQC_LOG_STFMT_SEC:
            len = snprintf(buff, VERIQC_LOG_BUFF_LEN, "[%010.3f]", plog->Time);
            break;
        case VERIQC_LOG_STFMT_HHMMSS:
            len = snprintf(buff, VERIQC_LOG_BUFF_LEN, "[%02.0f:%02.0f:%0*.*f]", 
                plog->Epoch[3], plog->Epoch[4], 6, 3, plog->Epoch[5]);
            break;
        default:
            len = snprintf(buff, VERIQC_LOG_BUFF_LEN, "[%010.3f]", plog->Time);
            break;
        }

        va_start(ap, pFormat); 
        vsnprintf(buff+len, VERIQC_LOG_BUFF_LEN - len, pFormat, ap); 
        va_end(ap);
        buff[VERIQC_LOG_BUFF_LEN-1] = '\0';   /* keep string safe */

        if(Level <= plog->LevelStdout && plog->StdoutFunc != NULL)
            plog->StdoutFunc(buff);
        if(Level <= plog->LevelFile && plog->SaveLogFunc != NULL)
            plog->SaveLogFunc(buff);
    }

    if(Level == VERIQC_LOG_LEVEL_FATAL) s_Log[Label].IsFatal = 1;

    return 0;
}

/**
 * @brief log message of a label to stdout and file according to level limit of 
 *      labels, format is same as standard library printf() function
 *      do NOT print any addtional message(e.g. log time)
 * @author CHC
 * @date 2020/04/14
 * @param[in]   Label   label of log message
 * @param[in]   Level   level of log message
 * @param[in]   pFormat format of log message(same as printf())
 * @return status(0: OK)
 * @note single message length is limited to 1024 default, make
 *  sure short than the limit.
 */
extern int VeriQCLogPrintf(int Label, int Level, const char *pFormat, ...)
{
    if(!s_Log[Label].IsOpen) VeriQCLogOpen(Label, NULL, NULL);
    va_list ap;
    VERIQC_LOG_T *plog = &s_Log[Label];

    char buff[VERIQC_LOG_BUFF_LEN];

    if((Level <= plog->LevelStdout && plog->StdoutFunc != NULL) || 
       (Level <= plog->LevelFile && plog->SaveLogFunc != NULL))
    {
        va_start(ap, pFormat); 
        vsnprintf(buff, VERIQC_LOG_BUFF_LEN, pFormat, ap); 
        va_end(ap);
        buff[VERIQC_LOG_BUFF_LEN-1] = '\0';   /* keep string safe */

        if(Level <= plog->LevelStdout && plog->StdoutFunc != NULL)
            plog->StdoutFunc(buff);
        if(Level <= plog->LevelFile && plog->SaveLogFunc != NULL)
            plog->SaveLogFunc(buff);
    }

    if(Level == VERIQC_LOG_LEVEL_FATAL) s_Log[Label].IsFatal = 1;

    return 0;
}

/**
 * @brief output message to stdout(printf),this function is use to 
 * @author CHC
 * @date 2021/02/02
 * @param[in]   pMsg     message to output
 */
static void OutputStdout(const char *pMsg) { printf("%s", pMsg); }

/**
 * @brief open log file and initial log struct, set default stdout level limit 
 *      to VERIQC_LOG_LEVEL_INFO, default file level limit to VERIQC_LOG_LEVEL_TRACE
 * @author CHC
 * @date 2020/04/14
 * @param[in]   Label           ins log label(optional: 0~VERIQC_MAX_LABEL)
 * @param[in]   StdoutFunc      log to stdout function(optional: NULL for printf)
 * @param[in]   SaveLogFunc     log to file function(optional: NULL)
 * @return status(0: OK) 
 */
extern int VeriQCLogOpen(int Label, VERIQC_OUTPUT_FUNC StdoutFunc, 
    VERIQC_OUTPUT_FUNC SaveLogFunc)
{
    if(!s_Log[Label].IsOpen)
    {
        s_Log[Label].IsOpen = 1;
        s_Log[Label].LevelStdout = VERIQC_LOG_LEVEL_INFO;
        s_Log[Label].LevelFile = VERIQC_LOG_LEVEL_TRACE;
        s_Log[Label].StreamFormat = VERIQC_LOG_STFMT_SEC;
    }
    if(StdoutFunc == NULL)
        s_Log[Label].StdoutFunc = OutputStdout;
    else
        s_Log[Label].StdoutFunc = StdoutFunc;
    s_Log[Label].SaveLogFunc = SaveLogFunc;
    return 0;
}

/**
 * @brief reset and close ins log
 * @author CHC
 * @date 2020/04/14
 * @param[in]   Label   ins log label
 * @return status(0: OK)
 */
extern int VeriQCLogClose(int Label)
{
    memset(&s_Log[Label], 0, sizeof(VERIQC_LOG_T));
    return 0;
}
