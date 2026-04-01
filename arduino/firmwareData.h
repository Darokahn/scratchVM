struct appDescriptor {
    uint16_t offset;
    uint16_t size;
    char name[12];
};

struct hardwareData {
    // properties
    int colorOrder;
    struct {
        int cs;
        int reset;
        int dc;
        int sdi;
        int sck;
        int led;
        int sdo;

        int colorOrder;
        int width;
        int height;
        int orientation;
    } screen;
    struct {
        int vcc;
        int gnd;
        int xAxis;
        int yAxis;
        int button;
    } controls;
};

struct firmwareHeader {
    uint16_t version;
    uint16_t appCount;
    uint16_t nextOffset;
    char magic[6];
    struct appDescriptor apps[];
};
