#include <stdint.h>
#include <TFT_eSPI.h>

struct appDescriptor {
    uint16_t offset;
    uint16_t size;
    char name[12];
};

struct hardwareData {
    TFT_config screen;
    struct {
        int8_t vcc;
        int8_t gnd;
        int8_t xAxis;
        int8_t yAxis;
        int8_t button;
    } controls;
};

void printHardwareData(struct hardwareData data) {
    printf(
        "{\n\r"
        "\t.screen = {\n\r"
        "\t\t.cs = %d,\n\r"
        "\t\t.reset = %d,\n\r"
        "\t\t.dc = %d,\n\r"
        "\t\t.sdi = %d,\n\r"
        "\t\t.sck = %d,\n\r"
        "\t\t.led = %d,\n\r"
        "\t\t.sdo = %d,\n\r"
        "\n\r"
        "\t\t.colorOrder = %d,\n\r"
        "\t\t.width = %d,\n\r"
        "\t\t.height = %d,\n\r"
        "\t\t.orientation = %d\n\r"
        "\t},\n\r"
        "\n\r"
        "\t.controls = {\n\r"
        "\t\t.vcc = %d,\n\r"
        "\t\t.gnd = %d,\n\r"
        "\t\t.xAxis = %d,\n\r"
        "\t\t.yAxis = %d,\n\r"
        "\t\t.button = %d\n\r"
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
