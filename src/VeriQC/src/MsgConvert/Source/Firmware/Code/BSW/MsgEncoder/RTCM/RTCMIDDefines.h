/**********************************************************************//**
		VeriQC

		RTCM Encoder Module
*-
@file   RTCMIDDefine.h
@author CHC
@date   2023/03/02
@brief

**************************************************************************/
#ifndef _RTCM_ID_DEFINES_H_
#define _RTCM_ID_DEFINES_H_

#ifdef __cplusplus
extern "C"
{
#endif

/** message id, keep consistent with NV*/
#define RTCM_MSG_ID_1001        772 /**< GPS L1-only observables, basic */
#define RTCM_MSG_ID_1002        774 /**< GPS L1-only observables, extended */
#define RTCM_MSG_ID_1003        776 /**< GPS L1/L2 basic observables, basic */
#define RTCM_MSG_ID_1004        770 /**< GPS L1/L2 basic observables, extended */
#define RTCM_MSG_ID_1009        885 /**< GLONASS L1-only observables, basic */
#define RTCM_MSG_ID_1010        887 /**< GLONASS L1-only observables, extended */
#define RTCM_MSG_ID_1011        889 /**< GLONASS L1/L2 basic observables, basic */
#define RTCM_MSG_ID_1012        891 /**< GLONASS L1/L2 basic observables, extended */

#define RTCM_MSG_ID_1071        1472 /**< GPS MSM1 */
#define RTCM_MSG_ID_1072        1473 /**< GPS MSM2 */
#define RTCM_MSG_ID_1073        1474 /**< GPS MSM3 */
#define RTCM_MSG_ID_1074        1475 /**< GPS MSM4 */
#define RTCM_MSG_ID_1075        1476 /**< GPS MSM5 */
#define RTCM_MSG_ID_1076        1477 /**< GPS MSM6 */
#define RTCM_MSG_ID_1077        1478 /**< GPS MSM7 */

#define RTCM_MSG_ID_1081        1479 /**< GLONASS MSM1 */
#define RTCM_MSG_ID_1082        1480 /**< GLONASS MSM2 */
#define RTCM_MSG_ID_1083        1481 /**< GLONASS MSM3 */
#define RTCM_MSG_ID_1084        1482 /**< GLONASS MSM4 */
#define RTCM_MSG_ID_1085        1483 /**< GLONASS MSM5 */
#define RTCM_MSG_ID_1086        1484 /**< GLONASS MSM6 */
#define RTCM_MSG_ID_1087        1485 /**< GLONASS MSM7 */

#define RTCM_MSG_ID_1091        1486 /**< Galileo MSM1 */
#define RTCM_MSG_ID_1092        1487 /**< Galileo MSM2 */
#define RTCM_MSG_ID_1093        1488 /**< Galileo MSM3 */
#define RTCM_MSG_ID_1094        1489 /**< Galileo MSM4 */
#define RTCM_MSG_ID_1095        1490 /**< Galileo MSM5 */
#define RTCM_MSG_ID_1096        1491 /**< Galileo MSM6 */
#define RTCM_MSG_ID_1097        1492 /**< Galileo MSM7 */

#define RTCM_MSG_ID_1101        1493 /**< SBAS MSM1 */
#define RTCM_MSG_ID_1102        1494 /**< SBAS MSM2 */
#define RTCM_MSG_ID_1103        1495 /**< SBAS MSM3 */
#define RTCM_MSG_ID_1104        1496 /**< SBAS MSM4 */
#define RTCM_MSG_ID_1105        1497 /**< SBAS MSM5 */
#define RTCM_MSG_ID_1106        1498 /**< SBAS MSM6 */
#define RTCM_MSG_ID_1107        1499 /**< SBAS MSM7 */

#define RTCM_MSG_ID_1111        1648 /**< QZSS MSM1 */
#define RTCM_MSG_ID_1112        1649 /**< QZSS MSM2 */
#define RTCM_MSG_ID_1113        1650 /**< QZSS MSM3 */
#define RTCM_MSG_ID_1114        1651 /**< QZSS MSM4 */
#define RTCM_MSG_ID_1115        1652 /**< QZSS MSM5 */
#define RTCM_MSG_ID_1116        1653 /**< QZSS MSM6 */
#define RTCM_MSG_ID_1117        1654 /**< QZSS MSM7 */

#define RTCM_MSG_ID_1121        1592 /**< BeiDou MSM1 */
#define RTCM_MSG_ID_1122        1593 /**< BeiDou MSM2 */
#define RTCM_MSG_ID_1123        1594 /**< BeiDou MSM3 */
#define RTCM_MSG_ID_1124        1595 /**< BeiDou MSM4 */
#define RTCM_MSG_ID_1125        1596 /**< BeiDou MSM5 */
#define RTCM_MSG_ID_1126        1597 /**< BeiDou MSM6 */
#define RTCM_MSG_ID_1127        1598 /**< BeiDou MSM7 */

#define RTCM_MSG_ID_1131        1792 /**< NAVIC MSM1 */
#define RTCM_MSG_ID_1132        1793 /**< NAVIC MSM2 */
#define RTCM_MSG_ID_1133        1794 /**< NAVIC MSM3 */
#define RTCM_MSG_ID_1134        1795 /**< NAVIC MSM4 */
#define RTCM_MSG_ID_1135        1796 /**< NAVIC MSM5 */
#define RTCM_MSG_ID_1136        1797 /**< NAVIC MSM6 */
#define RTCM_MSG_ID_1137        1798 /**< NAVIC MSM7 */

#define RTCM_MSG_ID_1005        765 /**< Stationary RTK Base Station Antenna Reference Point (ARP) */
#define RTCM_MSG_ID_1006        768 /**< Stationary RTK Base Station ARP with Antenna Height */
#define RTCM_MSG_ID_1007        852 /**< Extended Antenna Descriptor and Setup Information */
#define RTCM_MSG_ID_1008        854 /**< Extended Antenna Reference Station Description and Serial Number */
#define RTCM_MSG_ID_1033        1097 /**< Receiver and antenna descriptors */


#define RTCM_MSG_ID_1019        893 /**< GPS Ephemeris */
#define RTCM_MSG_ID_1020        895 /**< GLONASS Ephemeris */
#define RTCM_MSG_ID_1041        2179 /**< NAVIC Ephemeris */
#define RTCM_MSG_ID_1042        2171 /**< BeiDou Ephemeris */
#define RTCM_MSG_ID_1044        2177 /**< QZSS Ephemeris */
#define RTCM_MSG_ID_1045        2173 /**< Galileo F/NAV Ephemeris */
#define RTCM_MSG_ID_1046        2175 /**< Galileo I/NAV Ephemeris */


#define RTCM_MSG_ID_1230        1655 /**< GLONASS L1 and L2 Code-Phase Biases */


#define RTCM_MSG_ID_4063        4063 /**< huace extend rtcm command */
#define RTCM_MSG_ID_MSM4        4064 /**< MSM4 sets(RTCM1074/RTCM1084/RTCM1094/RTCM1114/RTCM1124/RTCM1134) */
#define RTCM_MSG_ID_MSM5        4065 /**< MSM5 sets(RTCM1075/RTCM1085/RTCM1095/RTCM1115/RTCM1125/RTCM1135) */
#define RTCM_MSG_ID_MSM7        4067 /**< MSM7 sets(RTCM1077/RTCM1087/RTCM1097/RTCM1117/RTCM1127/RTCM1137) */
#define RTCM_MSG_ID_EPH         4068 /**< EPH sets(RTCM1019/RTCM1020/RTCM1041/RTCM1042/RTCM1044/RTCM1045/RTCM1046) */
#define RTCM_MSG_ID_RTCMV3      4069 /**< Note: used for moving station to request observable messages, station and antenna message in one go */

#define RTCM_MSG_ID_COMPACTRTCMMSM3           4070 /**< Note: used for compact rtcm3 msm4 for radio transport after delete satellites */
#define RTCM_MSG_ID_COMPACTRTCMMSM4           4071 /**< Note: used for compact rtcm3 msm4 for radio transport after delete satellites */


/** message index id, use when RTCM encoding*/
#define RTCM_MSG_INDEX_1071        0                         /**< GPS MSM1 */
#define RTCM_MSG_INDEX_1081        (RTCM_MSG_INDEX_1071 + 1) /**< GLONASS MSM1 */
#define RTCM_MSG_INDEX_1091        (RTCM_MSG_INDEX_1081 + 1) /**< Galileo MSM1 */
#define RTCM_MSG_INDEX_1101        (RTCM_MSG_INDEX_1091 + 1) /**< SBAS MSM1 */
#define RTCM_MSG_INDEX_1111        (RTCM_MSG_INDEX_1101 + 1) /**< QZSS MSM1 */
#define RTCM_MSG_INDEX_1121        (RTCM_MSG_INDEX_1111 + 1) /**< BeiDou MSM1 */
#define RTCM_MSG_INDEX_1131        (RTCM_MSG_INDEX_1121 + 1) /**< NAVIC MSM1 */

#define RTCM_MSG_INDEX_1072        (RTCM_MSG_INDEX_1131 + 1) /**< GPS MSM2 */
#define RTCM_MSG_INDEX_1082        (RTCM_MSG_INDEX_1072 + 1) /**< GLONASS MSM2 */
#define RTCM_MSG_INDEX_1092        (RTCM_MSG_INDEX_1082 + 1) /**< Galileo MSM2 */
#define RTCM_MSG_INDEX_1102        (RTCM_MSG_INDEX_1092 + 1) /**< SBAS MSM2 */
#define RTCM_MSG_INDEX_1112        (RTCM_MSG_INDEX_1102 + 1) /**< QZSS MSM2 */
#define RTCM_MSG_INDEX_1122        (RTCM_MSG_INDEX_1112 + 1) /**< BeiDou MSM2 */
#define RTCM_MSG_INDEX_1132        (RTCM_MSG_INDEX_1122 + 1) /**< NAVIC MSM2 */

#define RTCM_MSG_INDEX_1073        (RTCM_MSG_INDEX_1132 + 1) /**< GPS MSM3 */
#define RTCM_MSG_INDEX_1083        (RTCM_MSG_INDEX_1073 + 1) /**< GLONASS MSM3 */
#define RTCM_MSG_INDEX_1093        (RTCM_MSG_INDEX_1083 + 1) /**< Galileo MSM3 */
#define RTCM_MSG_INDEX_1103        (RTCM_MSG_INDEX_1093 + 1) /**< SBAS MSM3 */
#define RTCM_MSG_INDEX_1113        (RTCM_MSG_INDEX_1103 + 1) /**< QZSS MSM3 */
#define RTCM_MSG_INDEX_1123        (RTCM_MSG_INDEX_1113 + 1) /**< BeiDou MSM3 */
#define RTCM_MSG_INDEX_1133        (RTCM_MSG_INDEX_1123 + 1) /**< NAVIC MSM3 */

#define RTCM_MSG_INDEX_1074        (RTCM_MSG_INDEX_1133 + 1) /**< GPS MSM4 */
#define RTCM_MSG_INDEX_1084        (RTCM_MSG_INDEX_1074 + 1) /**< GLONASS MSM4 */
#define RTCM_MSG_INDEX_1094        (RTCM_MSG_INDEX_1084 + 1) /**< Galileo MSM4 */
#define RTCM_MSG_INDEX_1104        (RTCM_MSG_INDEX_1094 + 1) /**< SBAS MSM4 */
#define RTCM_MSG_INDEX_1114        (RTCM_MSG_INDEX_1104 + 1) /**< QZSS MSM4 */
#define RTCM_MSG_INDEX_1124        (RTCM_MSG_INDEX_1114 + 1) /**< BeiDou MSM4 */
#define RTCM_MSG_INDEX_1134        (RTCM_MSG_INDEX_1124 + 1) /**< NAVIC MSM4 */

#define RTCM_MSG_INDEX_1075        (RTCM_MSG_INDEX_1134 + 1) /**< GPS MSM5 */
#define RTCM_MSG_INDEX_1085        (RTCM_MSG_INDEX_1075 + 1) /**< GLONASS MSM5 */
#define RTCM_MSG_INDEX_1095        (RTCM_MSG_INDEX_1085 + 1) /**< Galileo MSM5 */
#define RTCM_MSG_INDEX_1105        (RTCM_MSG_INDEX_1095 + 1) /**< SBAS MSM5 */
#define RTCM_MSG_INDEX_1115        (RTCM_MSG_INDEX_1105 + 1) /**< QZSS MSM5 */
#define RTCM_MSG_INDEX_1125        (RTCM_MSG_INDEX_1115 + 1) /**< BeiDou MSM5 */
#define RTCM_MSG_INDEX_1135        (RTCM_MSG_INDEX_1125 + 1) /**< NAVIC MSM5 */

#define RTCM_MSG_INDEX_1076        (RTCM_MSG_INDEX_1135 + 1) /**< GPS MSM6 */
#define RTCM_MSG_INDEX_1086        (RTCM_MSG_INDEX_1076 + 1) /**< GLONASS MSM6 */
#define RTCM_MSG_INDEX_1096        (RTCM_MSG_INDEX_1086 + 1) /**< Galileo MSM6 */
#define RTCM_MSG_INDEX_1106        (RTCM_MSG_INDEX_1096 + 1) /**< SBAS MSM6 */
#define RTCM_MSG_INDEX_1116        (RTCM_MSG_INDEX_1106 + 1) /**< QZSS MSM6 */
#define RTCM_MSG_INDEX_1126        (RTCM_MSG_INDEX_1116 + 1) /**< BeiDou MSM6 */
#define RTCM_MSG_INDEX_1136        (RTCM_MSG_INDEX_1126 + 1) /**< NAVIC MSM6 */

#define RTCM_MSG_INDEX_1077        (RTCM_MSG_INDEX_1136 + 1) /**< GPS MSM7 */
#define RTCM_MSG_INDEX_1087        (RTCM_MSG_INDEX_1077 + 1) /**< GLONASS MSM7 */
#define RTCM_MSG_INDEX_1097        (RTCM_MSG_INDEX_1087 + 1) /**< Galileo MSM7 */
#define RTCM_MSG_INDEX_1107        (RTCM_MSG_INDEX_1097 + 1) /**< SBAS MSM7 */
#define RTCM_MSG_INDEX_1117        (RTCM_MSG_INDEX_1107 + 1) /**< QZSS MSM7 */
#define RTCM_MSG_INDEX_1127        (RTCM_MSG_INDEX_1117 + 1) /**< BeiDou MSM7 */
#define RTCM_MSG_INDEX_1137        (RTCM_MSG_INDEX_1127 + 1) /**< NAVIC MSM7 */

#define RTCM_MSG_INDEX_1001        (RTCM_MSG_INDEX_1137 + 1) /**< GPS L1-only observables, basic */
#define RTCM_MSG_INDEX_1002        (RTCM_MSG_INDEX_1001 + 1) /**< GPS L1-only observables, extended */
#define RTCM_MSG_INDEX_1003        (RTCM_MSG_INDEX_1002 + 1) /**< GPS L1/L2 basic observables, basic */
#define RTCM_MSG_INDEX_1004        (RTCM_MSG_INDEX_1003 + 1) /**< GPS L1/L2 basic observables, extended */
#define RTCM_MSG_INDEX_1009        (RTCM_MSG_INDEX_1004 + 1) /**< GLONASS L1-only observables, basic */
#define RTCM_MSG_INDEX_1010        (RTCM_MSG_INDEX_1009 + 1) /**< GLONASS L1-only observables, extended */
#define RTCM_MSG_INDEX_1011        (RTCM_MSG_INDEX_1010 + 1) /**< GLONASS L1/L2 basic observables, basic */
#define RTCM_MSG_INDEX_1012        (RTCM_MSG_INDEX_1011 + 1) /**< GLONASS L1/L2 basic observables, extended */

#define RTCM_MSG_INDEX_1005        (RTCM_MSG_INDEX_1012 + 1) /**< Stationary RTK Base Station Antenna Reference Point (ARP) */
#define RTCM_MSG_INDEX_1006        (RTCM_MSG_INDEX_1005 + 1) /**< Stationary RTK Base Station ARP with Antenna Height */
#define RTCM_MSG_INDEX_1007        (RTCM_MSG_INDEX_1006 + 1) /**< Extended Antenna Descriptor and Setup Information */
#define RTCM_MSG_INDEX_1008        (RTCM_MSG_INDEX_1007 + 1) /**< Extended Antenna Reference Station Description and Serial Number */
#define RTCM_MSG_INDEX_1033        (RTCM_MSG_INDEX_1008 + 1) /**< Receiver and antenna descriptors */

#define RTCM_MSG_INDEX_1019        (RTCM_MSG_INDEX_1033 + 1) /**< GPS Ephemeris */
#define RTCM_MSG_INDEX_1020        (RTCM_MSG_INDEX_1019 + 1) /**< GLONASS Ephemeris */
#define RTCM_MSG_INDEX_1041        (RTCM_MSG_INDEX_1020 + 1) /**< NAVIC Ephemeris */
#define RTCM_MSG_INDEX_1042        (RTCM_MSG_INDEX_1041 + 1) /**< BeiDou Ephemeris */
#define RTCM_MSG_INDEX_1044        (RTCM_MSG_INDEX_1042 + 1) /**< QZSS Ephemeris */
#define RTCM_MSG_INDEX_1045        (RTCM_MSG_INDEX_1044 + 1) /**< Galileo F/NAV Ephemeris */
#define RTCM_MSG_INDEX_1046        (RTCM_MSG_INDEX_1045 + 1) /**< Galileo I/NAV Ephemeris */

#define RTCM_MSG_INDEX_1230        (RTCM_MSG_INDEX_1046 + 1) /**< GLONASS L1 and L2 Code-Phase Biases */

#define RTCM_MSG_INDEX_4063        (RTCM_MSG_INDEX_1230 + 1) /**< huace extend rtcm command */



#define RTCM_MSG_INDEX_MAX         (RTCM_MSG_INDEX_4063 + 1)



#define COMPACT_RTCM_MSG_INDEX_MSM3        (0)
#define COMPACT_RTCM_MSG_INDEX_MSM4        (COMPACT_RTCM_MSG_INDEX_MSM3 + 1)

#define COMPACT_RTCM_MSG_INDEX_MAX         (COMPACT_RTCM_MSG_INDEX_MSM4 + 1)


#ifdef __cplusplus
}
#endif

#endif

