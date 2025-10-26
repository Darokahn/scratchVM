const print = console.log.bind(console);

const shadowTypes = ["unobscuredShadow", "noShadow", "obscuredShadow"];

const ObjectRef = {};
const 

function processInput(input) {
    let valueAnnotation = {
        type: null,
        value: null
    };
    let inputType = shadowTypes[input[0] - 1];
    let value = input[1];
    let shadow = null;
    if (inputType === "unobscuredShadow") {
        shadow = input[1];
    }
    else if (inputType === "obscuredShadow") {
        shadow = input[2];
    }
    if (typeof value === "string") {
        valueAnnotation[type] = "objectRef";
    }
}
