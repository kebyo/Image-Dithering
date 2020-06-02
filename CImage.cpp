#include "CImage.h"
#include <limits.h>

CImage::CImage(FILE *f, SInput config) {
    file = f;
    if (fscanf(f, "P%i%i%i%i\n", &this->version, &this->width, &this->height, &max_val) != 4) {
        throw CExpension("Wrong amount data in file", f);
    }
    size = width * height;
    unsigned char *buffer = new unsigned char[size];
    pix = new double[size];
    fread(buffer, sizeof(unsigned char), size, f);
    for (int i = 0; i < size; i++) {
        pix[i] = (double) buffer[i];
    }
    for (int i = 0; i < 256; i++) {
        pallete[i] = newPix(i, config.bit);
    }
    delete[] buffer;
    fclose(f);
}

void CImage::ditherIt(SInput config) {
    if (config.gradient == '1') {
        makeGradient(config);
    }
    for (int i = 0; i < size; i++) {
        pix[i] = Gamma(pix[i], config.gamma);
    }
    switch (config.dith) {
        case 0:
            WithoutDith(config);
            break;
        case 1:
            Ordered8x8(config);
            break;
        case 2:
            Random(config);
            break;
        case 3:
            FloydSteinberg(config);
            break;
        case 4:
            JJN(config);
            break;
        case 5:
            Sierra(config);
            break;
        case 6:
            Atkinson(config);
            break;
        case 7:
            Halftone4x4orthogonal(config);
            break;
    }
    for (int i = 0; i < size; i++) {
        pix[i] = reverseGamma(pix[i], config.gamma);
    }
}

void CImage::makeGradient(SInput config) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            double newColor = 255.0 * (double) i / width;
            pix[j * width + i] = (int) round(newColor);
        }
    }
}

double CImage::Gamma(double value, double gamma) {
    value /= 255.0;
    if (gamma == 0.0) {
        double a = 0.055;
        if (value <= 0.04045) {
            return 255.0 * value / 12.92;
        }
        return 255.0 * pow((value + a) / (1.0 + a), 2.4);
    }
    return 255.0 * pow(value, gamma);
}

double CImage::reverseGamma(double value, double gamma) {
    value /= 255.0;
    if (gamma == 0.0) {
        double a = 0.055;
        if (value <= 0.0031308) {
            return 255.0 * value * 12.92;
        }
        return 255.0 * ((1.0 + a) * pow(value, (double) (1 / 2.4)) - a);
    }
    return 255.0 * pow(value, pow(gamma, -1));
}

void CImage::WithoutDith(SInput config) {
    for (int i = 0; i < size; i++) {
        pix[i] = findNearestPalleteCollor((int) pix[i]);
    }
}

void CImage::Ordered8x8(SInput config) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int clr = (int) pix[j * width + i];
            clr = clr + 255.0 * (Bayer[i % 8][j % 8] - 0.5);
            pix[j * width + i] = findNearestPalleteCollor(clr);
        }
    }
}

void CImage::Random(SInput config) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int clr = pix[index(i, j)];
            clr = clr + 255.0 * ((double) rand() / RAND_MAX - 0.5);
            pix[j * width + i] = findNearestPalleteCollor(clr);
        }
    }
}

void CImage::FloydSteinberg(SInput config) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            double oldPixel = pix[index(i, j)];
            double newPixel = findNearestPalleteCollor((int) oldPixel);
            double error = oldPixel - newPixel;
            pix[index(i, j)] = newPixel;
            if (i + 1 < width) {
                pix[index(i + 1, j)] += error * (7.0 / 16.0);
            }
            if (i - 1 >= 0 && j + 1 < height) {
                pix[index(i - 1, j + 1)] += error * (3.0 / 16.0);
            }
            if (j + 1 < height) {
                pix[index(i, j + 1)] += error * (5.0 / 16.0);
            }
            if (i + 1 < width && j + 1 < height) {
                pix[index(i + 1, j + 1)] += error * (1.0 / 16.0);
            }
        }
    }
}

void CImage::JJN(SInput config) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            double oldPixel = pix[index(i, j)];
            double newPixel = findNearestPalleteCollor((int) oldPixel);
            double error = oldPixel - newPixel;
            pix[index(i, j)] = newPixel;
            if (i + 1 < width) {
                pix[index(i + 1, j)] += error * (7.0 / 48.0);
            }
            if (i + 2 < width) {
                pix[index(i + 2, j)] += error * (5.0 / 48.0);
            }
            if (j + 1 < height) {
                if (i - 2 >= 0) {
                    pix[index(i - 2, j + 1)] += error * (3.0 / 48.0);
                }
                if (i - 1 >= 0) {
                    pix[index(i - 1, j + 1)] += error * (5.0 / 48.0);
                }
                pix[index(i, j + 1)] += error * (7.0 / 48.0);
                if (i + 1 < width) {
                    pix[index(i + 1, j + 1)] += error * (5.0 / 48.0);
                }
                if (i + 2 < width) {
                    pix[index(i + 2, j + 1)] += error * (3.0 / 48.0);
                }
            }
            if (j + 2 < height) {
                if (i - 2 >= 0) {
                    pix[index(i - 2, j + 2)] += error * (1.0 / 48.0);
                }
                if (i - 1 >= 0) {
                    pix[index(i - 1, j + 2)] += error * (3.0 / 48.0);
                }
                pix[index(i, j + 2)] += error * (5.0 / 48.0);
                if (i + 1 < width) {
                    pix[index(i + 1, j + 2)] += error * (3.0 / 48.0);
                }
                if (i + 2 < width) {
                    pix[index(i + 2, j + 2)] += error * (1.0 / 48.0);
                }
            }
        }
    }
}

void CImage::Sierra(SInput config) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            double oldPixel = pix[index(i, j)];
            double newPixel = findNearestPalleteCollor((int) oldPixel);
            double error = oldPixel - newPixel;
            pix[index(i, j)] = newPixel;
            if (i + 1 < width) {
                pix[index(i + 1, j)] += error * (5.0 / 32.0);
            }
            if (i + 2 < width) {
                pix[index(i + 2, j)] += error * (3.0 / 32.0);
            }
            if (j + 1 < height) {
                if (i - 2 >= 0) {
                    pix[index(i - 2, j + 1)] += error * (2.0 / 32.0);
                }
                if (i - 1 >= 0) {
                    pix[index(i - 1, j + 1)] += error * (4.0 / 32.0);
                }
                pix[index(i, j + 1)] += error * (5.0 / 32.0);
                if (i + 1 < width) {
                    pix[index(i + 1, j + 1)] += error * (4.0 / 32.0);
                }
                if (i + 2 < width) {
                    pix[index(i + 2, j + 1)] += error * (2.0 / 32.0);
                }
            }
            if (j + 2 < height) {
                if (i - 1 >= 0) {
                    pix[index(i - 1, j + 2)] += error * (2.0 / 32.0);
                }
                pix[index(i, j + 2)] += error * (3.0 / 32.0);
                if (i + 1 < width) {
                    pix[index(i + 1, j + 2)] += error * (2.0 / 32.0);
                }
            }
        }
    }
}

void CImage::Atkinson(SInput config) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            double oldPixel = pix[index(i, j)];
            double newPixel = findNearestPalleteCollor((int) oldPixel);
            double error = oldPixel - newPixel;
            pix[index(i, j)] = newPixel;
            if (i + 1 < width) {
                pix[index(i + 1, j)] += error * (1.0 / 8.0);
            }
            if (i + 2 < width) {
                pix[index(i + 2, j)] += error * (1.0 / 8.0);
            }
            if (j + 1 < height) {
                if (i - 1 >= 0) {
                    pix[index(i - 1, j + 1)] += error * (1.0 / 8.0);
                }
                pix[index(i, j + 1)] += error * (1.0 / 8.0);
                if (i + 1 < width) {
                    pix[index(i + 1, j + 1)] += error * (1.0 / 8.0);
                }
            }
            if (j + 2 < height) {
                pix[index(i, j + 2)] += error * (1.0 / 8.0);
            }
        }
    }
}

void CImage::Halftone4x4orthogonal(SInput config) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int clr = (int) pix[j * width + i];
            clr = clr + 255.0 * (Halftone[i % 4][j % 4] - 0.5);
            pix[j * width + i] = findNearestPalleteCollor(clr);
        }
    }

}

int CImage::newPix(int n, int bit) {
    if (bit == 8) {
        return n;
    }
    int tmp = n >> (8 - bit);
    int result = 0;
    switch (bit) {
        case 1:
            for (int i = 0; i < 7; i++) {
                result |= tmp;
                result = result << 1;
            }
            return result | tmp;
        case 2:
            for (int i = 0; i < 3; i++) {
                result |= tmp;
                result = result << 2;
            }
            return result | tmp;
        case 3:
            for (int i = 0; i < 2; i++) {
                result |= tmp;
                result = result << 3;
            }
            result >>= 3;
            result = result << 2;
            tmp = tmp >> 1;
            return result | tmp;
        case 4:
            for (int i = 0; i < 2; i++) {
                result |= tmp;
                result = result << 4;
            }
            return result >> 4;
        case 5:
            result |= tmp;
            result = result << 3;
            tmp = tmp >> 2;
            return result | tmp;
        case 6:
            result |= tmp;
            result = result << 2;
            tmp = tmp >> 4;
            return result | tmp;
        case 7:
            result |= tmp;
            result = result << 1;
            tmp = tmp >> 6;
            return result | tmp;
    }
    return 0;
}

int CImage::findNearestPalleteCollor(int value) {
    int min = 256;
    int result;
    for (int i = 0; i < 256; i++) {
        if (min > abs(value - pallete[i])) {
            min = abs(value - pallete[i]);
            result = pallete[i];
        }
    }
    return result;
}

int CImage::index(int x, int y) {
    return y * width + x;
}