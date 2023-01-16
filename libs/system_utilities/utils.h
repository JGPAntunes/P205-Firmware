/**
* @file		utils.h
* @brief	Contains utilities fuctions for general use.
* @version	1.0
* @date		july 2021
* @author	PFaria & JAntunes
*
* Copyright(C) 2020-2021, PFaria & JAntunes
* All rights reserved.
*/


#ifndef UTILS_H_
#define UTILS_H_


#define CONVERT_UINT16_TO_UINT8(value ,msb) (uint8_t) (msb == 0 ? (value & 0x00ff) : ((value & 0xff00) >> 8))


#endif /* UTILS_H_ */



