import * as compile from "./utils/compile.js";
import * as serial from "./utils/serialTools.js";
import {updateStatus, reportGameStatus} from "./utils/status.js";
import { unzipSync } from "https://unpkg.com/fflate/esm/browser.js";

let currentProjectInfo = null;
let stateExports = {};
let stateLocals = {};

// Parse project ID from Scratch URL
function parseProjectIDFromURL(url) {
    if (!url || !url.trim()) {
        return null;
    }
    
    // Remove whitespace
    url = url.trim();
    
    // If it's already just a number, return it
    if (/^\d+$/.test(url)) {
        return url;
    }
    
    // Try to extract ID from URL patterns:
    // https://scratch.mit.edu/projects/1238081605/editor/
    // https://scratch.mit.edu/projects/1238081605
    // scratch.mit.edu/projects/1238081605
    const patterns = [
        /scratch\.mit\.edu\/projects\/(\d+)/i,
        /\/projects\/(\d+)/i,
        /projects\/(\d+)/i
    ];
    
    for (const pattern of patterns) {
        const match = url.match(pattern);
        if (match && match[1]) {
            return match[1];
        }
    }
    
    return null;
}

function uint8ToBase64(uint8) {
    let binary = "";
    const chunkSize = 0x8000;
    for (let i = 0; i < uint8.length; i += chunkSize) {
        const chunk = uint8.subarray(i, i + chunkSize);
        binary += String.fromCharCode.apply(null, chunk);
    }
    return btoa(binary);
}


async function unzipFile(file) {
    const buffer = file.arrayBuffer;
    const bytes = new Uint8Array(buffer);
    const unzipped = unzipSync(bytes);
    return unzipped;
}

function sanitizeProjectName(rawName, fallback = "project") {
    const safeFallback = fallback || "project";
    if (!rawName || typeof rawName !== "string") {
        return safeFallback;
    }
    return rawName.replace(/[^a-zA-Z0-9_-]/g, "_").substring(0, 50) || safeFallback;
}

// Download Scratch project by URL and store with project name
async function downloadScratchProject() {
    try {
        const projectURLInput = document.getElementById("projectURLInput");
        
        if (!projectURLInput) {
            throw new Error("Input field not found");
        }
        
        const projectURL = projectURLInput.value.trim();
        
        if (!projectURL) {
            updateStatus("Please enter a Scratch project URL or ID", "warning");
            return;
        }
        
        // Parse project ID from URL
        const projectID = parseProjectIDFromURL(projectURL);
        
        if (!projectID) {
            updateStatus("Invalid Scratch project URL. Please use format: https://scratch.mit.edu/projects/1238081605", "error");
            return;
        }
        
        updateStatus(`Downloading Scratch project ${projectID}...`, "info");
        
        const options = {
            onProgress: (type, loaded, total) => {
                const progress = Math.round((loaded / total) * 100);
                updateStatus(`Downloading ${type}: ${progress}%`);
            }
        };
        
        const project = await SBDL.downloadProjectFromID(projectID, options);
        console.log("Downloaded project:", project);
        
        // Get project name from the downloaded project
        let projectName = sanitizeProjectName(project.title, `project_${projectID}`);
        if (!project.title) {
            updateStatus(`Warning: Could not extract project name, using ${projectName}`, "warning");
        }
        
        let file = await unzipFile(project, projectID);
        currentProjectInfo = {
            id: projectID,
            name: projectName
        };
        
        updateStatus(`✓ Project "${projectName}" (ID: ${projectID}) downloaded and stored successfully!`, "success");
        return compile.compileScratchProject(file);
        
        // Clear the input
        projectURLInput.value = "";
        
    } catch (error) {
        updateStatus(`✗ Error downloading project: ${error.message}`, "error");
        console.error("Download error:", error);
    }
}

function get(DOMselector) {
    return document.querySelector(DOMselector);
}

function enable(DOMelement) {
    DOMelement.style.display = DOMelement.displayOld || DOMelement.style.display;
}

function disable(DOMelement) {
    DOMelement.displayOld = DOMelement.style;
    DOMelement.style.display = "none";
}

let projectDownload = get("#projectDownload");
let serialMenu = get("#serialMenu");
let reportMenu = get("#reportMenu");

console.log(serialMenu);

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
    console.log(currentState, other);
    let switchStatus = states[other]({type: "switch"});
    if (switchStatus !== "ok") {
        states[currentState]({type: "switchFail"});
    }
    else {
        currentState = other;
    }
}

function registerStateUpdate(DOMobj, eventName, typeFilter) {
    DOMobj.addEventListener(
        eventName, 
        (event) => {
            if (typeFilter && typeFilter.includes(event.type)) return;
            updateState({type: "dom", eventName, event});
        }
    )
}

async function main() {
    updateState({type: "switch"});
    registerStateUpdate(projectDownload, "click");
    let textarea = document.getElementById("projectID");
    const options = {
      // May be called periodically with progress updates.
      onProgress: (type, loaded, total) => {
        // type is 'metadata', 'project', 'assets', or 'compress'
        console.log(type, loaded / total);
      }
    };
}

main();
