/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-08-21 11:45:56
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2024-08-21 11:47:15
 * @FilePath: /RaspiUSBL/general/convert.h
 * @Description: Unit conversion
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */
#ifndef _CONVERT_H_
#define _CONVERT_H_

#include <cmath>

inline double D2R(double degree) {
    return degree * M_PI / 180.0;
}

inline double R2D(double radian) {
    return radian * 180.0 / M_PI;
}

#endif // _CONVERT_H_