// template for the object representing the runtime of a stack of blocks
function threadTemplate() {
    return {
        startEvent: "" // The triggering event
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
        rotationStyle: true,
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
        sprites: [],
        images: [],
        code: []
    };
}

// get the sb3 file from the operation layer
function getFile(name) {
}

// extract relevant details from the sb3 file and load them into a details template
function getDetails(sb3File) {
}

// convert table-formatted details into a blob to be loaded
function convertDetails(details) {
}

// Replace the program data of a binary with the loaded data in the blob
function patchBinary(blob) {
}
