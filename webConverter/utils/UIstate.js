import * as compile from "./utils/compile.js";

let stateExports = {};
let stateLocals = {};

function get(DOMselector) {
    return document.querySelector(DOMselector);
}

function enable(DOMelement) {
    DOMelement.hidden = false;
}

function disable(DOMelement) {
    DOMelement.hidden = true;
}

let projectDownload = get("#projectDownload");
let serialMenu = get("#serialMenu");
let reportMenu = get("#reportMenu");

let states = {
    awaitingProject: async function (updateEvent) {
        if (updateEvent.type == "switch" || updateEvent.type == "restore") {
            stateLocals = {};
            enable(projectDownload);
            disable(serialMenu);
            disable(reportMenu);
        }
        else if (updateEvent.type == "dom") {
            switchState("awaitingConnection");
        }
        else if (updateEvent.type == "switchFail") {
            updateState("restore");
        }
    },


    webVmTesting: async function (updateEvent) {
        if (updateEvent.type == "switch") {
            stateLocals = {};
        }
    },


    awaitingConnection: async function (updateEvent) {
    },


    awaitingUpload: async function (updateEvent) {
    },


    awaitingFeedback: async function (updateEvent) {
    },
};

function updateState(updateEvent) {
    states[currentState](updateEvent);
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

function registerStateUpdate(DOMobj, eventName, updateArg) {
    DOMobj.addEventListener(
        eventName, 
        (event) => {
            updateState({type: "dom", eventName, event});
        }
    )
}
