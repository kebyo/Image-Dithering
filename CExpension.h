//
// Created by ssiie on 17.05.2020.
//

#ifndef GRAPH3_CEXPENSION_H
#define GRAPH3_CEXPENSION_H

#include <string>

using namespace std;

class CExpension {
public:
    CExpension(string error);

    CExpension(string error, FILE *file);

    string getError();

    FILE *getFile();

private:
    string error_;
    FILE *file_ = nullptr;
};


#endif //GRAPH3_CEXPENSION_H
