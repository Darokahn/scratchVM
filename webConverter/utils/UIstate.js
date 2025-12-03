// state machine ABI:
// 

let stateExports = {};
let stateLocals = {};

let states = {
    awaitingProject: async function (updateEvent) {
    },

    webVmTesting: async function (updateEvent) {
        let necessaryImports = {};
    },

    awaitingConnection: async function (updateEvent) {
        let necessaryImports = {};
    },

    awaitingUpload: async function (updateEvent) {
        let necessaryImports = {};
    },

    awaitingFeedback: async function (updateEvent) {
        let necessaryImports = {};
    },
};

function updateState(updateEvent) {
    states[currentState](updateEvent);
}

function registerStateUpdate(DOMobj, eventName, updateArg) {
}

let currentState = "awaitingProject";
function switchState(other) {
    let switchStatus = states[other]({type: "switch"});
    if (switchStatus !== "ok") {
        states[currentState]({type: "switchFail"});
    }
    else {
        currentState = other;
    }
}
