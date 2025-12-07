struct appDescriptor {
    uint16_t offset;
    uint16_t size;
    char name[12];
};

struct firmwareHeader {
    uint16_t version;
    uint16_t appCount;
    uint16_t nextOffset;
    char magic[6];
    struct appDescriptor apps[];
};
