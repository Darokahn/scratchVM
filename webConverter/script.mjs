import { unzipSync } from "https://unpkg.com/fflate/esm/browser.js";
import mime from 'https://cdn.skypack.dev/mime'; 

const files = {};

const offsets = {
    sprite: {
        x: { offset: 0, size: 4 },
        y: { offset: 4, size: 4 },
        rotation: { offset: 8, size: 2 },
        visible: { offset: 10, size: 1 },
        layer: { offset: 11, size: 1 },
        size: { offset: 12, size: 2 },
        widthRatio: { offset: 14, size: 1 },
        heightRatio: { offset: 15, size: 1 },
        rotationStyle: { offset: 16, size: 1 },
        costumeIndex: { offset: 17, size: 1 },
        costumeMax: { offset: 18, size: 1 },
        threadCount: { offset: 19, size: 1 },
        variableCount: { offset: 20, size: 1 },
        sizeof: 24
    },
    thread: {
        eventCondition: { offset: 0, size: 2 },
        entryPoint: { offset: 2, size: 2 },
        startEvent: { offset: 4, size: 2 },
        sizeof: 6
    }
};

// template for the object representing the runtime of a stack of blocks
function threadTemplate() {
    return {
        startEvent: "", // The triggering event
        eventCondition: "", // The parameter that must match for the triggering event to follow through
        entryPoint: 0,
    };
}

// template for the object representing each game object
function spriteTemplate() {
    return {
        x: 0,
        y: 0,
        rotation: 0,
        visible: true,
        layer: 0,
        size: 128,
        widthRatio: 0,
        heightRatio: 0,
        rotationStyle: 0,
        costumeIndex: 0,
        costumeMax: 0,
        threadCount: 1,
        variableCount: 0,
        threads: []
    };
}

// template for the object representing a whole game
function detailsTemplate() {
    return {
        unzippedFile: null,
        sprites: [],
        stageImages: [],
        spriteImages: [],
        code: []
    };
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
    for (let target of projectJson.targets) {
        let sprite = spriteTemplate();
        sprite.x = target.x;
        sprite.y = target.y;
        sprite.size = target.size;
        sprite.rotation = target.direction;
        sprite.visible = target.visible;
        sprite.costumeIndex = target.currentCostume;
        sprite.costumeMax = target.costumes.length;
        sprite.rotationStyle = target.rotationStyle;
        for (let costume of target.costumes) {
            let filename = costume.assetId + "." + costume.dataFormat;
            if (target.isStage) details.stageImages.push(filename);
            else details.spriteImages.push(filename);
        }
        details.sprites.push(sprite);
    }
    details.code = [
        32,
        11,
        1,
        30,
        0,
        11,
        1,
        255,
        0,
        11,
        1,
        255,
        10,
        21,
        22,
        32,
        33,
        32,
        33,
    ];
    return details;
}

// adjust the sprite's parameters to match the quirks of my C representation

function adjustSprite(sprite, isStage) {
    for (let i = 0; i < sprite.threadCount; i++) {
        sprite.threads.push(threadTemplate());
    }
    if (isStage) {
        console.log(sprite);
    }

}

// copy a sprite into the array's memory
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
        view[ "set" + sizes[offsetData.size]](totalOffset, struct[field], true);
        let value = view["get" + sizes[offsetData.size]](totalOffset, true);
    }
}

let PROJECTMAX = 4096 * 20;

async function convertScratchProject() {
    let header = {
        codeLength: 0,
        imageLength: 0,
        spriteCount: 0
    };
    const file = getFsEntry("project");
    const buffer = new Uint8Array(PROJECTMAX);
    let details = getDetails(file);
    let isStage = true;
    for (let sprite of details.sprites) {
        adjustSprite(sprite, isStage);
        isStage = false;
    }
    let index = 0;
    buffer.set(new Uint8Array(details.code), index);
    index += details.code.length;
    index = (index + 7) & ~7;
    header.codeLength = index;
    for (let image of details.stageImages) {
        let array = await getScaledImage(file, image, true);
        buffer.set(array, index);
        index += array.byteLength;
    }
    for (let image of details.spriteImages) {
        let array = await getScaledImage(file, image, false);
        buffer.set(array, index);
        index += array.byteLength;
    }
    index = (index + 7) & ~7;
    header.imageLength = index - header.codeLength;
    for (let sprite of details.sprites) {
        console.log("doing sprite");
        copyStruct(buffer.buffer, index, sprite, "sprite");
        index += offsets.sprite.sizeof;
        index = (index + 7) & ~7;
        for (let thread of sprite.threads) {
            console.log("doing thread");
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
        "};\n"
    );
    totalString += ("const uint8_t programData[] = {");
    for (let i = 0; i < PROJECTMAX; i++) {
        totalString += ("0x" + buffer[i].toString(16) + ", ");
    }
    totalString += ("\n};");
    console.log(totalString);
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

async function getScaledImage(directory, filename, isStage) {
    let resolution = isStage * 128 + !isStage * 32;
    let file = directory[filename];
    let type = mime.getType(filename);
    let {pixels, width, height} = await drawAndGetPixels(file, type);
    let scaledImage = new Uint16Array(resolution * resolution);
    let xStride = (width / resolution);
    let yStride = (height / resolution);
    for (let y = 0; y < resolution; y++) {
        for (let x = 0; x < resolution; x++) {
            scaledImage[y * resolution + x] = pixels[Math.trunc(y * resolution * yStride) + Math.trunc(x * xStride)]
        }
    }
    return scaledImage;
}

function main() {
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
