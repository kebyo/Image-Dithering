//
// Created by ssiie on 17.05.2020.
//

#include "CImage.h"

CImage::CImage(FILE *f) {
    file = f;
    if (fscanf(f, "P%i%i%i%i\n", &this->version, &this->width, &this->height, &max_val) != 4) {
        throw CExpension("Wrong amount data in file", f);
    }
    size = width * height;
    pix = new unsigned char[size];
    fread(pix, sizeof(unsigned char), size, f);
    fclose(f);
}

void CImage::ditherIt(SInput config) {
    if (config.gradient == '1') {
        makeGradient(config);
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
    if (gamma == 0) {
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
    if (gamma == 0) {
        double a = 0.055;
        if (value <= 0.0031308) {
            return 255.0 * value * 12.92;
        }
        return 255 * (1.0 + a) * pow(value, (double) 1 / 2.4) - a;
    }
    return 255.0 * pow(value, (double) 1 / gamma);
}

void CImage::WithoutDith(SInput config) {
    for (int i = 0; i < size; i++) {
        int clr = pix[i];
        clr = reverseGamma(clr, config.gamma);
        clr = newPix((int) clr, config.bit);
        pix[i] = Gamma(clr, config.gamma);
    }
}

void CImage::Ordered8x8(SInput config) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            double clr = reverseGamma(pix[j * width + i], config.gamma);
            clr = clr + 255.0 / config.bit * (Bayer[i % 8][j % 8] - 0.5);
            clr = newPix((int) clr, config.bit);
            pix[j * width + i] = round(Gamma(clr, config.gamma));
        }
    }
}

void CImage::Random(SInput config) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            double clr = reverseGamma(pix[j * width + i], config.gamma);
            clr = clr + 255.0 / config.bit * ((double) rand() / RAND_MAX - 0.5);
            clr = newPix((int) clr, config.bit);
            pix[j * width + i] = round(Gamma(clr, config.gamma));
        }
    }
}

void CImage::FloydSteinberg(SInput config) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            double oldPixel = reverseGamma(pix[index(i, j)], config.gamma);
            double newPixel = newPix(oldPixel, config.bit);
            pix[index(i, j)] = Gamma(newPixel, config.gamma);
            double quant_error = oldPixel - newPixel;
            if (i + 1 < width) {
                pix[index(i + 1, j)] = round(Gamma(pix[index(i + 1, j)] + quant_error * 7.0 / 16.0, config.gamma));
            }
            if (i - 1 >= 0 && j + 1 < height) {
                pix[index(i - 1, j + 1)] = round(
                        Gamma(pix[index(i - 1, j + 1)] + quant_error * 3.0 / 16.0, config.gamma));
            }
            if (j + 1 < height) {
                pix[index(i, j + 1)] = round(Gamma(pix[index(i, j + 1)] + quant_error * 5.0 / 16.0, config.gamma));
            }
            if (i + 1 < width && j + 1 < height) {
                pix[index(i + 1, j + 1)] = round(
                        Gamma(pix[index(i + 1, j + 1)] + quant_error * 1.0 / 16.0, config.gamma));
            }
        }
    }
}

void CImage::JJN(SInput config) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            double oldPixel = reverseGamma(pix[index(i, j)], config.gamma);
            double newPixel = newPix(oldPixel, config.bit);
            pix[index(i, j)] = Gamma(newPixel, config.gamma);
            double quant_error = oldPixel - newPixel;
            if (i + 1 < width) {
                pix[index(i + 1, j)] = round(
                        Gamma(pix[index(i + 1, j)] + quant_error * (double) 7.0 / 48.0, config.gamma));
            }
            if (i + 2 < width) {
                pix[index(i + 2, j)] = round(
                        Gamma(pix[index(i + 2, j)] + quant_error * (double) 5.0 / 48.0, config.gamma));
            }
            if (j + 1 < height) {
                if (i - 2 >= 0) {
                    pix[index(i - 2, j + 1)] = round(
                            Gamma(pix[index(i - 2, j + 1)] + quant_error * (double) 3.0 / 48.0, config.gamma));
                }
                if (i - 1 >= 0) {
                    pix[index(i - 1, j + 1)] = round(
                            Gamma(pix[index(i - 1, j + 1)] + quant_error * (double) 5.0 / 48.0, config.gamma));
                }
                pix[index(i, j + 1)] = round(
                        Gamma(pix[index(i, j + 1)] + quant_error * (double) 7.0 / 48.0, config.gamma));
                if (i + 1 >= 0) {
                    pix[index(i + 1, j + 1)] = round(
                            Gamma(pix[index(i + 1, j + 1)] + quant_error * (double) 5.0 / 48.0, config.gamma));
                }
                if (i + 2 >= 0) {
                    pix[index(i + 2, j + 1)] = round(
                            Gamma(pix[index(i + 2, j + 1)] + quant_error * (double) 3.0 / 48.0, config.gamma));
                }
            }
            if (j + 2 < height) {
                if (i - 2 >= 0) {
                    pix[index(i - 2, j + 2)] = round(
                            Gamma(pix[index(i - 2, j + 2)] + quant_error * (double) 1.0 / 48.0, config.gamma));
                }
                if (i - 1 >= 0) {
                    pix[index(i - 1, j + 2)] = round(
                            Gamma(pix[index(i - 1, j + 2)] + quant_error * (double) 3.0 / 48.0, config.gamma));
                }
                pix[index(i, j + 2)] = round(
                        Gamma(pix[index(i, j + 2)] + quant_error * (double) 5.0 / 48.0, config.gamma));
                if (i + 1 >= 0) {
                    pix[index(i + 1, j + 2)] = round(
                            Gamma(pix[index(i + 1, j + 2)] + quant_error * (double) 3.0 / 48.0, config.gamma));
                }
                if (i + 2 >= 0) {
                    pix[index(i + 2, j + 2)] = round(
                            Gamma(pix[index(i + 2, j + 2)] + quant_error * (double) 1.0 / 48.0, config.gamma));
                }
            }
        }
    }
}

void CImage::Sierra(SInput config) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            double oldPixel = reverseGamma(pix[index(i, j)], config.gamma);
            double newPixel = newPix(oldPixel, config.bit);
            pix[index(i, j)] = Gamma(newPixel, config.gamma);
            double quant_error = oldPixel - newPixel;
            for (int ni = 1; ni < 3; ni++) {
                if (i + ni < width) {
                    if (ni == 1) {
                        pix[index(i + ni, j)] = round(
                                Gamma(pix[index(i + ni, j)] + quant_error * (double) 5.0 / 32.0, config.gamma));
                    }
                    if (ni == 2) {
                        pix[index(i + ni, j)] = round(
                                Gamma(pix[index(i + ni, j)] + quant_error * (double) 3.0 / 32.0, config.gamma));
                    }
                }
            }
            if (j + 1 < height) {
                for (int ni = 0; ni < 3; ni++) {
                    if (i + ni < width) {
                        if (ni == 0) {
                            pix[index(i + ni, j + 1)] = round(
                                    Gamma(pix[index(i + ni, j + 1)] + quant_error * (double) 5.0 / 32.0, config.gamma));
                        }
                        if (ni == 1) {
                            pix[index(i + ni, j + 1)] = round(
                                    Gamma(pix[index(i + ni, j + 1)] + quant_error * (double) 4.0 / 32.0, config.gamma));
                        }
                        if (ni == 2) {
                            pix[index(i + ni, j + 1)] = round(
                                    Gamma(pix[index(i + ni, j + 1)] + quant_error * (double) 2.0 / 32.0, config.gamma));
                        }
                    }
                }
                for (int ni = -2; ni < 0; ni++) {
                    if (i + ni >= 0) {
                        if (ni == -2) {
                            pix[index(i + ni, j + 1)] = round(
                                    Gamma(pix[index(i + ni, j + 1)] + quant_error * (double) 2.0 / 32.0, config.gamma));
                        }
                        if (ni == -1) {
                            pix[index(i + ni, j + 1)] = round(
                                    Gamma(pix[index(i + ni, j + 1)] + quant_error * (double) 4.0 / 32.0, config.gamma));
                        }
                    }
                }
            }
            if (j + 2 < height) {
                pix[index(i, j + 2)] = round(
                        Gamma(pix[index(i, j + 2)] + quant_error * (double) 3.0 / 32.0, config.gamma));
                if (i - 1 >= 0) {
                    pix[index(i - 1, j + 2)] = round(
                            Gamma(pix[index(i - 1, j + 2)] + quant_error * (double) 2.0 / 32.0, config.gamma));
                }
                if (i + 1 < width) {
                    pix[index(i + 1, j + 2)] = round(
                            Gamma(pix[index(i + 1, j + 2)] + quant_error * (double) 2.0 / 32.0, config.gamma));
                }
            }
        }
    }
}

void CImage::Atkinson(SInput config) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            double oldPixel = reverseGamma(pix[index(i, j)], config.gamma);
            double newPixel = newPix(oldPixel, config.bit);
            pix[index(i, j)] = Gamma(newPixel, config.gamma);
            double quant_error = oldPixel - newPixel;
            if (i + 1 < width) {
                pix[index(i + 1, j)] = round(
                        Gamma(pix[index(i + 1, j)] + quant_error * (double) 1.0 / 8.0, config.gamma));
            }
            if (i + 2 < width) {
                pix[index(i + 2, j)] = round(
                        Gamma(pix[index(i + 2, j)] + quant_error * (double) 1.0 / 8.0, config.gamma));
            }
            if (j + 1 < height) {
                if (i + 1 < width) {
                    pix[index(i + 1, j + 1)] = round(
                            Gamma(pix[index(i + 1, j + 1)] + quant_error * (double) 1.0 / 8.0, config.gamma));
                }
                if (i - 1 >= 0) {
                    pix[index(i - 1, j + 1)] = round(
                            Gamma(pix[index(i - 1, j + 1)] + quant_error * (double) 1.0 / 8.0, config.gamma));
                }
                pix[index(i, j + 1)] = round(
                        Gamma(pix[index(i, j + 1)] + quant_error * (double) 1.0 / 8.0, config.gamma));
            }
            if (j + 2 < height) {
                pix[index(i, j + 2)] = round(
                        Gamma(pix[index(i, j + 2)] + quant_error * (double) 1.0 / 8.0, config.gamma));
            }
        }
    }
}

void CImage::Halftone4x4orthogonal(SInput config) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            double clr = reverseGamma(pix[j * width + i], config.gamma);
            clr = clr + 255.0 / config.bit * (Halftone[i % 8][j % 8] / 16 - 0.5);
            clr = newPix((int) clr, config.bit);
            pix[j * width + i] = round(Gamma(clr, config.gamma));
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

int CImage::index(int x, int y) {
    return y * width + x;
}