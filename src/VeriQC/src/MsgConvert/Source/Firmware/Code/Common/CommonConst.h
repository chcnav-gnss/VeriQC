/**********************************************************************//**
			 VeriQC

			 Common Module
 *-
 @file CommonConst.h
 @author CHC
 @date 2022/05/01 14:22:31

@brief common used constant defines for all module

**************************************************************************/
#ifndef _COMMON_CONST_H_
#define _COMMON_CONST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef PI
#define PI				3.1415926535897932
#endif

#ifndef PI_F
#define PI_F			3.14159265358979f
#endif

#ifndef PI_HALF_F
#define PI_HALF_F        (PI_F /2)
#endif

#define RTOD              (180.0/PI) /**< 57.2957795130 , rad to degree */
#define DTOR              (PI/180.0) /**< 0.01745329252 , degree to rad */

#define ARC_SEC_TO_RAD	  (DTOR/3600.0)        /**< arc sec to radian */

#define RTOD_F            57.2957795130f  /**<  rad to degree */
#define DTOR_F            0.01745329252f  /**< degree to rad */

/** const float degree to radian*/
#define CONST_F_DTOR(_Degree) (DTOR_F * (_Degree##f))
#define CONST_D_DTOR(_Degree) (DTOR * (_Degree))

/** const float radian to degree*/
#define CONST_F_RTOD(_Radian) (RTOD_F * (_Radian##f))
#define CONST_D_RTOD(_Radian) (RTOD * (_Radian))

#ifdef __cplusplus
}
#endif
#endif
