const print = console.log.bind(console);

const enums = {
  unobscuredShadow: 1,
  noShadow: 2,
  obscuredShadow: 3,
  NUM: 4,
  POSNUM: 5,
  WHOLENUM: 6,
  INTNUM: 7,
  ANGLENUM: 8,
  COLOR: 9,
  TEXT: 10,
  BROADCAST: 11,
  VAR: 12,
  LIST: 13,
  OBJECTREF: 14,
};

const definitions = {};
for (const [key, value] of Object.entries(enums)) {
  definitions[key] = value;
  definitions[value] = key; // reverse mapping
}

export const inputMap = {
    "up arrow": 0,
    "down arrow": 1,
    "left arrow": 2,
    "right arrow": 3,
    "space": 4
};

export const events = [
    "EVENT_WHENKEYPRESSED",
    "EVENT_WHENBROADCASTRECEIVED",
    "EVENT_WHENBACKDROPSWITCHESTO",
    "CONTROL_START_AS_CLONE",
    "EVENT_WHENFLAGCLICKED",
    "EVENT_WHENTHISSPRITECLICKED",
    "EVENT_WHENGREATERTHAN",
];

// endianness should be supplied as a string, or not passed at all. Current design only respects little-endian.
function toScaledInt32Tuple(number, endianness) {
    if (endianness !== undefined && endianness !== "little") {
        console.error("toScaledInt32Tuple only handles little endian values");
        return undefined;
    }
    let whole = Math.floor(number);
    let fraction = number - whole;


    // fraction is given in scale 0 -> 1
    // I need to transform it to the scale 0 -> 65k (16bit int limit)
    
    fraction *= 65535;
    fraction = Math.round(fraction);

    let components = new Uint16Array([fraction, whole]);
    let componentBytes = new Uint8Array([fraction, fraction >> 8, whole, whole >> 8]); // All params implicitly masked by 8
    return componentBytes;
}

function toBytes(string) {
    const bytes = new TextEncoder().encode(string);
    return bytes;
}

function toCodeLiteral(number, byteSize, endianness) {
    if (endianness !== undefined && endianness !== "little") {
        console.error("toCodeLiteral only handles little endian values");
        return undefined;
    }
    let bytes = [];
    for (let i = 0; i < byteSize; i++) {
        bytes.push(number & 0xff);
        number >>= 8;
    }
    return bytes;
}

let objectIndex = {
    sprites: [],
    broadcasts: [],
    backdrops: [],
    variables: [],
};

function indexObjects(project) {
}

function findSprite(name, blocks, project) {
    return 0;
    // TODO: search index of sprite names and return enumerated position.
    // Create index from project if not present.
}

function findBroadcast(name, blocks, project) {
    return 0;
}

function findBackdrop(name, blocks, project) {
    return 0;
}

function findVariable(name, id, blocks, project) {
    return [0, 0];
}

const pushFuncs = {
    NUM: (input, code) => {
        let number = Number(input.value[0]);
        let opcode = "INNER_PUSHNUMBER";
        code.push(opcode);
        code.push(...toScaledInt32Tuple(number));
    },
    POSNUM: "NUM",
    WHOLENUM: "NUM",
    INTNUM: "NUM",
    ANGLENUM: (input) => {
        console.log("ANGLENUM");
    },
    COLOR: (input) => {
        console.log("COLOR");
    },
    TEXT: (input, code) => {
        if (Number(input.value[0]) !== NaN) {
            return pushFuncs.NUM(input, code);
        }
        code.push("INNER_PUSHTEXT");
        code.push(...toBytes(input.value[0]));
        code.push(0);
    },
    BROADCAST: (input) => {
        console.log("BROADCAST");
    },
    VAR: (input, code) => {
        code.push("INNER_FETCHVAR");
        let [spriteIndex, varIndex] = findVariable(...input.value);
        code.push(...toCodeLiteral(spriteIndex, 2));
        code.push(...toCodeLiteral(varIndex, 2));
    },
    LIST: (input) => {
        console.log("LIST", code);
    },
    OBJECTREF: (input, code, blocks) => {
        let block = blocks[input.value];
        while (block != null) {
            console.log(block);
            compileBlock(block, code, blocks);
            block = blocks[block.next];
        }
    }
}

export function processInput(input) {
    let valueAnnotation = {
        type: null,
        value: null
    };
    let inputType = input[0];
    let value = input[1];
    // shadows aren't important and we don't use them for anything, but keeping track of them helps document the format better.
    let shadow = null;
    if (inputType === definitions.unobscuredShadow) {
        shadow = input[1];
    }
    else if (inputType === definitions.obscuredShadow) {
        shadow = input[2];
    }
    // strings are always references to objects
    if (typeof value === "string") {
        valueAnnotation.type = "OBJECTREF";
        valueAnnotation.value = value;
        return valueAnnotation;
    }
    if (!value) return null;
    valueAnnotation.type = definitions[value[0]];
    valueAnnotation.value = value.slice(1);
    return valueAnnotation;
}

export function processField(field) {
    return field;
    // TODO: figure out the general shape of scratch fields and handle them here.
}

export function processBlock(block) {
    // stripped version of the block that contains everything we need
    block.opcode = block.opcode.toUpperCase();
    let processed = {
        opcode: block.opcode,
        next: block.next,
        parent: block.parent,
        topLevel: block.topLevel,
        inputs: {},
        fields: {},
    };
    for (let [key, value] of Object.entries(block.inputs)) {
        processed.inputs[key] = processInput(value);
    }
    for (let [key, value] of Object.entries(block.fields)) {
        processed.fields[key] = processField(value);
    }
    return processed;
}

export function processBlocks(blocks) {
    for (let [key, value] of Object.entries(blocks)) {
        blocks[key] = processBlock(value);
    }
}

function pushInput(input, code, blocks) {
    let pushFunc = input.type;
    while (typeof pushFunc === "string") {
        pushFunc = pushFuncs[pushFunc];
    }
    if (pushFunc === undefined) {
        console.error("pushFuncs does not contain value for", input);
        return;
    }
    pushFunc(input, code, blocks);
}

function pushField(field, code) {
    console.log("in pushField:", field);
}

function getEventCondition(hat) {
    let defaultFunc = () => {return 0};
    const eventFuncs = {
        EVENT_WHENKEYPRESSED: () => {
            return inputMap[hat.fields.KEY_OPTION[0]];
        },
        EVENT_WHENBROADCASTRECEIVED: () => {
            return findBroadcast(hat.fields.BROADCAST_OPTION[0]);
        },
        EVENT_WHENBACKDROPSWITCHESTO: () => {
            return findBackdrop(hat.fields.BACKDROP[0]);
        },
        CONTROL_START_AS_CLONE: defaultFunc,
        EVENT_WHENFLAGCLICKED: defaultFunc,
        EVENT_WHENTHISSPRITECLICKED: defaultFunc,
        EVENT_WHENGREATERTHAN: defaultFunc,
    };
    return eventFuncs[hat.opcode]();
}

let specialFunctions = {
    CONTROL_REPEAT: (block, code, blocks) => {
        code.push("INNER_LOOPINIT"); // get a loop frame on the loop stack to track repetition
        let loopBegin = code.length;
        pushInput(block.inputs.TIMES, code, blocks);
        code.push("INNER_JUMPIFREPEATDONE");
        let loopBreakPosition = code.length;
        code.push(0, 0); // to be filled with end-of-loop
        pushInput(block.inputs.SUBSTACK, code, blocks);
        code.push("INNER_LOOPINCREMENT");
        code.push("INNER_LOOPJUMP");
        code.push(...toCodeLiteral(loopBegin, 2))
        code[loopBreakPosition] = (code.length & 0xff);
        code[loopBreakPosition + 1] = ((code.length >> 8) & 0xff);
    },
    CONTROL_WAIT: (block, code, blocks) => {
        for (let input of Object.values(block.inputs)) {
            pushInput(input, code, blocks);
        }
        code.push(block.opcode);
        code.push("INNER__WAITITERATION");
    },
    CONTROL_FOREVER: (block, code, blocks) => {
        let beginning = code.length;
        pushInput(block.inputs.SUBSTACK, code, blocks);
        code.push("INNER_LOOPJUMP");
        code.push(...toCodeLiteral(beginning, 2));
    },
    CONTROL_IF: (block, code, blocks) => {
        pushInput(block.inputs.CONDITION, code, blocks);
        code.push("INNER_JUMPIFNOT");
        let index = code.length;
        code.push(0, 0);
        pushInput(block.inputs.SUBSTACK, code, blocks);
        code[index] = (code.length & 0xff);
        code[index + 1] = ((code.length >> 8) & 0xff);
    },
    CONTROL_IF_ELSE: (block, code, blocks) => {
        pushInput(block.inputs.CONDITION, code, blocks);
        code.push("INNER_JUMPIFNOT");
        let elseIndex = code.length;
        code.push(0, 0);
        pushInput(block.inputs.SUBSTACK, code, blocks);
        code.push("INNER_JUMP");
        let endIndex = code.length;
        code.push(0, 0);
        code[elseIndex] = (code.length & 0xff);
        code[elseIndex + 1] = ((code.length >> 8) & 0xff);
        pushInput(block.inputs.SUBSTACK2, code, blocks);
        code[endIndex] = (code.length & 0xff);
        code[endIndex + 1] = ((code.length >> 8) & 0xff);
    },
    LOOKS_COSTUMENUMBERNAME: (block, code) => {
        code.push(block.opcode);
        if (block.fields.NUMBER_NAME[0] == "number") {
            code.push(...toCodeLiteral(0, 2));
        }
        else if (block.fields.NUMBER_NAME[0] == "name") {
            code.push(...toCodeLiteral(1, 2));
        }
    },
    SENSING_KEYOPTIONS: (block, code) => {
        code.push("INNER_PUSHNUMBER")
        code.push(...toScaledInt32Tuple(inputMap[block.fields.KEY_OPTION[0]]));
    },
    MOTION_SETROTATIONSTYLE: (block, code) => {
        code.push(block.opcode);
        code.push(...toCodeLiteral(["left-right", "don't rotate", "all around"].indexOf(block.fields.STYLE[0]), 2));
    },
    MOTION_GOTO_MENU: (block, code) => {
        let to = block.fields.TO[0];
        if (to === undefined) {
            console.error("incorrect assumption about the definite shape of motion_goto_menu. block is", block);
            return;
        }
        code.push("INNER_FETCHPOSITION");
        const fieldValues = {
            _random_: -1,
            _mouse_: -2,
        };
        let fieldValue = fieldValues[to] || findSprite(to);
        code.push(...toCodeLiteral(fieldValue, 2));
    },
    MOTION_GOTO: (block, code, blocks) => {
        for (let input of Object.values(block.inputs)) pushInput(input, code, blocks);
        code.push("MOTION_GOTOXY");
    },
    MOTION_GLIDETO_MENU: (block, code) => {
        let to = block.fields.TO[0];
        if (to === undefined) {
            console.error("incorrect assumption about the definite shape of motion_glideto_menu. block is", block);
            return;
        }
        code.push("INNER_FETCHPOSITION");
        const fieldValues = {
            _random_: -1,
            _mouse_: -2,
        };
        let fieldValue = fieldValues[to] || findSprite(to);
        code.push(...toCodeLiteral(fieldValue, 2));
    },
    MOTION_GLIDESECSTOXY: (block, code) => {
        for (let input of Object.values(block.inputs)) pushInput(input, code, blocks);
        code.push(block.opcode);
        code.push("INNER__GLIDEITERATION");
    },
    MOTION_GLIDETO: (block, code, blocks) => {
        for (let input of Object.values(block.inputs)) pushInput(input, code, blocks);
        code.push("MOTION_GLIDESECSTOXY");
        code.push("INNER__GLIDEITERATION");
    },
    MOTION_XPOSITION: (block, code) => {
        code.push(block.opcode);
        code.push(...toCodeLiteral(-1, 2));
    },
    MOTION_YPOSITION: (block, code) => {
        code.push(block.opcode);
        code.push(...toCodeLiteral(-1, 2));
    },
    DATA_SETVARIABLETO: (block, code, blocks) => {
        for (let input of Object.values(block.inputs)) pushInput(input, code, blocks);
        code.push(block.opcode);
        code.push(...toCodeLiteral(-1, 2));
        code.push(...toCodeLiteral(findVariable(...block.fields.VARIABLE), 2));
    }
};

export function compileBlock(block, code, blocks) {
    let specialFunction = specialFunctions[block.opcode];
    if (specialFunction !== undefined) {
        specialFunction(block, code, blocks);
    }
    else {
        for (let input of Object.values(block.inputs)) {
            pushInput(input, code, blocks);
        }
        code.push(block.opcode);
        if (Object.keys(block.fields).length > 0) {
            console.log(block);
        }
        for (let field of Object.values(block.fields)) {
            pushField(field, code);
        }
    }
}

export function compileBlocks(hat, blocks, code) {
    let entryPoint = code.length;
    let startEvent = events.indexOf(hat.opcode);
    let eventCondition = getEventCondition(hat);
    let block = blocks[hat.next];
    while (block != null) {
        compileBlock(block, code, blocks);
        block = blocks[block.next];
    }
    code.push("CONTROL_STOP");
    return {entryPoint, startEvent, eventCondition};
}

export function getCodeAsCarray(code) {
    let values = code.join(", ");
    return ["const enum SCRATCH_opcode insertedCode[] = {", values, "};\n"].join("");
}

function printCodeAsCarray(code) {
    console.log(getCodeAsCarray(code));
}
