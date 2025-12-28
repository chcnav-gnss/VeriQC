/**********************************************************************//**
		VeriQC

	   Common Module
*-
@file    ASCIICharFuncs.h
@author  CHC
@date    2016/12/07           2022/12/02
@brief   ASCII char-based processing functions

**************************************************************************/
#ifndef _ASCII_CHAR_FUNCS_H_
#define _ASCII_CHAR_FUNCS_H_

#include "DataTypes.h"

/** Bit masks for the possible character properties */
#define CHAR_P_INVISIBLE        (1 << 0)   /**< invisible character */
#define CHAR_P_UPPER            (1 << 1)   /**< uppercase letter */
#define CHAR_P_LOWER            (1 << 2)   /**< lowercase letter */
#define CHAR_P_DIGIT            (1 << 3)   /**< digit[0-9] */
#define CHAR_P_VALID            (1 << 4)   /**< valid character */
#define CHAR_P_HEX              (1 << 5)   /**< hexadecimal digit */
#define CHAR_P_MINUS            (1 << 6)   /**< minus [-] */
#define CHAR_P_POINT            (1 << 7)   /**< point [.] */

#define CHAR_P_LETTER  (CHAR_P_UPPER | CHAR_P_LOWER) /**< uppercase letter and lowercase letter */
#define CHAR_P_HEXUPP  (CHAR_P_UPPER | CHAR_P_HEX)   /**< hexadecimal 'A'-'F' */
#define CHAR_P_HEXLOW  (CHAR_P_LOWER | CHAR_P_HEX)   /**< hexadecimal 'a'-'f' */

/** determine if the character is ASCII */
#define IS_CHAR_ASCII(_C) ((((unsigned char)(_C)) <= 0x7F) ? TRUE : FALSE)

unsigned char CharToHex(unsigned char Character);

#endif
