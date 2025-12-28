/**********************************************************************//**
		VeriQC

		Common Module
*-
@file   GNSSConstants.h
@author CHC
@date   2022/10/12
@brief  define constants used in VeriQC

**************************************************************************/
#ifndef _GNSS_CONSTANTS_H_
#define _GNSS_CONSTANTS_H_

#ifndef PI
#define PI				3.1415926535897932
#endif

#define WGS_AXIS_A			6378137.0				/**< A - WGS-84 earth's semi major axis*/
#define	WGS_AXIS_B			6356752.3142451795		/**< B - WGS-84 earth's semi minor axis*/
#define WGS_E1_SQR			0.006694379990141317	/**< (A/B)^2-1, 1st numerical eccentricity*/
#define WGS_E2_SQR			0.006739496742276435	/**< 1-(B/A)^2, 2nd numerical eccentricity*/
#define WGS_SQRT_GM			19964981.8432173887	    /**< square root of GM*/
#define WGS_OMEGDOTE		7.2921151467e-5			/**< earth rotate rate*/

#define CGCS2000_SQRT_GM	19964980.3856652962	    /**< square root of GM */
#define CGCS2000_OMEGDOTE	7.292115e-5				/**< earth rotate rate */

#define FE_WGS84            0.00335281066474748     /* earth flattening (WGS84) */
#define RE_WGS84            6378137.0               /* earth semimajor axis (WGS84) (m) */

#define CLIGHT_DS           3.3356409519815204E-9   /* (1.0/CLIGHT) */
#define OMGE				7.2921151467E-5 	    /**< earth angular velocity (IS-GPS) (rad/s) */

#endif /**< _GNSS_CONSTANTS_H_ */
