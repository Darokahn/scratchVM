import { unzipSync } from "https://unpkg.com/fflate/esm/browser.js";
import * as opcode from "./utils/opcode.js";
import * as programStructure from "./utils/programStructure.js";

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
        costumes: null,
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
        sprite.costumes = target.costumes;
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
        if (block.topLevel && opcode.events.includes(block.opcode)) {
            ids.push(id);
        }
    }
    return ids;
}

function compileSprite(code, sprite, blocks, project) {
    opcode.processBlocks(blocks);
    let threadIds = indexThreads(blocks);
    for (let threadId of threadIds) {
        let hat = blocks[threadId];
        let thread = opcode.compileBlocks(hat, blocks, code, project);
        sprite.struct.threads.push(thread);
    }
    sprite.struct.threadCount = sprite.struct.threads.length;
}

function compileSprites(sprites, projectJson) {
    let code = [];
    for (let sprite of sprites) {
        let blocks = projectJson["targets"][sprite.struct.id]["blocks"]
        compileSprite(code, sprite, blocks, projectJson);
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
    details.imageBuffer = await programStructure.getImageBuffer(file, details);
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
        "// THIS IS A GENERATED FILE!\n#include <string.h>\n#include <stdlib.h>\n#include \"scratch.h\"\nenum SCRATCH_opcode* code;\nint spriteCount;\nint eventTypeOffsets[__EVENTTYPECOUNT];\nbool inputState[5];\n"
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
    let i = 0;
    totalString += `void initData(const struct SCRATCH_header header, struct SCRATCH_sprite* sprites[SPRITEMAX]) {
    int offsetTotal = 0;
    eventTypeOffsets[ONKEY] = offsetTotal;
    offsetTotal += 5;
    eventTypeOffsets[ONMESSAGE] = offsetTotal;
    offsetTotal += header.messageCount;
    eventTypeOffsets[ONBACKDROP] = offsetTotal;
    offsetTotal += header.backdropCount;
    eventTypeOffsets[ONCLONE] = -1; // ONCLONE is an event, but it is not triggered globally so takes no space in the events array
    offsetTotal += 0;
    eventTypeOffsets[ONFLAG] = offsetTotal;
    offsetTotal += 1;
    eventTypeOffsets[ONCLICK] = -1;
    offsetTotal += 0;
    eventTypeOffsets[ONLOUDNESS] = -1;
    offsetTotal += 0;\n`
    for (let sprite of details.sprites) {
        totalString += `\tsprites[${i}] = SCRATCH_makeNewSprite((struct SCRATCH_spriteHeader){.x = ${sprite.struct.x}, .y = ${sprite.struct.y}, .rotation = ${sprite.struct.rotation}, .visible = ${sprite.struct.visible}, .layer = ${sprite.struct.layer}, .size = ${sprite.struct.size}, .rotationStyle = ${sprite.struct.rotationStyle}, .costumeIndex = ${sprite.struct.costumeIndex}, .costumeMax = ${sprite.struct.costumeMax}, .threadCount = ${sprite.struct.threadCount}, .variableCount = ${sprite.struct.variableCount}, .id=${sprite.struct.id}});\n`
        let j = 0;
        for (let thread of sprite.struct.threads) {
            totalString += `\t\tSCRATCH_initThread(&(sprites[${i}]->threads[${j}]), (struct SCRATCH_threadHeader) {.eventCondition = ${thread.eventCondition}, .entryPoint = ${thread.entryPoint}, .startEvent = ${thread.startEvent}});\n`;
            j++;
        }
        i++;
    }
    totalString += ("\tspriteCount = header.spriteCount;\n");
    totalString += ("}\n");
    totalString += programStructure.getImageBufferAsCarray(details.imageBuffer) + "\n";
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
