#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#if (defined(_WIN32) || defined(__WIN32__))
#define mkdir(A, B) mkdir(A)
#endif

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

#define readBytesA(__VAR__, __SIZE__, __FILE__) \
    char *__VAR__ = (char *)malloc(__SIZE__);\
    fread(__VAR__, sizeof(char), __SIZE__, __FILE__);\

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

typedef struct {
    uint32_t padding;
    uint32_t offset;
} TextureAddress;

int main(int argc, char *argv[]) {
    int ch;
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

            mkdir("output", 0755);
            chmod("output", 0755);
            chdir("./output");

            char filename[100];
            FILE *stringFile = fopen("string.txt", "wb");

            char chunkName[5];
            chunkName[4] = '\0';
            uint32_t totalSize;
            while (fread(chunkName, 1, 4, file) > 0) {
                read32(chunkSize, file);
                long chunkLast = ftell(file) + chunkSize;
                fprintf(stdout, "%s: %u\n", chunkName, chunkSize);

                if (strcmp(chunkName, "FORM") == 0) {
                    totalSize = chunkSize;
                } else {
                    if (strcmp(chunkName, "STRG") == 0) {
                        read32(entryNum, file);
                        fprintf(stdout, "%u texts\n", entryNum);

                        read32array(entryOffsets, entryNum, file);

                        for (int i = 0; i < entryNum - 1; i++) {
                            read32(entrySize, file);

                            readBytes(itemBuf, entrySize, file);
                            fwrite(itemBuf, entrySize, 1, stringFile);

                            fputs("\n", stringFile);
                            fseek(file, 1, SEEK_CUR); // 1 byte margin between strings 
                        }

                        fseek(file, chunkLast, SEEK_SET);
                    } else if (strcmp(chunkName, "TXTR") == 0) {
                        mkdir("texture", 0755);
                        chmod("texture", 0755);
                        chdir("./texture");

                        read32(entryNum, file);
                        fprintf(stdout, "%u texture files\n", entryNum);

                        read32array(entryOffsets, entryNum, file);
                        readTarray(TextureAddress, addresses, entryNum, file);

                        // 最後のテクスチャ (entryNum - 1 番目) が読み込めない 仕様？
                        for (int i = 0; i < entryNum - 1; i++) {
                            TextureAddress a = addresses[i];
                            TextureAddress b = addresses[i + 1];
                            uint32_t entrySize = b.offset - a.offset;
                            readBytes(itemBuf, entrySize, file);
                            writeToFile(itemBuf, entrySize, "%d.png", i);
                        }

                        chdir("..");
                        fseek(file, chunkLast, SEEK_SET);
                    } else if (strcmp(chunkName, "AUDO") == 0) {
                        mkdir("audio", 0755);
                        chmod("audio", 0755);
                        chdir("./audio");

                        read32(entryNum, file);
                        fprintf(stdout, "%u audio files\n", entryNum);

                        read32array(entryOffsets, entryNum, file);

                        for (int i = 0; i < entryNum; i++) {
                            fseek(file, entryOffsets[i], SEEK_SET);
                            read32(entrySize, file);
                            readBytes(itemBuf, entrySize, file);
                            writeToFile(itemBuf, entrySize, "%d.wav", i);
                        }

                        chdir("..");
                        fseek(file, chunkLast, SEEK_SET);
                    } else {
                        //readBytesA(itemBuf, chunkSize, file); // なぜかここでクラッシュする
                        //free(itemBuf);
                        fseek(file, chunkSize, SEEK_CUR);
                        //writeToFile(itemBuf, chunkSize, "%s/%s", outdir, chunkName);
                    }
                }
            }
            break;
        }
        default:
            usage();
        }
    }
}
