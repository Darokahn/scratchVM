import { unzipSync } from "https://unpkg.com/fflate/esm/browser.js";
import mime from 'https://cdn.skypack.dev/mime'; 
import * as opcode from "./utils/opcode.js";

const SCRATCHWIDTH = 480;
const SCRATCHHEIGHT = 360;

const files = {};

const offsets = {
    sprite: {
        x: { offset: 0, size: 4 }, // scaledInt32 (high half is whole number, low half is fractional number)
        y: { offset: 4, size: 4 }, // scaledInt32
        rotation: { offset: 8, size: 2 }, // scaled rotation (0 -> 360 maps to 0 -> 655636)
        visible: { offset: 10, size: 1 },
        layer: { offset: 11, size: 1 },
        size: { offset: 12, size: 2 },
        rotationStyle: { offset: 14, size: 1 }, // bool
        costumeIndex: { offset: 15, size: 1 },
        costumeMax: { offset: 16, size: 1 },
        threadCount: { offset: 17, size: 1 },
        variableCount: { offset: 18, size: 1 },
        id: {offset: 19, size: 1},
        sizeof: 20
    },
    thread: {
        eventCondition: { offset: 0, size: 2 }, // enum
        entryPoint: { offset: 2, size: 2 },
        startEvent: { offset: 4, size: 2 }, // enum
        sizeof: 6
    }
};

// template for the object representing the runtime of a stack of blocks
function threadTemplate() {
    return {
        startEvent: 0, // The triggering event
        eventCondition: 0, // The parameter that must match for the triggering event to follow through
        entryPoint: 0,
    };
}


// template for the object representing each game object
function spriteTemplate() {
    return {
        id: "",
        index: 0,
        tempThreads: [],
        struct: {
            x: 0,
            y: 0,
            rotation: 0,
            visible: true,
            layer: 0,
            size: 128,
            rotationStyle: 0,
            costumeIndex: 0,
            costumeMax: 0,
            threadCount: 0,
            variableCount: 0,
            id: 0,
            threads: []
        },
    }
}

// template for the object representing a whole game
function detailsTemplate() {
    return {
        unzippedFile: null,
        messages: {},
        backdropCount: 0,
        sprites: [],
        stageImages: [],
        spriteImages: [],
        code: []
    };
}

function toScaledInt32(x) {
    // Clamp and convert to 16-bit signed
    x = (x << 16) >> 16;
    return x << 16;
}

function degreesToScaled16(degrees) {
    // Normalize degrees to [0, 360)
    degrees = ((degrees % 360) + 360) % 360;

    // Scale to 0–65536 range (wraps naturally in 16-bit)
    let scaled = Math.round((degrees / 360) * 65536) & 0xFFFF;

    return scaled;
}

// get the sb3 file from the operation layer
function getFsEntry(name) {
    return files[name];
}

// get the static binary for patching
async function getBinary() {
    let file = await fetch("a.out");
    return file;
}

// extract relevant details from the sb3 file and load them into a details template
function getDetails(project) {
    let details = detailsTemplate();
    details.unzippedFile = project;
    let projectJson = JSON.parse(new TextDecoder("utf-8").decode(project["project.json"])); // I hate javascript
    for (let [index, target] of projectJson.targets.entries()) {
        let key = target.name;
        let sprite = spriteTemplate();
        sprite.name = key;
        sprite.index = index;
        sprite.struct.id = index;
        sprite.struct.variableCount = Object.entries(target.variables).length;
        sprite.struct.x = target.x;
        sprite.struct.y = target.y;
        sprite.struct.size = target.size;
        sprite.struct.rotation = target.direction;
        sprite.struct.visible = target.visible;
        sprite.struct.costumeIndex = target.currentCostume;
        sprite.struct.costumeMax = target.costumes.length;
        sprite.struct.rotationStyle = target.rotationStyle;
        for (let costume of target.costumes) {
            let filename = costume.assetId + "." + costume.dataFormat;
            if (target.isStage) details.stageImages.push(filename);
            else details.spriteImages.push(filename);
        }
        adjustSprite(sprite, target.isStage);
        details.sprites.push(sprite);
    }
    details.code = compileSprites(details.sprites, projectJson);
    return details;
}

// Initialize the threads with every block 
function indexThreads(blocks) {
    let ids = [];
    for (let [id, block] of Object.entries(blocks)) {
        console.log(id, block);
        if (block.topLevel && opcode.events.includes(block.opcode)) {
            ids.push(id);
        }
    }
    return ids;
}

function compileSprite(code, sprite, blocks) {
    opcode.processBlocks(blocks);
    let threadIds = indexThreads(blocks);
    console.log("compiling sprite");
    for (let threadId of threadIds) {
        let hat = blocks[threadId];
        let thread = opcode.compileBlocks(hat, blocks, code);
        console.log(thread);
        sprite.struct.threads.push(thread);
    }
    sprite.struct.threadCount = sprite.struct.threads.length;
}

function compileSprites(sprites, projectJson) {
    let code = [];
    for (let sprite of sprites) {
        let blocks = projectJson["targets"][sprite.index]["blocks"]
        compileSprite(code, sprite, blocks);
    }
    return code;
}

// adjust the sprite's parameters to match the quirks of my C representation
function adjustSprite(sprite, isStage) {
    if (isStage) {
        sprite.struct.visible = true;
    }
    sprite.struct.x = toScaledInt32(sprite.struct.x);
    sprite.struct.y = toScaledInt32(sprite.struct.y);
    sprite.struct.rotation = degreesToScaled16(sprite.struct.rotation);
    sprite.struct.rotationStyle = ["left-right", "don't rotate", "all around"].indexOf(sprite.struct.rotationStyle);
    sprite.struct.size = Number(+sprite.struct.size || 0);
}

// copy a sprite into the array's memory the way it will be represented in the memory of the processor
function copyStruct(buffer, offset, struct, name) {
    const view = new DataView(buffer);

    const sizes = {
        1: "Uint8",
        2: "Uint16",
        4: "Uint32",
        8: "Uint64"
    };

    let layout = offsets[name];

    for (let field in layout) {
        if (field == "sizeof") continue;
        let offsetData = layout[field];
        let totalOffset = offset + offsetData.offset;
        let setSize = "set" + sizes[offsetData.size];
        view[setSize](totalOffset, struct[field], true);
    }
}

let PROJECTMAX = 4096 * 100;

async function convertScratchProject() {
    let header = {
        codeLength: 0,
        imageLength: 0,
        spriteCount: 0,
        messageCount: 0,
        backdropCount: 0,
    };
    const file = getFsEntry("project");
    const buffer = new Uint8Array(PROJECTMAX);
    let details = getDetails(file);
    header.backdropCount = details.stageImages.length;
    let index = 0;
    buffer.set(new Uint8Array(details.code), index);
    index += details.code.length;
    index = (index + 7) & ~7;
    header.codeLength = index;
    for (let image of details.stageImages) {
        let {scaledImage, width, height} = await getScaledImageFromFile(file, image, true);
        let array = scaledImage;
        array = RGB888to565(array);
        let canvas = drawRGB565ToCanvas(array, 128, 128);
        document.body.appendChild(canvas);
        buffer.set(new Uint8Array([255, 255]), index);
        index += 2;
        buffer.set(new Uint8Array(array.buffer), index);
        index += array.byteLength;
    }
    for (let image of details.spriteImages) {
        let {scaledImage, width, height} = await getScaledImageFromFile(file, image, false);
        let array = scaledImage;
        let widthRatio = 255 * width /SCRATCHWIDTH;
        let heightRatio = 255 * height /SCRATCHHEIGHT;
        array = RGB888to565(array);
        let canvas = drawRGB565ToCanvas(array, 32, 32);
        document.body.appendChild(canvas);
        buffer.set(new Uint8Array([widthRatio, heightRatio]), index);
        index += 2;
        buffer.set(new Uint8Array(array.buffer), index);
        index += array.byteLength;
    }
    index = (index + 7) & ~7;
    header.imageLength = index - header.codeLength;
    for (let sprite of details.sprites) {
        copyStruct(buffer.buffer, index, sprite.struct, "sprite");
        index += offsets.sprite.sizeof;
        index = (index + 7) & ~7;
        for (let thread of sprite.struct.threads) {
            copyStruct(buffer.buffer, index, thread, "thread");
            index += offsets.thread.sizeof;
            index = (index + 7) & ~7;
        }
    }
    header.spriteCount = details.sprites.length;
    printAsCfile(details, header, buffer);
}

function printAsCfile(details, header, buffer) {
    let totalString = ""
    totalString += (
        "// THIS IS A GENERATED FILE!\n#include <string.h>\n#include <stdlib.h>\n#include \"programData.h\"\n#include \"scratch.h\"\nenum SCRATCH_opcode* code;\n"
    );
    totalString += (
        "const struct SCRATCH_header header = {.spriteCount = " + header.spriteCount + 
        ", .codeLength = " + header.codeLength + 
        ", .imageLength = " + header.imageLength +
        ", .messageCount = " + header.messageCount +
        ", .backdropCount = " + header.backdropCount +
        "};\n"
    );
    totalString += opcode.getCodeAsCarray(details.code);
    totalString += (
        "bool events[" +
        (Object.entries(opcode.inputMap).length + Object.keys(details.messages).length + header.backdropCount + 1) +
        "];\n" +
        "int eventCount = sizeof events" +
        ";\n"
    );
    totalString += ("const uint8_t programData[] = {");
    for (let i = 0; i < PROJECTMAX; i++) {
        totalString += ("0x" + buffer[i].toString(16) + ", ");
    }
    totalString += ("\n};");
    console.log(totalString);
    fetch("upload/definitions.c", {
        method: 'POST',
        headers: {},
        body: totalString
    });
}

async function addZipToFs(file) {
    const buffer = await file.arrayBuffer();
    const bytes = new Uint8Array(buffer);
    const unzipped = unzipSync(bytes);
    files["project"] = unzipped;
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

        const pixels = ctx.getImageData(0, 0, canvas.width, canvas.height).data;

        return { pixels, width: canvas.width, height: canvas.height };
    } finally {
        URL.revokeObjectURL(url);
    }
}

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

function assert(condition, phrase) {
    if (!condition) console.log("ERROR: " + phrase);
}

async function main() {
    const dropzone = document.getElementById("dropzone");

    dropzone.addEventListener("dragover", e => {
      e.preventDefault(); // Required so drop works
      dropzone.style.background = "#eef";
    });

    dropzone.addEventListener("dragleave", () => {
      dropzone.style.background = "";
    });

    dropzone.addEventListener("drop", e => {
      e.preventDefault();
      dropzone.style.background = "";
        
      const file = e.dataTransfer.files[0];
      if (file && file.name.endsWith(".sb3")) {
          addZipToFs(file);
      }
    });

    const submitButton = document.getElementById("submit");
    submitButton.onclick = convertScratchProject;
}

main();
