#include <stdint.h>

struct appDescriptor {
    uint16_t offset;
    uint16_t size;
    char name[12];
};

struct hardwareData {
    struct {
        uint8_t cs;
        uint8_t reset;
        uint8_t dc;
        uint8_t sdi;
        uint8_t sck;
        uint8_t led;
        uint8_t sdo;

        uint8_t colorOrder;
        uint16_t width;
        uint16_t height;
        uint8_t orientation;
    } screen;
    struct {
        uint8_t vcc;
        uint8_t gnd;
        uint8_t xAxis;
        uint8_t yAxis;
        uint8_t button;
    } controls;
};

void printHardwareData(struct hardwareData data) {
    printf(
        "{\n\r"
        "\t.screen = {\n\r"
        "\t\t.cs = %u,\n\r"
        "\t\t.reset = %u,\n\r"
        "\t\t.dc = %u,\n\r"
        "\t\t.sdi = %u,\n\r"
        "\t\t.sck = %u,\n\r"
        "\t\t.led = %u,\n\r"
        "\t\t.sdo = %u,\n\r"
        "\n\r"
        "\t\t.colorOrder = %u,\n\r"
        "\t\t.width = %u,\n\r"
        "\t\t.height = %u,\n\r"
        "\t\t.orientation = %u\n\r"
        "\t},\n\r"
        "\n\r"
        "\t.controls = {\n\r"
        "\t\t.vcc = %u,\n\r"
        "\t\t.gnd = %u,\n\r"
        "\t\t.xAxis = %u,\n\r"
        "\t\t.yAxis = %u,\n\r"
        "\t\t.button = %u\n\r"
        "\t}\n\r"
        "}\n\r",
        data.screen.cs,
        data.screen.reset,
        data.screen.dc,
        data.screen.sdi,
        data.screen.sck,
        data.screen.led,
        data.screen.sdo,
        data.screen.colorOrder,
        data.screen.width,
        data.screen.height,
        data.screen.orientation,
        data.controls.vcc,
        data.controls.gnd,
        data.controls.xAxis,
        data.controls.yAxis,
        data.controls.button
    );
}

// Kept for backward compatibility with older versions that did not describe their own hardware.
// If `magic` holds "magic", this is an old version without the hardware struct.
struct firmwareHeaderOld {
    uint16_t version;
    uint16_t appCount;
    uint16_t nextOffset;
    char magic[6];
    struct appDescriptor apps[];
};


struct firmwareHeader {
    char magic[16];
    char version[16];
    char initialVersion[16];
    uint16_t appCount;
    uint16_t nextOffset;
    struct hardwareData hardware;
    struct appDescriptor apps[];
};
