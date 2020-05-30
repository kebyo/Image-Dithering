//
// Created by ssiie on 17.05.2020.
//

#include "CExpension.h"

CExpension::CExpension(string error) {
    this->error_ = error;
}

CExpension::CExpension(string error, FILE *file) {
    this->error_ = error;
    this->file_ = file;
}

string CExpension::getError() {
    return this->error_;
}

FILE *CExpension::getFile() {
    return this->file_;
}