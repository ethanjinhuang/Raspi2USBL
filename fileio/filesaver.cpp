/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-07-03 11:03:59
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2024-08-19 13:43:06
 * @FilePath: /RaspiUSBL/fileio/filesaver.cpp
 * @Description: Define the file saver
 * @             2024-08-19 add absl support
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */
#include "filesaver.h"
#include "absl/strings/str_format.h"
#include <iomanip>
#include <iostream>
#include <sstream>

FileSaver::FileSaver(const string &filename, int columns, int filetype) {
    bool __attribute__((unused)) isopen = open(filename, columns, filetype);
#ifdef _FILEIO_DEBUG_
    std::cout << "fileSaver::fileSaver() called, file state:" << isopen << std::endl;
#endif
}

bool FileSaver::open(const string &filename, int columns, int filetype) {
    auto type =
        (filetype == TEXT || filetype == HEX) ? std::ios_base::out : (std::ios_base::out | std::ios_base::binary);
    filefp_.open(filename, type);

    columns_  = columns;
    filetype_ = filetype;

    return isOpen();
}

bool FileSaver::open(const string &filename, int filetype) {
    auto type =
        (filetype == TEXT || filetype == HEX) ? std::ios_base::out : (std::ios_base::out | std::ios_base::binary);
    filefp_.open(filename, type);

    filetype_ = filetype;

    return isOpen();
}

void FileSaver::dump(const vector<double> &data) {
    dump_(data);
}

void FileSaver::dumpn(const vector<vector<double>> &data) {
    for (const auto &k : data) {
        dump_(k);
    }
}

void FileSaver::dump_(const vector<double> &data) {
    if (filetype_ == TEXT) {
        // Text format output
        string                      line;
        constexpr absl::string_view format = "%-15.9lf ";
        line                               = absl::StrFormat(format, data[0]);
        for (size_t k = 1; k < data.size(); ++k) {
            absl::StrAppendFormat(&line, format, data[k]);
        }
        filefp_ << line << "\n";
    } else if (filetype_ == BINARY) {

        // Binary format output
        filefp_.write(reinterpret_cast<const char *>(data.data()), sizeof(double) * data.size());
    } else if (filetype_ == HEX) {
        // HEX format output
        std::ostringstream oss;
        for (const auto &value : data) {
            // Convert double to hexadecimal
            oss << std::hexfloat << value
                << " "; // Use std::hexfloat to output the hexadecimal representation of floating-point numbers
        }
        filefp_ << oss.str() << "\n";
    }
    filefp_.flush();
}