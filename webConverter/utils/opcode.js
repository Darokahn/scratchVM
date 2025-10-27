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
    if (typeof value === "string") {
        valueAnnotation.type = "OBJECTREF";
        valueAnnotation.value = value;
        return valueAnnotation;
    }
    valueAnnotation.type = definitions[value[0]];
    valueAnnotation.value = value[1];
    return valueAnnotation;
}

export function processField(field) {
    return field;
}

export function processBlock(block) {
    // stripped version of the block that contains everything we need
    let processed = {
        opcode: block.opcode,
        next: block.next,
        parent: block.parent,
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

export function compileBlock(block, code) {
}
