#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define readT(__TYPE__, __VAR__, __FILE__) \
    __TYPE__ __VAR__;\
    fread(&__VAR__, sizeof(__TYPE__), 1, __FILE__);\

#define readTarray(__TYPE__, __VAR__, __NUM__, __FILE__) \
    __TYPE__ __VAR__[__NUM__];\
    fread(__VAR__, sizeof(__TYPE__), __NUM__, __FILE__);\

#define readByte(__VAR__, __FILE__) \
    readT(char, __VAR__, __FILE__)

#define readBytes(__VAR__, __NUM__, __FILE__) \
    readTarray(char, __VAR__, __NUM__, __FILE__)

#define read32(__VAR__, __FILE__) \
    readT(uint32_t, __VAR__, __FILE__)

#define read32array(__VAR__, __NUM__, __FILE__) \
    readTarray(uint32_t, __VAR__, __NUM__, __FILE__)

#define writeToFile(__BYTES__, __SIZE__, __FORMAT__, ...) {\
    char filename[100];\
    sprintf(filename, __FORMAT__, __VA_ARGS__);\
    FILE *file = fopen(filename, "wb");\
    fwrite(__BYTES__, __SIZE__, 1, file);\
    fclose(file);\
}\

static void usage(void) {
    fprintf(stderr, "usage: gmspack [-ae] [file]\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int ch;
    char *outdir = "output";
    while ((ch = getopt(argc, argv, "a:e:")) != -1) {
        switch (ch) {
        case 'a':
            fprintf(stdout, "%s %s\n", optarg, argv[1]);
            break;
        case 'e': {
            FILE *file = fopen(optarg, "rb");
            if (!file) {
                fprintf(stderr, "cannot open file\n");
                exit(1);
            }

            char filename[100];
            sprintf(filename, "%s/string.txt", outdir);
            FILE *stringFile = fopen(filename, "wb");

            char chunkName[4];
            while (fread(chunkName, 4, 1, file) > 0) {
                read32(chunkSize, file);
                long chunkTop = ftell(file);
                long chunkLast = chunkTop + chunkSize;
                if (strcmp(chunkName, "FORM") == 0) {
                } else {
                    if (strcmp(chunkName, "STRG") == 0) {
                        read32(entryNum, file);
                        fprintf(stdout, "%ld texts\n", entryNum);

                        read32array(entryOffsets, entryNum, file);

                        for (int i = 0; i < entryNum - 1; i++) {
                            read32(entrySize, file);

                            readBytes(itemBuf, entrySize, file);
                            fwrite(itemBuf, entrySize, 1, stringFile);

                            fputs("\n", stringFile);
                            fseek(file, 1, SEEK_CUR); // 1 byte margin between strings 
                        }

                    } else if (strcmp(chunkName, "TXTR") == 0) {
                        read32(entryNum, file);
                        fprintf(stdout, "%ld texture files\n", entryNum);

                    } else if (strcmp(chunkName, "AUDO") == 0) {
                        read32(entryNum, file);
                        fprintf(stdout, "%ld audio files\n", entryNum);

                        read32array(entryOffsets, entryNum, file);

                        for (int i = 0; i < entryNum - 1; i++) {
                            fseek(file, entryOffsets[i], SEEK_SET);
                            read32(entrySize, file);
                            readBytes(itemBuf, entrySize, file);
                            writeToFile(itemBuf, entrySize, "%s/%d.wav", outdir, i);
                        }
                    } else {
                        readBytes(itemBuf, chunkSize, file);
                        //writeToFile(itemBuf, chunkSize, "%s/%s", outdir, chunkName);
                    }

                    fseek(file, chunkLast, SEEK_SET);
                }
            }
            break;
        }
        default:
            usage();
        }
    }
}
