import fs from 'fs';

let allblocks = JSON.parse(fs.readFileSync('project.json')).targets;

let reducedblocks = {};

for (let element of allblocks) {
    for (let block of Object.values(element.blocks)) {
        reducedblocks[block.opcode] = [Object.keys(block.inputs), Object.keys(block.fields)];
    }
}

let implFile = "";
let implFileFunctionTable = "SCRATCH_function operations[MAXOPCODE] = {\n";
let opcodeFile = "enum SCRATCH_opcode opcodes {\n";

for (let element in reducedblocks) {
    console.log(element);
    implFile += "SCRATCH_implementFunction(" + element + ") {\n    // [" + reducedblocks[element][0] + "], [" + reducedblocks[element][1] + "]\n}\n";
    implFileFunctionTable += "    [" + element.toUpperCase() + "] = " + element + ";\n";
    opcodeFile += "    " + element.toUpperCase() + ",\n";
}

opcodeFile += "};\n";
implFile += implFileFunctionTable + "};";

console.log(opcodeFile);
