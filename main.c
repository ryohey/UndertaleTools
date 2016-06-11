#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include "structs.h"

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

// the caller must free() the returnd string
#define readStringAt(__VAR__, __OFFSET__, __FILE__) \
    char __VAR__[10000];\
    fseek(__FILE__, __OFFSET__, SEEK_SET);\
    fscanf(__FILE__, "%s", __VAR__);

static void usage(void) {
    fprintf(stderr, "usage: gmspack [-ae] [file]\n");
    exit(1);
}

static void dump(char *bytes, long size) {
    fprintf(stdout, "========");
    for (long i = 0; i < size; i++) {
        if (i % 8 == 0) {
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "%d\t", bytes[i]);
    }
    fprintf(stdout, "\n========\n");
}

static void dump32uint(uint32_t *bytes, long size, int wrap) {
    fprintf(stdout, "========");
    for (long i = 0; i < size; i++) {
        if (i % wrap == 0) {
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "%x\t", bytes[i]);
    }
    fprintf(stdout, "\n========\n");
}

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

            mkdir("chunk", 0755);
            chmod("chunk", 0755);

            while (1) {
                Chunk chunk;
                if (fread(&chunk, sizeof(Chunk), 1, file) != 1) {
                    break;
                }

                // add string terminater
                char chunkName[5];
                memcpy(chunkName, chunk.name, 4);
                chunkName[4] = '\0';
                long chunkTop = ftell(file);
                long chunkLast = chunkTop + chunk.size;
                fprintf(stdout, "(%ld) %s: %u\n", chunkTop, chunkName, chunk.size);

                if (strcmp(chunkName, "FORM") == 0) {
                    uint32_t totalSize = chunk.size;
                } else {
                    if (strcmp(chunkName, "STRG") == 0) {
                        FILE *stringFile = fopen("string.txt", "wb");

                        read32(entryNum, file);
                        read32array(entryOffsets, entryNum, file);
                        fprintf(stdout, "%u texts\n", entryNum);

                        for (int i = 0; i < entryNum; i++) {
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
                            readStringAt(spriteName, sprite.nameOffset, file);

                            fprintf(spriteFile, "\"%s\",%u,%u,%u,%u,%u,%u,%u\n", 
                                spriteName, 
                                sprite.size.width, sprite.size.height, 
                                sprite.bounds.left, sprite.bounds.top, 
                                sprite.bounds.right, sprite.bounds.bottom, 
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

                        for (int i = 0; i < entryNum; i++) {
                            fseek(file, entryOffsets[i], SEEK_SET);
                            readT(Font, font, file);
                            read32array(glyphOffsets, font.glyphCount, file);
                            
                            readStringAt(fileName, font.fileNameOffset, file);
                            readStringAt(name, font.nameOffset, file);

                            fprintf(fontFile, "\"%s\",\"%s\",%u\n", fileName, name, font.pointSize);

                            // read glyph

                            openFile(glyphFile, "wb", "%d.csv", i);
                            GlyphPrintCSVHeader(glyphFile);

                            for (int n = 0; n < font.glyphCount; n++) {
                                fseek(file, glyphOffsets[n], SEEK_SET);
                                readT(Glyph, glyph, file);
                                GlyphPrintCSV(glyphFile, glyph);
                            }

                            fclose(glyphFile);
                        }

                        fclose(fontFile);
                        chdir("..");
                        fseek(file, chunkLast, SEEK_SET);
                    } else if (strcmp(chunkName, "ROOM") == 0) {
                        mkdir("room", 0755);
                        chmod("room", 0755);
                        chdir("./room");

                        FILE *roomFile = fopen("room.csv", "wb");
                        FILE *bgFile = fopen("background.csv", "wb");
                        FILE *viewFile = fopen("view.csv", "wb");
                        FILE *objFile = fopen("object.csv", "wb");

                        RoomPrintCSVHeader(roomFile);
                        BackgroundPrintCSVHeader(bgFile);
                        ViewPrintCSVHeader(viewFile);
                        RoomObjectPrintCSVHeader(objFile);

                        read32(entryNum, file);
                        read32array(entryOffsets, entryNum, file);
                        fprintf(stdout, "%u rooms\n", entryNum);

                        for (int i = 0; i < entryNum; i++) {
                            fseek(file, entryOffsets[i], SEEK_SET);
                            readT(Room, room, file);

                            {
                                read32(num, file);
                                read32array(bgOffsets, num, file);
                                readTarray(Background, arr, num, file);

                                for (int n = 0; n < num; n++) {
                                    BackgroundPrintCSV(bgFile, arr[n], i);
                                }
                            }

                            {
                                read32(num, file);
                                read32array(viewOffsets, num, file);
                                readTarray(View, arr, num, file);

                                for (int n = 0; n < num; n++) {
                                    ViewPrintCSV(viewFile, arr[n], i);
                                }
                            }

                            {
                                read32(num, file);
                                read32array(objOffsets, num, file);
                                readTarray(RoomObject, arr, num, file);

                                for (int n = 0; n < num; n++) {
                                    RoomObjectPrintCSV(objFile, arr[n], i);
                                }
                            }
                        
                            readStringAt(name, room.nameOffset, file);
                            readStringAt(caption, room.captionOffset, file);
                            RoomPrintCSV(roomFile, room, name, caption);

                            // read32(tileNum, file);
                            // fprintf(stdout, "tileNum: %u\n", tileNum); 
                            // read32array(tileOffsets, tileNum, file);
                            // fprintf(stdout, "c %d\n", ftell(file));
                        }

                        fclose(roomFile);
                        fclose(bgFile);
                        fclose(viewFile);
                        fclose(objFile);
                        chdir("..");
                        fseek(file, chunkLast, SEEK_SET);
                    } else {
                        chdir("./chunk");
                        copyToFile(file, chunk.size, "%s.chunk", chunkName);
                        chdir("..");
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
