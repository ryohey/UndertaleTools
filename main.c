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

#define readList(__SIZE_VAR__, __ARRAY_VAR__, __FILE__) \
    read32(__SIZE_VAR__, __FILE__);\
    read32array(__ARRAY_VAR__, __SIZE_VAR__, __FILE__);

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
                fprintf(stdout, "%s (%u KB): ", chunkName, chunk.size / 1024);

                if (strcmp(chunkName, "FORM") == 0) {
                    uint32_t totalSize = chunk.size;
                } else {
                    if (strcmp(chunkName, "STRG") == 0) {
                        FILE *stringFile = fopen("string.txt", "wb");

                        readList(entryNum, entryOffsets, file);
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

                        readList(entryNum, entryOffsets, file);
                        fprintf(stdout, "%u texture files\n", entryNum);

                        readTarray(TextureAddress, addresses, entryNum, file);

                        // 最後のテクスチャ (entryNum - 1 番目) が読み込めない 仕様？
                        for (int i = 0; i < entryNum; i++) {
                            uint32_t nextOffset = i < entryNum - 1 ? addresses[i + 1].offset : chunkLast;
                            uint32_t entrySize = nextOffset - addresses[i].offset;
                            copyToFile(file, entrySize, "%d.png", i);
                        }

                        chdir("..");
                        fseek(file, chunkLast, SEEK_SET);
                    } else if (strcmp(chunkName, "AUDO") == 0) {
                        mkdir("audio", 0755);
                        chmod("audio", 0755);
                        chdir("./audio");

                        readList(entryNum, entryOffsets, file);
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

                        readList(entryNum, entryOffsets, file);
                        fprintf(stdout, "%u sprites\n", entryNum);

                        SpritePrintCSVHeader(spriteFile);

                        for (int i = 0; i < entryNum - 1; i++) {
                            fseek(file, entryOffsets[i], SEEK_SET);
                            readT(Sprite, sprite, file);
                            readList(textureCount, textureAddresses, file);
                            readStringAt(spriteName, sprite.nameOffset, file);

                            SpritePrintCSV(spriteFile, sprite, spriteName);
                        }

                        fclose(spriteFile);
                        fseek(file, chunkLast, SEEK_SET);
                    } else if (strcmp(chunkName, "FONT") == 0) {
                        mkdir("font", 0755);
                        chmod("font", 0755);
                        chdir("./font");

                        readList(entryNum, entryOffsets, file);
                        fprintf(stdout, "%u fonts\n", entryNum);

                        FILE *fontFile = fopen("font.csv", "wb");
                        FontPrintCSVHeader(fontFile);

                        for (int i = 0; i < entryNum; i++) {
                            fseek(file, entryOffsets[i], SEEK_SET);
                            readT(Font, font, file);
                            readList(glyphCount, glyphOffsets, file);
                            
                            readStringAt(fileName, font.fileNameOffset, file);
                            readStringAt(name, font.nameOffset, file);

                            FontPrintCSV(fontFile, font, fileName, name);

                            // read glyph

                            openFile(glyphFile, "wb", "glyph%d.csv", i);
                            GlyphPrintCSVHeader(glyphFile);

                            for (int n = 0; n < glyphCount; n++) {
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
                        FILE *tileFile = fopen("tile.csv", "wb");

                        RoomPrintCSVHeader(roomFile);
                        BackgroundPrintCSVHeader(bgFile);
                        ViewPrintCSVHeader(viewFile);
                        RoomObjectPrintCSVHeader(objFile);
                        TilePrintCSVHeader(tileFile);

                        readList(entryNum, entryOffsets, file);
                        fprintf(stdout, "%u rooms\n", entryNum);

                        for (int i = 0; i < entryNum; i++) {
                            fseek(file, entryOffsets[i], SEEK_SET);
                            readT(Room, room, file);

                            {
                                readList(num, offsets, file);
                                readTarray(Background, arr, num, file);

                                for (int n = 0; n < num; n++) {
                                    BackgroundPrintCSV(bgFile, arr[n], i);
                                }
                            }

                            {
                                readList(num, offsets, file);
                                readTarray(View, arr, num, file);

                                for (int n = 0; n < num; n++) {
                                    ViewPrintCSV(viewFile, arr[n], i);
                                }
                            }

                            {
                                readList(num, offsets, file);
                                readTarray(RoomObject, arr, num, file);

                                for (int n = 0; n < num; n++) {
                                    RoomObjectPrintCSV(objFile, arr[n], i);
                                }
                            }
                        
                            {
                                readList(num, offsets, file);
                                readTarray(Tile, arr, num, file);

                                for (int n = 0; n < num; n++) {
                                    TilePrintCSV(tileFile, arr[n], i);
                                }
                            }
                        
                            readStringAt(name, room.nameOffset, file);
                            readStringAt(caption, room.captionOffset, file);
                            RoomPrintCSV(roomFile, room, name, caption);
                        }

                        fclose(roomFile);
                        fclose(bgFile);
                        fclose(viewFile);
                        fclose(objFile);
                        fclose(tileFile);
                        chdir("..");
                        fseek(file, chunkLast, SEEK_SET);
                    } else {
                        chdir("./chunk");
                        copyToFile(file, chunk.size, "%s.chunk", chunkName);
                        chdir("..");

                        fprintf(stdout, "skip.\n");
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
