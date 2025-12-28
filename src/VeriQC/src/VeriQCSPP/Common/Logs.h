/*************************************************************************//**
           VeriQC
         SPP Module
 *-
@file   Logs.h
@author CHC
@date   5 September 2022
@brief  debug log print
*****************************************************************************/
#ifndef _LOGS_H_
#define _LOGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VERIQC_MAX_LABEL        2       /**< max label */
#define LABEL_VERIQC            0
#define LABEL_GNSS              1       /**< log label of GNSS */

#ifdef _WIN32
#define VERIQC_FILENAME(x)         strrchr(x, '\\')?strrchr(x, '\\')+1:x   /**< get file name of a full path */
#define LINE_FEED       "\n"        /**< line feed character */
#else 
#define VERIQC_FILENAME(x)         strrchr(x, '/')?strrchr(x, '/')+1:x     /**< get file name of a full path */
#define LINE_FEED       "\r\n"      /**< line feed character */
#endif

#define VERIQC_LOG_LEVEL_OFF    0   /**< level close(close log if log level limit) */
#define VERIQC_LOG_LEVEL_FATAL  1   /**< fatal error, broken system or library */
#define VERIQC_LOG_LEVEL_ERROR  2   /**< error, maybe lead to fatal */
#define VERIQC_LOG_LEVEL_WARN   3   /**< warning, library may not work as expected */
#define VERIQC_LOG_LEVEL_INFO   4   /**< info, message need to notice */
#define VERIQC_LOG_LEVEL_DEBUG  5   /**< simple message for debug */
#define VERIQC_LOG_LEVEL_TRACE  6   /**< details for debug */

/* show time format(stfmt) */
typedef enum {
    VERIQC_LOG_STFMT_SEC = 0,
    VERIQC_LOG_STFMT_HHMMSS,
}VERIQC_LOG_STFMT_E;

struct _QC_TIME_T;
typedef void(*VERIQC_OUTPUT_FUNC)(const char *pMsg);
int VeriQCLogOpen(int Label, VERIQC_OUTPUT_FUNC StdoutFunc, 
    VERIQC_OUTPUT_FUNC SaveLogFunc);
int VeriQCLogClose(int Label);
int VeriQCLogSetTime(int Label, const struct _QC_TIME_T* pTime);
int VeriQCLogSetStreamFormat(int Label, VERIQC_LOG_STFMT_E StreamFormat);
int VeriQCLogSetLevelFile(int Label, int Level);
int VeriQCLogSetLevelStdout(int Label, int Level);
int VeriQCLogPrintfp(int Label, int Level, const char *pFormat, ...);

/**
 * @brief ins log functions/macro
 * @author  CHC
 * @date    2020/04/15
 * @param[in]   label   label of current log
 * @param[in]   format  format of current log(same as printf())
 */
#define VERIQC_LOG_FATAL(Label, Format, ...)  (VeriQCLogPrintfp(Label, VERIQC_LOG_LEVEL_FATAL, \
    "FATAL[%s] " Format LINE_FEED, __FUNCTION__, ##__VA_ARGS__))
#define VERIQC_LOG_ERROR(Label, Format, ...)  (VeriQCLogPrintfp(Label, VERIQC_LOG_LEVEL_ERROR, \
    "ERROR[%s] " Format LINE_FEED, __FUNCTION__, ##__VA_ARGS__))
#define VERIQC_LOG_WARN(Label, Format,  ...)  (VeriQCLogPrintfp(Label, VERIQC_LOG_LEVEL_WARN,  \
    "WARN[%s] " Format  LINE_FEED, __FUNCTION__, ##__VA_ARGS__))
#define VERIQC_LOG_INFO(Label, Format,  ...)  (VeriQCLogPrintfp(Label, VERIQC_LOG_LEVEL_INFO,  \
    "INFO[%s] " Format LINE_FEED, __FUNCTION__, ##__VA_ARGS__))
#define VERIQC_LOG_DEBUG(Label, Format, ...)  (VeriQCLogPrintfp(Label, VERIQC_LOG_LEVEL_DEBUG, \
    "DEBUG[%s] " Format LINE_FEED, __FUNCTION__, ##__VA_ARGS__))

/* make debug/trace level log output only available under debug mode  */
#if (defined _DEBUG || defined FOR_M620 || defined DEBUGAR)
#define VERIQC_LOG_TRACE(Label, Format, ...)  (VeriQCLogPrintfp(Label, VERIQC_LOG_LEVEL_TRACE, \
    "TRACE[%s] " Format LINE_FEED, __FUNCTION__, ##__VA_ARGS__))
#else
#define VERIQC_LOG_TRACE(Label, Format, ...)
#endif  /* _DEBUG */

#define VERIQC_LOG_ON(Label, ID)  (VeriQCLogPrintfp(Label, VERIQC_LOG_LEVEL_INFO, \
    "TAG (%s) %i" LINE_FEED, __FUNCTION__, ID))

#define GNSS_LOG_FATAL(Format, ...) VERIQC_LOG_FATAL(LABEL_GNSS, Format, ##__VA_ARGS__)
#define GNSS_LOG_ERROR(Format, ...) VERIQC_LOG_ERROR(LABEL_GNSS, Format, ##__VA_ARGS__)
#define GNSS_LOG_WARN(Format, ...)  VERIQC_LOG_WARN(LABEL_GNSS, Format, ##__VA_ARGS__)
#define GNSS_LOG_INFO(Format, ...)  VERIQC_LOG_INFO(LABEL_GNSS, Format, ##__VA_ARGS__)
#define GNSS_LOG_DEBUG(Format, ...) VERIQC_LOG_DEBUG(LABEL_GNSS, Format, ##__VA_ARGS__)
#define GNSS_LOG_TRACE(Format, ...) VERIQC_LOG_TRACE(LABEL_GNSS, Format, ##__VA_ARGS__)
#define GNSS_LOG_ON(ID)             VERIQC_LOG_ON(LABEL_GNSS, ID)

#ifdef __cplusplus
}
#endif

#endif /* VERIQC_LOG_H_ */
