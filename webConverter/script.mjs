import { unzipSync } from "fflate";
import fs from "fs"

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
        rotationStyle: "",
        costumeIndex: 0,
        costumeMax: 0,
        threadCount: 0,
        variableCount: 0,
        threads: []
    };
}

// template for the object representing a whole game
function detailsTemplate() {
    return {
        unzippedFile: null,
        sprites: [],
        images: [],
        code: []
    };
}

// get the sb3 file from the operation layer
function getFile(name) {
    let file = fs.readFileSync(name);
    return file;
}

// get the static binary for patching
function getBinary() {
    let file = fs.readFileSync("a.out");
    return file;
}

// extract relevant details from the sb3 file and load them into a details template
function getDetails(sb3File) {
    let details = detailsTemplate();
    console.log(sb3File);
    let unzipped = unzipSync(sb3File);
    details.unzippedFile = unzipped;
    let projectJson = JSON.parse(new TextDecoder("utf-8").decode(unzipped["project.json"])); // I hate javascript
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
            details.images.push(filename);
        }
        details.sprites.push(sprite);
    }
    console.log(details);
}

// convert table-formatted details into a blob to be loaded
function convertDetails(details) {
}

// Replace the program data of a binary with the loaded data in the blob
function patchBinary(blob) {
}

getDetails(getFile("All Basic Blocks.sb3"));
