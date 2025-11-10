/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-08-14 23:38:48
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:53:24
 * @FilePath: /Raspi2USBL/fileio/filebase.h
 * @Description:
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _FILEBASE_H_
#define _FILEBASE_H_

#include <fstream>
#include <vector>

using std::string;
using std::vector;

class fileBased {

public:
    static const int TEXT   = 0;
    static const int BINARY = 1;
    static const int HEX    = 2;

    fileBased()             = default;
    ~fileBased() {
        if (isOpen()) {
            filefp_.close();
        }
    }

    void close() {
        filefp_.close();
    }

    bool isOpen() {
        return filefp_.is_open();
    }

    bool isEOF() {
        return filefp_.eof();
    }

    std::fstream &fstream() {
        return filefp_;
    }

protected:
    std::fstream filefp_;
    int          filetype_ = TEXT; // TEXT=1 BINARY=0
    int          columns_  = 0;
};

#endif // _FILEBASE_H_