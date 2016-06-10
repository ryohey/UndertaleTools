typedef struct {
    char a, r, g, b;
} Color;

extern void ColorPrintJSON(FILE *file, Color c) {
    fprintf(file, "{ \"a\":%d, \"r\":%d, \"g\":%d, \"b\":%d }\n", c.a, c.r, c.g, c.b);
}

typedef struct {
    uint32_t width, height;
} Size;

extern void SizePrintJSON(FILE *file, Size s) {
    fprintf(file, "{ \"width\":%u, \"height\":%u }\n", s.width, s.height);
}

typedef struct {
    uint32_t x, y;
} Point;

extern void PointPrintJSON(FILE *file, Point p) {
    fprintf(file, "{ \"x\":%f, \"y\":%f }\n", p.x, p.y);
}

typedef struct {
    uint32_t left, right, bottom, top;
} Rect;

extern void RectPrintJSON(FILE *file, Rect r) {
    fprintf(file, "{ \"top\":%u, \"left\":%u, \"right\":%u, \"bottom\":%u }\n", r.top, r.left, r.right, r.bottom);
}

typedef struct {
    float x, y;
} PointF;

extern void PointFPrintJSON(FILE *file, PointF p) {
    fprintf(file, "{ \"x\":%f, \"y\":%f }\n", p.x, p.y);
}

typedef struct {
    char name[4];
    uint32_t size;
} Chunk;

typedef struct {
    uint32_t padding;
    uint32_t offset;
} TextureAddress;

typedef struct {
    uint32_t nameOffset;
    Size size;
    Rect bounds;
    uint32_t boundingBoxMode;
    uint32_t isSeparateColMasks;
    Point origin;
    uint32_t textureCount;
} Sprite;

typedef struct {
    uint32_t fileNameOffset;
    uint32_t nameOffset;
    uint32_t pointSize;
    uint32_t isBold;
    uint32_t isItalic;
    char antiAliasMode;
    char charset;
    uint32_t texturePageId;
    PointF scale;
    char unknown[2];
    uint32_t glyphCount;
} Font;

typedef struct {
    uint8_t keyCode;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t shift;
    uint32_t offset;
} Glyph;

extern void GlyphPrintCSVHeader(FILE *file) {
    fprintf(file, "keyCode,x,y,width,height,shift,offset\n");
}

extern void GlyphPrintCSV(FILE *file, Glyph g) {
    fprintf(file, "%u,%u,%u,%u,%u,%u,%u\n", g.keyCode, g.x, g.y, g.width, g.height, g.shift, g.offset);
}

typedef struct {
    uint32_t nameOffset;
    uint32_t padding[3];
    uint32_t textureAddress;
} BackgroundDefinition;

typedef struct {
    uint32_t isEnabled;
    uint32_t isForeground;
    uint32_t id;
    Point position;
    Point tilePosition;
    Point speed;
    uint32_t isStretch;
} Background;

typedef struct {
    uint32_t nameOffset;
    uint32_t id;
} ScriptDefinition;

typedef struct {
    uint32_t nameOffset;
    uint32_t spriteIndex;
} GameObjectDefinition;

typedef struct {
    uint32_t nameOffset;
    uint32_t captionOffset;
    Size size;
    uint32_t speed;
    uint32_t isPersistent;
    Color color;
    uint32_t isDrawBackgroundColor;
    uint32_t padding;
    uint32_t flags;
    uint32_t backgroundOffset;
    uint32_t viewOffset;
    uint32_t objectOffset;
    uint32_t tileOffset;
    uint32_t world;
    Rect bounds;
    PointF gravity;
    float metersPerPixel;
} Room;

extern void RoomPrintJSON(FILE *file, Room r, char *name, char *caption) {
}

typedef struct {
    Point position;
    uint32_t id;
    uint32_t instanceId;
    uint32_t createCodeId;
    PointF scale;
    Color color;
    float rotation;
} GameObject;

typedef struct {
    Point position;
    uint32_t id;
    Point sourcePosition;
    Size size;
    uint32_t depth;
    uint32_t instanceId;
    PointF scale;
    Color color;
} Tile;

typedef struct {
    uint32_t isEnabled;
    Point position;
    Size size;
    Point portPosition;
    Size portSize;
    Point border;
    Point speed;
    uint32_t objectId;
} View;

typedef struct {
    uint32_t nameOffset;
    uint32_t size;
} Script;
