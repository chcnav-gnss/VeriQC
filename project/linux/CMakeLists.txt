cmake_minimum_required(VERSION 3.10)


project(VeriQC)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -std=gnu99 -g -fPIC -Wno-unused-variable -Wno-write-strings -D__linux -DMSG_CONVERT -D_HW_CMODEL_ -DVERIQC_SURVEY")
set(PROP_MSG_DECODER_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../../proj/PropMsgDecoder/Lib/Linux/libPropMsgDecoder.a)


set(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../proj/VeriQC/src)
set(COMMONDIR ${SRCDIR}/Common)
set(INTERFACEDIR ${SRCDIR}/Interface)
set(MSGCONVERTDIR ${SRCDIR}/MsgConvert/Source/Firmware/Code/BSW/MsgDecoder/RTCM)
set(MSGCONVERT_UNITTESTDIR ${MSGCONVERTDIR}/UnitTest/MsgConvert/Code)
set(MSGCONVERT_COMMONDIR ${MSGCONVERT_UNITTESTDIR}/Common)
set(MSGCONVERT_DECODERDIR ${MSGCONVERT_UNITTESTDIR}/Decoder)
set(MSGCONVERT_ENCODERDIR ${MSGCONVERT_UNITTESTDIR}/Encoder)
set(COMMON_FIRMWAREDIR ${SRCDIR}/MsgConvert/Source/Firmware/Code/Common)
set(HALDIR ${SRCDIR}/MsgConvert/Source/Firmware/Code/HAL)
set(ALGORITHM_BASEDIR ${SRCDIR}/QCSource/AlgorithmBase)
set(GNSSQC_PROCESSDIR ${SRCDIR}/QCSource/GNSSQCProcess)
set(VERIQCSPP_COMMONDIR ${SRCDIR}/VeriQCSPP/Common)
set(GNSSBASEDIR ${VERIQCSPP_COMMONDIR}/GNSSBase)
set(VERIQCSPP_INTERFACE ${SRCDIR}/VeriQCSPP/Interface)
set(SPPDIR ${SRCDIR}/VeriQCSPP/SPP)


include_directories(
    ${SRCDIR}/QCSource/Include
    ${SRCDIR}/QCSource/GNSSQCProcess
    ${SRCDIR}/QCSource/AlgorithmBase
    ${SRCDIR}/Interface
    ${MSGCONVERT_UNITTESTDIR}
    ${MSGCONVERT_COMMONDIR}
    ${MSGCONVERT_DECODERDIR}
    ${MSGCONVERT_ENCODERDIR}
    ${MSGCONVERTDIR}/Version
    ${SRCDIR}/MsgConvert/Source/Firmware/Baseband/Track/TrackLoop
    ${COMMON_FIRMWAREDIR}
    ${SRCDIR}/MsgConvert/Source/Firmware/Code
    ${SRCDIR}/MsgConvert/Source/Firmware/Project/Code
    ${SRCDIR}
    ${GNSSBASEDIR}
    ${VERIQCSPP_COMMONDIR}
    ${VERIQCSPP_INTERFACE}
    ${SPPDIR}
    ${COMMONDIR}
)


file(GLOB SOURCES
    ${SRCDIR}/*.c
    ${COMMONDIR}/*.c
    ${INTERFACEDIR}/*.c
    ${MSGCONVERTDIR}/*.c
    ${MSGCONVERT_UNITTESTDIR}/*.c
    ${MSGCONVERT_UNITTESTDIR}/Common/*.c
    ${MSGCONVERT_UNITTESTDIR}/Decoder/*.c
    ${MSGCONVERT_UNITTESTDIR}/Decoder/Rinex/*.c
    ${MSGCONVERT_UNITTESTDIR}/Decoder/RTCM/*.c
    ${MSGCONVERT_UNITTESTDIR}/Encoder/RINEX/*.c
    ${COMMON_FIRMWAREDIR}/*.c
    ${COMMON_FIRMWAREDIR}/CheckSum/*.c
    ${HALDIR}/*.c
    ${ALGORITHM_BASEDIR}/*.c
    ${GNSSQC_PROCESSDIR}/*.c
    ${VERIQCSPP_COMMONDIR}/*.c
    ${GNSSBASEDIR}/*.c
    ${VERIQCSPP_INTERFACE}/*.c
    ${SPPDIR}/*.c
)


add_library(VeriQC SHARED ${SOURCES})
target_link_libraries(VeriQC ${PROP_MSG_DECODER_LIB_PATH})