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
    if (fread(&__VAR__, sizeof(__TYPE__), 1, __FILE__) != 1) {\
        fprintf(stderr, "failed to read file\n");\
    }\

#define readTarray(__TYPE__, __VAR__, __NUM__, __FILE__) \
    __TYPE__ __VAR__[__NUM__];\
    if (fread(__VAR__, sizeof(__TYPE__), __NUM__, __FILE__) != __NUM__) {\
        fprintf(stderr, "failed to read file\n");\
    }\

#define readByte(__VAR__, __FILE__) \
    readT(char, __VAR__, __FILE__)

#define readBytes(__VAR__, __NUM__, __FILE__) \
    readTarray(char, __VAR__, __NUM__, __FILE__)

#define readBytesA(__VAR__, __SIZE__, __FILE__) \
    char *__VAR__ = (char *)malloc(__SIZE__);\
    if (fread(__VAR__, sizeof(char), __SIZE__, __FILE__) != __SIZE__) {\
        fprintf(stderr, "failed to read file\n");\
    }\

#define read32(__VAR__, __FILE__) \
    readT(uint32_t, __VAR__, __FILE__)

#define read32array(__VAR__, __NUM__, __FILE__) \
    readTarray(uint32_t, __VAR__, __NUM__, __FILE__)

#define openFile(__VAR__, __OPT__, __FORMAT__, ...) \
    FILE *__VAR__;\
    {\
        char filename[100];\
        sprintf(filename, __FORMAT__, __VA_ARGS__);\
        __VAR__ = fopen(filename, __OPT__);\
    }

#define writeToFile(__BYTES__, __SIZE__, __FORMAT__, ...) {\
    openFile(file, "wb", __FORMAT__, __VA_ARGS__);\
    if (fwrite(__BYTES__, __SIZE__, 1, file) != 1) {\
        fprintf(stderr, "failed to write file\n");\
    }\
    fclose(file);\
}\

#define copyToFile(__FILE__, __SIZE__, __FORMAT__, ...) {\
    char *bytes = (char *)malloc(__SIZE__);\
    if (fread(bytes, sizeof(char), __SIZE__, __FILE__) != __SIZE__) {\
        fprintf(stderr, "failed to read file\n");\
    }\
    writeToFile(bytes, __SIZE__, __FORMAT__, __VA_ARGS__);\
    free(bytes);\
}

static void usage(void) {
    fprintf(stderr, "usage: gmspack [-ae] [file]\n");
    exit(1);
}

typedef struct {
    uint32_t padding;
    uint32_t offset;
} TextureAddress;

typedef struct {
    uint32_t nameOffset;
    uint32_t width;
    uint32_t height;
    uint32_t left;
    uint32_t right;
    uint32_t bottom;
    uint32_t top;
    char unknown[20];
    uint32_t xOrigin;
    uint32_t yOrigin;
    uint32_t textureCount;
} Sprite;

typedef struct {
    uint32_t fileNameOffset;
    uint32_t nameOffset;
    uint32_t pointSize;
    char unknown[28];
    uint32_t glyphCount;
} Font;

typedef struct {
    uint8_t keyCode;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t shift;
    uint16_t offset;
    uint16_t unknown;
} Glyph;

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

            char chunkName[5];
            chunkName[4] = '\0';
            uint32_t totalSize;
            while (fread(chunkName, 1, 4, file) > 0) {
                read32(chunkSize, file);
                long chunkLast = ftell(file) + chunkSize;
                fprintf(stdout, "(%ld) %s: %u\n", ftell(file), chunkName, chunkSize);

                if (strcmp(chunkName, "FORM") == 0) {
                    totalSize = chunkSize;
                } else {
                    if (strcmp(chunkName, "STRG") == 0) {
                        FILE *stringFile = fopen("string.txt", "wb");

                        read32(entryNum, file);
                        read32array(entryOffsets, entryNum, file);
                        fprintf(stdout, "%u texts\n", entryNum);

                        for (int i = 0; i < entryNum - 1; i++) {
                            read32(entrySize, file);

                            readBytes(itemBuf, entrySize, file);
                            fwrite(itemBuf, entrySize, 1, stringFile);

                            fputs("\n", stringFile);
                            fseek(file, 1, SEEK_CUR); // 1 byte margin between strings 
                        }

                        fclose(stringFile);
                        fseek(file, chunkLast, SEEK_SET);
                    } else if (strcmp(chunkName, "TXTR") == 0) {
                        mkdir("texture", 0755);
                        chmod("texture", 0755);
                        chdir("./texture");

                        read32(entryNum, file);
                        read32array(entryOffsets, entryNum, file);
                        fprintf(stdout, "%u texture files\n", entryNum);

                        readTarray(TextureAddress, addresses, entryNum, file);

                        // 最後のテクスチャ (entryNum - 1 番目) が読み込めない 仕様？
                        for (int i = 0; i < entryNum - 1; i++) {
                            TextureAddress a = addresses[i];
                            TextureAddress b = addresses[i + 1];
                            uint32_t entrySize = b.offset - a.offset;
                            fprintf(stdout, "unpacking textures (%u KB) %d/%u...\n", entrySize / 1024, i + 1, entryNum);
                            copyToFile(file, entrySize, "%d.png", i);
                        }

                        chdir("..");
                        fseek(file, chunkLast, SEEK_SET);
                    } else if (strcmp(chunkName, "AUDO") == 0) {
                        mkdir("audio", 0755);
                        chmod("audio", 0755);
                        chdir("./audio");

                        read32(entryNum, file);
                        read32array(entryOffsets, entryNum, file);
                        fprintf(stdout, "%u audio files\n", entryNum);

                        for (int i = 0; i < entryNum; i++) {
                            fseek(file, entryOffsets[i], SEEK_SET);
                            read32(entrySize, file);
                            copyToFile(file, entrySize, "%d.wav", i);
                        }

                        chdir("..");
                        fseek(file, chunkLast, SEEK_SET);
                    } else if (strcmp(chunkName, "SPRT") == 0) {
                        FILE *spriteFile = fopen("sprite.csv", "wb");

                        read32(entryNum, file);
                        read32array(entryOffsets, entryNum, file);
                        fprintf(stdout, "%u sprites\n", entryNum);

                        fprintf(spriteFile, "name,width,height,left,top,right,bottom,textureCount\n");

                        for (int i = 0; i < entryNum - 1; i++) {
                            fseek(file, entryOffsets[i], SEEK_SET);
                            readT(Sprite, sprite, file);
                            read32array(textureAddresses, sprite.textureCount, file);

                            // read unknown bytes
                            long remainSize = entryOffsets[i + 1] - ftell(file);
                            readBytes(unknown, remainSize, file);

                            // read sprite name
                            fseek(file, sprite.nameOffset, SEEK_SET);
                            char spriteName[1000];
                            fscanf(file, "%s", spriteName);

                            fprintf(spriteFile, "\"%s\",%u,%u,%u,%u,%u,%u,%u\n", 
                                spriteName, 
                                sprite.width, sprite.height, 
                                sprite.left, sprite.top, 
                                sprite.right, sprite.bottom, 
                                sprite.textureCount);
                        }

                        fclose(spriteFile);
                        fseek(file, chunkLast, SEEK_SET);
                    } else if (strcmp(chunkName, "FONT") == 0) {
                        mkdir("font", 0755);
                        chmod("font", 0755);
                        chdir("./font");

                        FILE *fontFile = fopen("font.csv", "wb");

                        read32(entryNum, file);
                        read32array(entryOffsets, entryNum, file);
                        fprintf(stdout, "%u fonts\n", entryNum);

                        fprintf(fontFile, "fileName,name,pointSize\n");

                        for (int i = 0; i < entryNum - 1; i++) {
                            fseek(file, entryOffsets[i], SEEK_SET);
                            readT(Font, font, file);
                            read32array(glyphOffsets, font.glyphCount, file);
                            
                            fseek(file, font.fileNameOffset, SEEK_SET);
                            char fileName[1000];
                            fscanf(file, "%s", fileName);
                            
                            fseek(file, font.nameOffset, SEEK_SET);
                            char name[1000];
                            fscanf(file, "%s", name);

                            fprintf(fontFile, "\"%s\",\"%s\",%u\n", fileName, name, font.pointSize);

                            // read glyph

                            openFile(glyphFile, "wb", "%d.csv", i);
                            fprintf(glyphFile, "keyCode,x,y,width,height,shift,offset,unknown\n");

                            for (int n = 0; n < font.glyphCount; n++) {
                                fseek(file, glyphOffsets[n], SEEK_SET);
                                readT(Glyph, glyph, file);
                                fprintf(glyphFile, "%u,%u,%u,%u,%u,%u,%u,%u\n", 
                                    glyph.keyCode, 
                                    glyph.x, glyph.y, 
                                    glyph.width, glyph.height, 
                                    glyph.shift, glyph.offset, glyph.unknown);
                            }

                            fclose(glyphFile);
                        }

                        fclose(fontFile);
                        chdir("..");
                        fseek(file, chunkLast, SEEK_SET);
                    } else {
                        // skip chunk
                        fseek(file, chunkSize, SEEK_CUR);
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
