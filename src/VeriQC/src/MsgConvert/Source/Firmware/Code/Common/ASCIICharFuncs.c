/**********************************************************************//**
		VeriQC

	   Common Module
*-
@file    ASCIICharFuncs.c
@author  CHC
@date    2016/12/07           2022/12/02
@brief   ASCII char-based processing functions
@brief   add IsStringDigit func 2023/01/05

**************************************************************************/
#include "ASCIICharFuncs.h"

/**
 * 0-31,127 invisible char
 * 48-57    digital
 * 65-90    uppercase letter
 * 97-122   lowercase letter
 * other    punctuation char
 */
static const unsigned char ASCIITypeTable[] = {
	CHAR_P_INVISIBLE,              				/**<   0(Dec), 0x00(Hex), NUL(Char) */
	CHAR_P_INVISIBLE,              				/**<   1, 0x01, SOH */
	CHAR_P_INVISIBLE,              				/**<   2, 0x02, STX */
	CHAR_P_INVISIBLE,              				/**<   3, 0x03, ETX */
	CHAR_P_INVISIBLE,              				/**<   4, 0x04, EOT */
	CHAR_P_INVISIBLE,              				/**<   5, 0x05, ENQ */
	CHAR_P_INVISIBLE,              				/**<   6, 0x06, ACK */
	CHAR_P_INVISIBLE,              				/**<   7, 0x07, BEL */
	CHAR_P_INVISIBLE,              				/**<   8, 0x08,  BS */
	CHAR_P_INVISIBLE,              				/**<   9, 0x09,  HT */
	CHAR_P_INVISIBLE,              				/**<  10, 0x0A,  LF */
	CHAR_P_INVISIBLE,              				/**<  11, 0x0B,  VT */
	CHAR_P_INVISIBLE,              				/**<  12, 0x0C,  FF */
	CHAR_P_INVISIBLE,              				/**<  13, 0x0D,  CR */
	CHAR_P_INVISIBLE,              				/**<  14, 0x0E,  SO */
	CHAR_P_INVISIBLE,              				/**<  15, 0x0F,  SI */
	CHAR_P_INVISIBLE,              				/**<  16, 0x10, DLE */
	CHAR_P_INVISIBLE,              				/**<  17, 0x11, DC1 */
	CHAR_P_INVISIBLE,              				/**<  18, 0x12, DC2 */
	CHAR_P_INVISIBLE,              				/**<  19, 0x13, DC3 */
	CHAR_P_INVISIBLE,              				/**<  20, 0x14, DC4 */
	CHAR_P_INVISIBLE,              				/**<  21, 0x15, NAK */
	CHAR_P_INVISIBLE,              				/**<  22, 0x16, SYN */
	CHAR_P_INVISIBLE,              				/**<  23, 0x17, ETB */
	CHAR_P_INVISIBLE,              				/**<  24, 0x18, CAN */
	CHAR_P_INVISIBLE,              				/**<  25, 0x19,  EM */
	CHAR_P_INVISIBLE,              				/**<  26, 0x1A, SUB */
	CHAR_P_INVISIBLE,              				/**<  27, 0x1B, ESC */
	CHAR_P_INVISIBLE,              				/**<  28, 0x1C,  FS */
	CHAR_P_INVISIBLE,              				/**<  29, 0x1D,  GS */
	CHAR_P_INVISIBLE,              				/**<  30, 0x1E,  RS */
	CHAR_P_INVISIBLE,              				/**<  31, 0x1F,  US */
	CHAR_P_VALID,                  				/**<  32, 0x20, (space) */
	CHAR_P_VALID,                  				/**<  33, 0x21,   ! */
	CHAR_P_VALID,                  				/**<  34, 0x22,   " */
	CHAR_P_VALID,                  				/**<  35, 0x23,   # */
	CHAR_P_VALID,                  				/**<  36, 0x24,   $ */
	CHAR_P_VALID,                  				/**<  37, 0x25,   % */
	CHAR_P_VALID,                  				/**<  38, 0x26,   & */
	CHAR_P_VALID,                  				/**<  39, 0x27,   ' */
	CHAR_P_VALID,                  				/**<  40, 0x28,   ( */
	CHAR_P_VALID,                  				/**<  41, 0x29,   ) */
	CHAR_P_VALID,                  				/**<  42, 0x2A,   * */
	CHAR_P_VALID,                  				/**<  43, 0x2B,   + */
	CHAR_P_VALID,                  				/**<  44, 0x2C,   , */
	CHAR_P_VALID | CHAR_P_MINUS,                /**<  45, 0x2D,   - */
	CHAR_P_VALID | CHAR_P_POINT,                /**<  46, 0x2E,   . */
	CHAR_P_VALID,                  				/**<  47, 0x2F,   / */
	CHAR_P_DIGIT | CHAR_P_VALID | CHAR_P_HEX,	/**<  48, 0x30,   0 */
	CHAR_P_DIGIT | CHAR_P_VALID | CHAR_P_HEX,	/**<  49, 0x31,   1 */
	CHAR_P_DIGIT | CHAR_P_VALID | CHAR_P_HEX,	/**<  50, 0x32,   2 */
	CHAR_P_DIGIT | CHAR_P_VALID | CHAR_P_HEX,	/**<  51, 0x33,   3 */
	CHAR_P_DIGIT | CHAR_P_VALID | CHAR_P_HEX,	/**<  52, 0x34,   4 */
	CHAR_P_DIGIT | CHAR_P_VALID | CHAR_P_HEX,	/**<  53, 0x35,   5 */
	CHAR_P_DIGIT | CHAR_P_VALID | CHAR_P_HEX,	/**<  54, 0x36,   6 */
	CHAR_P_DIGIT | CHAR_P_VALID | CHAR_P_HEX,	/**<  55, 0x37,   7 */
	CHAR_P_DIGIT | CHAR_P_VALID | CHAR_P_HEX,	/**<  56, 0x38,   8 */
	CHAR_P_DIGIT | CHAR_P_VALID | CHAR_P_HEX,	/**<  57, 0x39,   9 */
	CHAR_P_VALID,                  				/**<  58, 0x3A,   : */
	CHAR_P_VALID,                  				/**<  59, 0x3B,   ; */
	CHAR_P_VALID,                  				/**<  60, 0x3C,   < */
	CHAR_P_VALID,                  				/**<  61, 0x3D,   = */
	CHAR_P_VALID,                  				/**<  62, 0x3E,   > */
	CHAR_P_VALID,                  				/**<  63, 0x3F,   ? */
	CHAR_P_VALID,                  				/**<  64, 0x40,   @ */
	CHAR_P_HEXUPP | CHAR_P_VALID,  				/**<  65, 0x41,   A */
	CHAR_P_HEXUPP | CHAR_P_VALID,  				/**<  66, 0x42,   B */
	CHAR_P_HEXUPP | CHAR_P_VALID,  				/**<  67, 0x43,   C */
	CHAR_P_HEXUPP | CHAR_P_VALID,  				/**<  68, 0x44,   D */
	CHAR_P_HEXUPP | CHAR_P_VALID,  				/**<  69, 0x45,   E */
	CHAR_P_HEXUPP | CHAR_P_VALID,  				/**<  70, 0x46,   F */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  71, 0x47,   G */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  72, 0x48,   H */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  73, 0x49,   I */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  74, 0x4A,   J */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  75, 0x4B,   K */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  76, 0x4C,   L */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  77, 0x4D,   M */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  78, 0x4E,   N */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  79, 0x4F,   O */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  80, 0x50,   P */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  81, 0x51,   Q */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  82, 0x52,   R */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  83, 0x53,   S */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  84, 0x54,   T */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  85, 0x55,   U */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  86, 0x56,   V */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  87, 0x57,   W */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  88, 0x58,   X */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  89, 0x59,   Y */
	CHAR_P_UPPER | CHAR_P_VALID,   				/**<  90, 0x5A,   Z */
	CHAR_P_VALID,                  				/**<  91, 0x5B,   [ */
	CHAR_P_VALID,                  				/**<  92, 0x5C,   \ */
	CHAR_P_VALID,                  				/**<  93, 0x5D,   ] */
	CHAR_P_VALID,                  				/**<  94, 0x5E,   ^ */
	CHAR_P_VALID,                  				/**<  95, 0x5F,   _ */
	CHAR_P_VALID,                  				/**<  96, 0x60,   ` */
	CHAR_P_HEXLOW | CHAR_P_VALID,  				/**<  97, 0x61,   a */
	CHAR_P_HEXLOW | CHAR_P_VALID,  				/**<  98, 0x62,   b */
	CHAR_P_HEXLOW | CHAR_P_VALID,  				/**<  99, 0x63,   c */
	CHAR_P_HEXLOW | CHAR_P_VALID,  				/**< 100, 0x64,   d */
	CHAR_P_HEXLOW | CHAR_P_VALID,  				/**< 101, 0x65,   e */
	CHAR_P_HEXLOW | CHAR_P_VALID,  				/**< 102, 0x66,   f */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 103, 0x67,   g */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 104, 0x68,   h */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 105, 0x69,   i */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 106, 0x6A,   j */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 107, 0x6B,   k */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 108, 0x6C,   l */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 109, 0x6D,   m */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 110, 0x6E,   n */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 111, 0x6F,   o */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 112, 0x70,   p */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 113, 0x71,   q */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 114, 0x72,   r */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 115, 0x73,   s */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 116, 0x74,   t */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 117, 0x75,   u */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 118, 0x76,   v */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 119, 0x77,   w */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 120, 0x78,   x */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 121, 0x79,   y */
	CHAR_P_LOWER | CHAR_P_VALID,   				/**< 122, 0x7A,   z */
	CHAR_P_VALID,                  				/**< 123, 0x7B,   { */
	CHAR_P_VALID,                  				/**< 124, 0x7C,   | */
	CHAR_P_VALID,                  				/**< 125, 0x7D,   } */
	CHAR_P_VALID,                  				/**< 126, 0x7E,   ~ */
	CHAR_P_INVISIBLE               				/**< 127, 0x7F, DEL */
};

/**********************************************************************//**
@brief convert character to the hexadecimal number it represents

@param Character [In] ASCII code(unsigned char)

@return hexadecimal number of the character

@author CHC
@date 2022/12/05
**************************************************************************/
unsigned char CharToHex(unsigned char Character)
{
	if (!IS_CHAR_ASCII(Character))
	{
		return -1;
	}
	unsigned char CharType = ASCIITypeTable[(int)Character];
	if ((CharType & CHAR_P_DIGIT))
	{
		return (Character - 48);
	}
	else if ((CharType & CHAR_P_HEXUPP) == CHAR_P_HEXUPP)
	{
		return (Character - 65 + 10);
	}
	else if ((CharType & CHAR_P_HEXLOW) == CHAR_P_HEXLOW)
	{
		return (Character - 97 + 10);
	}
	else
	{
		return -1;
	}
}
