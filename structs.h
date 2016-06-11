typedef struct {
    int32_t width, height;
} Size;

typedef struct {
    int32_t x, y;
} Point;

typedef struct {
    uint32_t left, right, bottom, top;
} Rect;

typedef struct {
    float x, y;
} PointF;

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
    Point position;
} Sprite;

extern void SpritePrintCSVHeader(FILE *file) {
    fprintf(file, "name,width,height,left,top,right,bottom,boundingBoxMode,isSeparateColMasks,x,y\n");
}

extern void SpritePrintCSV(FILE *file, Sprite s, char *name) {
    fprintf(file, "\"%s\",%u,%u,%d,%d,%d,%d,%u,%u,%d,%d\n", 
        name, 
        s.size.width, s.size.height, 
        s.bounds.left, s.bounds.top, s.bounds.right, s.bounds.bottom, 
        s.boundingBoxMode, s.isSeparateColMasks, s.position.x, s.position.y);
}

typedef struct {
    uint32_t fileNameOffset;
    uint32_t nameOffset;
    uint32_t pointSize;
    uint32_t isBold;
    uint32_t isItalic;
    char antiAliasMode;
    char charset;
    int32_t texturePageId;
    PointF scale;
    uint16_t unknown;
} Font;

extern void FontPrintCSVHeader(FILE *file) {
    fprintf(file, "fileName,name,pointSize,isBold,isItalic,antiAliasMode,charset,texturePageId,scaleX,scaleY,unknown\n");
}

extern void FontPrintCSV(FILE *file, Font f, char *fileName, char *name) {
    fprintf(file, "\"%s\",\"%s\",%u,%u,%u,%u,%u,%d,%f,%f,%u\n", 
        fileName, name, f.pointSize, f.isBold, f.isItalic, f.antiAliasMode, f.charset, f.texturePageId, f.scale.x, f.scale.y, f.unknown);
}

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
    int32_t id;
    Point position;
    uint32_t isTileX;
    uint32_t isTileY;
    Point speed;
    uint32_t isStretch;
} Background;

extern void BackgroundPrintCSVHeader(FILE *file) {
    fprintf(file, "room,isEnabled,isForeground,id,positionX,positionY,isTileX,isTileY,speedX,speedY,isStretch\n");
}

extern void BackgroundPrintCSV(FILE *file, Background b, int room) {
    fprintf(file, "%d,%u,%u,%d,%d,%d,%u,%u,%d,%d,%u\n", 
        room,
        b.isEnabled, b.isForeground, b.id, 
        b.position.x, b.position.y, 
        b.isTileX, b.isTileY,
        b.speed.x, b.speed.y,
        b.isStretch);
}

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
    uint32_t color; // argb
    uint32_t isDrawBackgroundColor;
    int32_t unknown;
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

extern void RoomPrintCSVHeader(FILE *file) {
    fprintf(file, "name,caption,sizeWidth,sizeHeight,speed,isPersistent,color,isDrawBackgroundColor,unknown,flags,world,boundsLeft,boundsRight,boundsBottom,boundsTop,gravityX,gravityY,metersPerPixel\n");
}

extern void RoomPrintCSV(FILE *file, Room r, char *name, char *caption) {
    fprintf(file, "%s,%s,%u,%u,%u,%u,%x,%u,%d,%u,%u,%u,%u,%u,%u,%f,%f,%f\n",
        name, caption,
        r.size.width, r.size.height,
        r.speed, r.isPersistent,
        r.color, r.isDrawBackgroundColor, 
        r.unknown, r.flags, r.world,
        r.bounds.left, r.bounds.right, r.bounds.bottom, r.bounds.top,
        r.gravity.x, r.gravity.y,
        r.metersPerPixel);
}

typedef struct {
    Point position;
    int32_t id;
    int32_t instanceId;
    int32_t createCodeId;
    PointF scale;
    uint32_t color; // argb
    float rotation;
} RoomObject;

extern void RoomObjectPrintCSVHeader(FILE *file) {
    fprintf(file, "room,positionX,positionY,id,instanceId,createCodeId,scaleX,scaleY,color,rotation\n");
}

extern void RoomObjectPrintCSV(FILE *file, RoomObject o, int room) {
    fprintf(file, "%d,%d,%d,%d,%d,%d,%f,%f,%x,%f\n", 
        room,
        o.position.x, o.position.y,
        o.id, o.instanceId, o.createCodeId,
        o.scale.x, o.scale.y,
        o.color, o.rotation);
}

typedef struct {
    float density, restitution, group, linearDamping, angularDamping, unknown1, friction, unknown2, kinematic;
} ObjectPhysics;

typedef struct {
    uint32_t nameOffset;
    uint32_t spriteId;
    uint32_t isVisible;
    uint32_t isSolid;
    int depth;
    uint32_t isPersistent;
    int parentId;
    int maskId;
    uint32_t hasPhysics;
    uint32_t isSensor;
    uint32_t collisionShape;
    ObjectPhysics physics;
} GameObject;

typedef struct {
    Point position;
    int32_t id;
    Point sourcePosition;
    Size size;
    uint32_t depth;
    int32_t instanceId;
    PointF scale;
    uint32_t color;
} Tile;

extern void TilePrintCSVHeader(FILE *file) {
    fprintf(file, "room,x,y,id,sourcePositionX,sourcePositionY,width,height,depth,instanceId,scaleX,scaleY,color\n");
}

extern void TilePrintCSV(FILE *file, Tile o, int room) {
    fprintf(file, "%d,%d,%d,%d,%d,%d,%u,%u,%u,%d,%f,%f,%x\n", 
        room,
        o.position.x, o.position.y,
        o.id, 
        o.sourcePosition.x, o.sourcePosition.y,
        o.size.width, o.size.height,
        o.depth, o.instanceId,
        o.scale.x, o.scale.y,
        o.color);
}

typedef struct {
    uint32_t isEnabled;
    Point position;
    Size size;
    Point portPosition;
    Size portSize;
    Point border;
    Point speed;
    int32_t objectId;
} View;

extern void ViewPrintCSVHeader(FILE *file) {
    fprintf(file, "room,isEnabled,positionX,positionY,sizeWidth,sizeHeight,portPositionX,portPositionY,portSizeWidth,portSizeHeight,borderX,borderY,speedX,speedY,objectId\n");
}

extern void ViewPrintCSV(FILE *file, View v, int room) {
    fprintf(file, "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%d,%d,%d\n",
        room,
        v.isEnabled,
        v.position.x,
        v.position.y,
        v.size.width,
        v.size.height,
        v.portPosition.x,
        v.portPosition.y,
        v.portSize.width,
        v.portSize.height,
        v.border.x,
        v.border.y,
        v.speed.x,
        v.speed.y,
        v.objectId);
}

typedef struct {
    uint32_t nameOffset;
    uint32_t size;
} Script;

typedef struct {
    uint32_t nameOffset;
    uint32_t flags;
    uint32_t fileTypeOffset;
    uint32_t fileNameOffset;
    uint32_t unknown;
    float volume;
    float pitch;
    int32_t groupID;
    int32_t audioID;
} Sound;

extern void SoundPrintCSVHeader(FILE *file) {
    fprintf(file, "nameOffset,flags,fileType,fileName,unknown,volume,pitch,groupID,audioID\n");
}

extern void SoundPrintCSV(FILE *file, Sound s, char *name, char *fileType, char *fileName) {
    fprintf(file, "%s,%u,%s,%s,%u,%f,%f,%d,%d\n", name, s.flags, fileType, fileName, s.unknown, s.volume, s.pitch, s.groupID, s.audioID);
}
