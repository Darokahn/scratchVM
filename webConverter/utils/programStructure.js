import mime from 'https://cdn.skypack.dev/mime'; 

const SCRATCHWIDTH = 480;
const SCRATCHHEIGHT = 360;

const structImageSize = 258;
const imageNameSize = 256;

async function scaleImage(pixels, width, height, targetWidth, targetHeight) {
    // pixels: RGBA8888
    // result: RGB888
    let scaledImage = new Uint8Array(targetWidth * targetHeight * 3);
    let xStride = (width / targetWidth);
    let yStride = (height / targetHeight);
    let rowLengthSource = width;
    let rowLengthScaled = targetWidth;
    for (let y = 0; y < targetHeight; y++) {
        for (let x = 0; x < targetWidth; x ++) {
            let sourceIndex = Math.trunc(y * yStride) * rowLengthSource + Math.trunc(x * xStride);        // index into an RGBA array
            let scaledIndex = y * rowLengthScaled + x;                               // index into an RGB array
            sourceIndex *= 4;
            scaledIndex *= 3;
            let alpha = pixels[sourceIndex] + 3;
            scaledImage[scaledIndex] = pixels[sourceIndex];
            scaledImage[scaledIndex+1] = pixels[sourceIndex+1];
            scaledImage[scaledIndex+2] = pixels[sourceIndex+2];

            // replace (0, 0, 0) with (1, 1, 1) so (0, 0, 0) is reserved for transparency
            let sum = (scaledImage[scaledIndex] + scaledImage[scaledIndex + 1] + scaledImage[scaledIndex + 2]);
            let isBlack = sum == 0;
            if (isBlack && alpha != 0) {
                scaledImage[scaledIndex] = 1;
                scaledImage[scaledIndex + 1] = 1;
                scaledImage[scaledIndex + 2] = 1;
            }
            else if (alpha == 0) {
                scaledImage[scaledIndex] = 0;
                scaledImage[scaledIndex + 1] = 0;
                scaledImage[scaledIndex + 2] = 0;
            }
        }
    }
    return scaledImage;
}

function drawRGB888ToCanvas(rgbArray, width, height) {
    if (rgbArray.length !== width * height * 3) {
        throw new Error("Array length does not match width*height*3");
    }

    // Create a canvas
    const canvas = document.createElement("canvas");
    canvas.width = width;
    canvas.height = height;
    const ctx = canvas.getContext("2d");

    // Create an ImageData object (RGBA)
    const imageData = ctx.createImageData(width, height);
    const data = imageData.data;

    // Convert RGB array to RGBA
    for (let i = 0, j = 0; i < rgbArray.length; i += 3, j += 4) {
        data[j] = rgbArray[i];       // R
        data[j + 1] = rgbArray[i + 1]; // G
        data[j + 2] = rgbArray[i + 2]; // B
        data[j + 3] = 255;           // A
    }

    // Put the image data on the canvas
    ctx.putImageData(imageData, 0, 0);

    return canvas;
}

function RGB888to565(uint8Array) {
    let result = new Uint16Array(uint8Array.length / 3);
    for (let i = 0; i < result.length; i++) {
        let sourceIndex = i * 3;
        let red = uint8Array[sourceIndex];
        let green = uint8Array[sourceIndex + 1];
        let blue = uint8Array[sourceIndex + 2];
        red = Math.floor(red * 31 / 255);
        green = Math.floor(green * 63 / 255);
        blue = Math.floor(blue * 31 / 255);
        let finalColor = red << 11 | green << 5 | blue;
        result[i] = finalColor;
    }
    return result;
}

function drawRGB565ToCanvas(rgb565Array, width, height) {
    if (rgb565Array.length !== width * height) {
        throw new Error("Array length does not match width*height");
    }

    const canvas = document.createElement("canvas");
    canvas.width = width;
    canvas.height = height;
    const ctx = canvas.getContext("2d");
    const imageData = ctx.createImageData(width, height);
    const data = imageData.data;

    for (let i = 0; i < rgb565Array.length; i++) {
        const value = rgb565Array[i];

        // Extract R, G, B components
        const r5 = (value >> 11) & 0x1F; // 5 bits red
        const g6 = (value >> 5) & 0x3F;  // 6 bits green
        const b5 = value & 0x1F;         // 5 bits blue

        // Convert to 8-bit per channel
        const r8 = (r5 << 3) | (r5 >> 2); // replicate high bits to low
        const g8 = (g6 << 2) | (g6 >> 4);
        const b8 = (b5 << 3) | (b5 >> 2);

        const j = i * 4;
        data[j] = r8;
        data[j + 1] = g8;
        data[j + 2] = b8;
        data[j + 3] = 255; // fully opaque
    }

    ctx.putImageData(imageData, 0, 0);
    return canvas;
}

async function getScaledImageFromFile(directory, filename, isStage) {
    let resolution = isStage * 128 + !isStage * 32;
    let file = directory[filename];
    let type = mime.getType(filename);
    let {pixels, width, height} = await drawAndGetPixels(file, type);
    let scaledImage = await scaleImage(pixels, width, height, resolution, resolution);
    return {scaledImage, width, height};
}

async function drawAndGetPixels(uint8arr, mimeType) {
    // mimeType: "image/png", "image/jpeg", or "image/svg+xml"
    const blob = new Blob([uint8arr], { type: mimeType });
    const url = URL.createObjectURL(blob);

    try {
        const img = new Image();
        img.src = url;
        await img.decode();

        const canvas = document.createElement("canvas");
        canvas.width = img.width;
        canvas.height = img.height;

        const ctx = canvas.getContext("2d");
        ctx.drawImage(img, 0, 0);

        const pixels = ctx.getImageData(0, 0, Math.max(canvas.width, 1), Math.max(canvas.height, 1)).data;

        return { pixels, width: canvas.width, height: canvas.height };
    } finally {
        URL.revokeObjectURL(url);
    }
}

export async function getImageBuffer(file, details) {
    let totalSize = 0;
    let spriteSize = 128*128*2;
    for (let sprite of details.sprites) {
        for (let image of sprite.costumes) {
            totalSize += structImageSize + spriteSize;
        }
        spriteSize = 32*32*2;
    }
    let buffer = new Uint8Array(totalSize);
    spriteSize = 128*128*2;
    let spriteWidth = 128;
    let isStage = true;
    let index = 0;
    for (let sprite of details.sprites) {
        for (let image of sprite.costumes) {
            // get the image data from the file
            let {scaledImage, width, height} = await getScaledImageFromFile(file, image.md5ext, isStage);
            let array = scaledImage;
            array = RGB888to565(array);

            if (isStage) {
                width = SCRATCHWIDTH;
                height = SCRATCHHEIGHT;
            }

            let widthRatio = Math.min(255 * width / SCRATCHWIDTH, 255);
            let heightRatio = Math.min(255 * height / SCRATCHHEIGHT, 255);
            let canvas = drawRGB565ToCanvas(array, spriteWidth, spriteWidth);

            document.body.appendChild(canvas);
            let costumeName = new TextEncoder().encode(image.name);
            let costumeNameBuffer = new Uint8Array(imageNameSize);
            costumeNameBuffer.set(costumeName);
            buffer.set(new Uint8Array([widthRatio, heightRatio]), index);
            index += 2;
            buffer.set(costumeNameBuffer, index);
            index += imageNameSize;
            buffer.set(new Uint8Array(array.buffer), index);
            index += array.byteLength;
        }
        isStage = false;
        spriteWidth = 32;
        spriteSize = 32*32;
    }
    return buffer;
}

export function getImageBufferAsCarray(buffer) {
    return ["uint8_t imageBuffer[] = {", buffer.join(", "), "}\n;"].join("");
}
