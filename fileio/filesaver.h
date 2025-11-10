#ifndef _FILESAVER_H_
#define _FILESAVER_H_

#include "../config/defineconfig.h"
#include "filebase.h"

class FileSaver : public fileBased {
public:
    FileSaver() = default;
    FileSaver(const string &filename, int columns, int filetype);

    bool open(const string &filename, int columns, int filetype);

    bool open(const string &filename, int filetype);

    void dump(const vector<double> &data);

    void dumpn(const vector<vector<double>> &data);

private:
    void dump_(const vector<double> &data);
};

#endif // _FILESAVER_H_