#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

struct __attribute__((packed)) bitmapHeader {
    uint16_t signature;
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    uint32_t xPPM;
    uint32_t yPPM;
    uint32_t colorsUsed;
    uint32_t importantColors;
};

int height = 0;
int width = 0;

int loadBitmap(int fd, uint8_t* outBuffer) {
    // assume `is` is a filestream which has consumed only the first two bytes of the file.
    struct bitmapHeader b;
    read(fd, (void*) &b, sizeof b);
    if (
            (b.bitsPerPixel != 32 && b.bitsPerPixel != 24) ||   // reject bitmaps that aren't in >= 24-bit color
            b.signature != 19778                                // reject non-bitmaps
        ) {
        return -1;
    }
    int scanDirection = -1;
    if (b.height < 0) {
        b.height *= -1;
        scanDirection *= -1;
    }
    width = b.width;
    height = b.height;
    lseek(fd, b.dataOffset, SEEK_SET);
    unsigned int bytesRead = 0;
    size_t size = width * (b.bitsPerPixel / 8); // calculate a size that will always exhaust at the end of a row
    size = (size + 3) & ~3; // align to a 4-byte boundary
    uint8_t* buffer = (uint8_t*) malloc(size);
    uint8_t* ptr;
    for (int y = 0; y < height; y++) {
        read(fd, (void*) buffer, size);
        ptr = buffer;
        for (int x = 0; x < width; x++) {
            int blue = *ptr++;
            int green = *ptr++;
            int red = *ptr++;
            int alpha = 255;
            if (b.bitsPerPixel == 32) {
                alpha = *ptr++; // skip alpha
            }
            if (alpha) {
                outBuffer[y * width + x] = 1;
            }
            else {
                outBuffer[y * width + x] = 0;
            }
        }
        printf("\n");
        bytesRead += ptr - buffer;
    }
imageFinished:
    free(buffer);
    return 0;
}

int main() {
    int fd = open("font.bmp", O_RDONLY);
    uint8_t bytes[4096];
    loadBitmap(fd, bytes);
    printf("%s", "uint8_t letterBytes[] = {");
    for (int i = 0; i < 94; i++) {
        int xOffset = i * 4;
        for (int y = 4; y >= 0; y--) {
            for (int x = 0; x < 3; x++) {
                printf("%d, ", bytes[y * width + x + xOffset]);
            }
        }
    }
    printf("%s", "};");
}
