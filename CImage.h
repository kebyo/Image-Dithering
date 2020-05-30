#ifndef GRAPH3_CIMAGE_H
#define GRAPH3_CIMAGE_H

#include <iostream>
#include "CExpension.h"
#include <cmath>
#include <string>
#include "Matrix.h"

struct SInput {
    const char *inputName;
    const char *outputName;
    char gradient;
    int dith;
    int bit;
    double gamma;
};


class CImage {
public:
    CImage(FILE *f, SInput config);

    void ditherIt(SInput config);

    friend void getAnsw(const char *fileName, CImage &output);


private:
    int pallete[256];
    FILE *file;
    int version;
    int width;
    int height;
    int max_val;
    int size;
    double *pix;

    double Gamma(double value, double gamma);

    double reverseGamma(double value, double gamma);

    void WithoutDith(SInput config);

    void Ordered8x8(SInput config);

    void Random(SInput config);

    void FloydSteinberg(SInput config);

    void JJN(SInput config);

    void Sierra(SInput config);

    void Atkinson(SInput config);

    void Halftone4x4orthogonal(SInput config);

    void makeGradient(SInput config);

    int newPix(int n, int bit);

    int index(int x, int y);

    int findNearestPalleteCollor(int value);

};


#endif //GRAPH3_CIMAGE_H
