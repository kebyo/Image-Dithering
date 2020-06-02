#include <iostream>
#include "CExpension.h"
#include <string.h>
#include "CImage.h"

SInput input;

void getAnsw(const char *fileName, CImage &output) {
    FILE *new_f = fopen(fileName, "wb");
    if (!new_f) {
        throw CExpension("Output file didn't open", new_f);
    }
    fprintf(new_f, "P%i\n%i %i\n%i\n", 5, output.width, output.height, output.max_val);
    unsigned char *buffer = new unsigned char[output.size];
    for (int i = 0; i < output.size; i++) {
        buffer[i] = (unsigned char) output.pix[i];
    }
    fwrite(buffer, sizeof(unsigned char), output.size, new_f);
    delete[] buffer;
    fclose(new_f);
}

int main(int argc, char *argv[]) {
    try {
        FILE *f;
        if (argc != 7) {
            throw CExpension("Wrong amount of arguments");
        }
        for (int i = 1; i < argc; i++) {
            switch (i) {
                case 1:
                    input.inputName = argv[i];
                    f = fopen(input.inputName, "rb");
                    if (!f) {
                        throw CExpension("Input File didn't open", f);
                    }
                    break;
                case 2:
                    input.outputName = argv[i];
                    break;
                case 3: {
                    input.gradient = argv[i][0];
                    break;
                }
                case 4:
                    input.dith = atoi(argv[i]);
                    if (input.dith < 0 || input.dith > 7) {
                        throw CExpension("Dither number is < 0 or > 7", f);
                    }
                    break;
                case 5:
                    input.bit = atoi(argv[i]);
                    break;
                case 6:
                    input.gamma = atof(argv[i]);
                    break;
            }
        }
        CImage image(f, input);
        image.ditherIt(input);
        getAnsw(input.outputName, image);
        return 0;
    } catch (CExpension &expension) {
        cerr << expension.getError();
        if (expension.getFile()) {
            fclose(expension.getFile());
        }
        exit(1);
    }
}
