import { unzipSync } from "https://unpkg.com/fflate/esm/browser.js";
import * as opcode from "./utils/opcode.js";
import * as imageDrawing from "./utils/imageDrawing.js";

const SIZERATIO = 1024;

const files = {};
let currentProjectInfo = null;

function sanitizeProjectName(rawName, fallback = "project") {
    const safeFallback = fallback || "project";
    if (!rawName || typeof rawName !== "string") {
        return safeFallback;
    }
    return rawName.replace(/[^a-zA-Z0-9_-]/g, "_").substring(0, 50) || safeFallback;
}

// template for the object representing each game object
function spriteTemplate() {
    return {
        name: "",
        costumes: [],
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
        imageBuffer: null,
        sprites: [],
        code: [],
        objectIndex: {}
    };
}

function toScaledInt32(x) {
    // Clamp and convert to 16-bit signed
    x = (x << 16) >> 16;
    return x << 16;
}

function degreesToScaled32(degrees) {
    degrees %= 360;
    let scaled = (degrees / 360 * (2 ** 32)) & 0xFFFFFFFF;

    return scaled;
}

// get the sb3 file from the operation layer
function getFsEntry(name) {
    return files[name];
}

// extract relevant details from the sb3 file and load them into a details template
async function getDetails(project) {
    let details = detailsTemplate();
    details.unzippedFile = project;
    let projectJson = JSON.parse(new TextDecoder("utf-8").decode(project["project.json"])); // I hate javascript
    for (let [index, target] of projectJson.targets.entries()) {
        let key = target.name;
        let sprite = spriteTemplate();
        sprite.name = target.name
        sprite.costumes = target.costumes;
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
        adjustSprite(sprite, target.isStage);
        details.sprites.push(sprite);
    }
    details.objectIndex = opcode.indexObjects(projectJson);
    details.code = compileSprites(details.sprites, projectJson);
    details.imageBuffer = await imageDrawing.getImageBuffer(project, details);
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
        let thread = opcode.compileBlocks(hat, sprite, blocks, code, project);
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
    sprite.struct.rotation = degreesToScaled32(sprite.struct.rotation);
    sprite.struct.rotationStyle = ["left-right", "don't rotate", "all around"].indexOf(sprite.struct.rotationStyle);
    sprite.struct.size = Number(+sprite.struct.size / 100 * SIZERATIO || 0);
}

function bytesToCarray(bytes, name) {
    console.log(bytes);
    return ["const unsigned char ", name, "[] = {", bytes.join(", "), "};"].join("");
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

function pad(array, align) {
    while ((array.length % align) !== 0) {
        array.push(0);
    }
}

async function convertScratchProject() {
    const file = getFsEntry("project");
    let details = await getDetails(file);
    let bytes = await getProgramAsBlob(details);
    sendFile(new Uint8Array(bytes), "programData.bin");
    sendFile(bytesToCarray(bytes, "programData"), "definitions.c");
}

async function sendFile(blob, name) {
    console.log(name);
    fetch("upload/" + name, {
        method: 'POST',
        headers: {},
        body: blob
    });
}

function pushInt(arr, val, size) {
    for (let i = 0; i < size; i++) {
        arr.push((val >> (i * 8)) & 0xff);
    }
}

// assume little endian and no padding
function toIntStruct(arr, sizes) {
    if (arr.length !== sizes.length) {
        console.error("toIntStruct: lengths don't match");
        return;
    }
    let intStruct = [];
    let index = 0;
    for (let i = 0; i < arr.length; i++) {
        let val = arr[i];
        let size = sizes[i];
        // arrays in the size list encode arrays in the struct
        if (Array.isArray(size)) {
            let unitSize = size[0];
            let count = size[1];
            for (let j = 0; j < count; j++) {
                pushInt(intStruct, val[j], unitSize);
            }
        }
        else {
            pushInt(intStruct, val, size);
        }
    }
    return intStruct;
}

function makeSprite(spriteBase) {
    let sizes = [
        4, 4, 4,
        2,
        1, 1, 1, 1, 1, 1, 1, 1
    ];
    return toIntStruct(
        [
            spriteBase.x, spriteBase.y, spriteBase.rotation,
            spriteBase.size,
            spriteBase.visible, spriteBase.layer, spriteBase.rotationStyle, spriteBase.costumeIndex,
            spriteBase.costumeMax, spriteBase.threadCount, spriteBase.variableCount, spriteBase.id
        ],
        sizes
    );
}

function makeThread(threadBase) {
    let sizes = [2, 2, 1];
    return toIntStruct(
        [threadBase.eventCondition, threadBase.entryPoint, threadBase.startEvent],
        sizes
    );
}

async function getProgramAsBlob(details) {
    console.log(details.code);
    let code = opcode.getCodeAsBuffer(details.code);
    pad(code, 4);
    const enc = new TextEncoder();
    let headerArray = [
        details.sprites.length,
        code.length,
        5,
        Object.keys(details.objectIndex.broadcasts).length,
        Object.keys(details.objectIndex.backdrops).length,
        0,
        0,
        0,
        0,
        0
    ];
    let headerArraySizes = [
        2,
        2,
        2,
        2,
        2,
        2,
        4,
        4,
        4,
        4
    ];
    // just to get a length
    let magicBytes = enc.encode("scratch!");
    let headerStruct = toIntStruct(headerArray, headerArraySizes);
    let spriteBuffer = [];
    let threadBuffer = [];
    details.sprites.forEach(sprite => {
        spriteBuffer.push(...makeSprite(sprite.struct))
        pad(spriteBuffer, 4);
        sprite.struct.threads.forEach( thread => {
            threadBuffer.push(...makeThread(thread));
            pad(threadBuffer, 2);
        });
    });
    headerArray[5] = headerStruct.length;
    headerArray[6] = headerStruct.length + code.length;
    headerArray[7] = headerStruct.length + code.length + spriteBuffer.length;
    headerArray[8] = headerStruct.length + code.length + spriteBuffer.length + threadBuffer.length;

    console.log(headerStruct, code, spriteBuffer, threadBuffer);
    let imageBytes = details.imageBuffer;
    let dataSize = headerStruct.length + code.length + spriteBuffer.length + threadBuffer.length + imageBytes.length;
    headerArray[9] = dataSize;
    // for real this time
    headerStruct = toIntStruct(headerArray, headerArraySizes);
    pad(headerStruct, 4);
    let bytes = [...magicBytes, ...headerStruct, ...code, ...spriteBuffer, ...threadBuffer, ...imageBytes];
    return bytes;
}

async function addZipToFs(file, name = "project") {
    const buffer = file.arrayBuffer;
    const bytes = new Uint8Array(buffer);
    const unzipped = unzipSync(bytes);
    files[name] = unzipped;
    return unzipped;
}

// Global variables for ESP32 functionality
let serialPort = null;
let serialReader = null;

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
        
        await addZipToFs(project, "project");
        currentProjectInfo = {
            id: projectID,
            name: projectName
        };
        
        updateStatus(`✓ Project "${projectName}" (ID: ${projectID}) downloaded and stored successfully!`, "success");
        convertScratchProject();
        
        // Clear the input
        projectURLInput.value = "";
        
    } catch (error) {
        updateStatus(`✗ Error downloading project: ${error.message}`, "error");
        console.error("Download error:", error);
    }
}

// Connect to serial port
async function connectSerial() {
    try {
        // Check if Web Serial API is available
        if (!("serial" in navigator)) {
            throw new Error("Web Serial API not supported in this browser");
        }
        
        updateStatus("Requesting serial port access...", "info");
        
        // Request port access
        serialPort = await navigator.serial.requestPort();
        
        // Open the port with appropriate baud rate for ESP32
        await serialPort.open({ baudRate: 115200 });
        
        updateStatus("✓ Serial port connected successfully", "success");
        
        // Show disconnect button, hide connect button
        const connectBtn = document.getElementById("connectSerial");
        const disconnectBtn = document.getElementById("disconnectSerial");
        if (connectBtn) connectBtn.style.display = "none";
        if (disconnectBtn) disconnectBtn.style.display = "inline-block";
        
        // Set up reader for incoming data (optional, for debugging)
        const textDecoder = new TextDecoderStream();
        const readableStreamClosed = serialPort.readable.pipeTo(textDecoder.writable);
        serialReader = textDecoder.readable.getReader();
        
        // Start reading in background (non-blocking)
        readSerialData();
        
    } catch (error) {
        if (error.name === "NotFoundError") {
            updateStatus("No serial port selected", "warning");
        } else {
            updateStatus(`✗ Error connecting to serial: ${error.message}`, "error");
        }
        console.error("Serial connection error:", error);
        serialPort = null;
    }
}

// Read serial data (background task)
async function readSerialData() {
    if (!serialReader) return;
    
    try {
        while (true) {
            const { value, done } = await serialReader.read();
            if (done) {
                break;
            }
            // Log received data (optional)
            console.log("Serial RX:", new TextDecoder().decode(value));
        }
    } catch (error) {
        console.error("Serial read error:", error);
    }
}

// Reset ESP32 using DTR/RTS signals if available, or send reset command
async function resetESP32() {
    try {
        // Try to use setSignals if available (Web Serial API feature)
        if (serialPort && 'setSignals' in serialPort) {
            try {
                // Toggle DTR to reset ESP32 (DTR low = reset)
                await serialPort.setSignals({ dataTerminalReady: false });
                await new Promise(resolve => setTimeout(resolve, 100)); // Hold reset for 100ms
                await serialPort.setSignals({ dataTerminalReady: true });
                updateStatus("ESP32 reset via DTR signal", "info");
                return;
            } catch (error) {
                console.warn("setSignals not supported or failed, trying alternative method:", error);
            }
        }
        
        // Fallback: Send a reset command sequence
        // Some ESP32 firmwares listen for specific commands
        const writer = serialPort.writable.getWriter();
        try {
            // Send a break or reset sequence
            // Common ESP32 reset: send Ctrl+C (0x03) or specific reset command
            const resetCommand = new Uint8Array([0x03]); // Ctrl+C to interrupt
            await writer.write(resetCommand);
            await new Promise(resolve => setTimeout(resolve, 100));
            updateStatus("ESP32 reset via command", "info");
        } finally {
            writer.releaseLock();
        }
    } catch (error) {
        console.warn("Reset attempt failed, continuing anyway:", error);
        updateStatus("Reset attempt failed, continuing...", "warning");
    }
}

// Disconnect serial port
async function disconnectSerial() {
    try {
        if (serialReader) {
            await serialReader.cancel();
            serialReader = null;
        }
        
        if (serialPort) {
            await serialPort.close();
            serialPort = null;
        }
        
        updateStatus("Serial port disconnected", "info");
        
        // Show connect button, hide disconnect button
        const connectBtn = document.getElementById("connectSerial");
        const disconnectBtn = document.getElementById("disconnectSerial");
        if (connectBtn) connectBtn.style.display = "inline-block";
        if (disconnectBtn) disconnectBtn.style.display = "none";
        
    } catch (error) {
        updateStatus(`✗ Error disconnecting: ${error.message}`, "error");
        console.error("Disconnect error:", error);
    }
}

// Send program data via serial
async function sendProgramDataViaSerial() {
    try {
        if (!serialPort) {
            throw new Error("Serial port not connected. Please connect first.");
        }
        
        // Get the program data from getProgramAsBlob
        const file = getFsEntry("project");
        if (!file) {
            throw new Error("No project loaded. Please load a project first.");
        }
        
        updateStatus("Generating program data...", "info");
        const details = await getDetails(file);
        const bytes = await getProgramAsBlob(details);
        
        if (!bytes || bytes.length === 0) {
            throw new Error("No program data to send");
        }
        
        updateStatus(`Sending ${bytes.length} bytes via serial...`, "info");
        
        // Wait a bit for ESP32 to boot after reset
        await new Promise(resolve => setTimeout(resolve, 1000));
        
        // Convert bytes array to Uint8Array
        const dataToSend = new Uint8Array(bytes);
        
        // Get the writable stream
        const writer = serialPort.writable.getWriter();
        
        try {
            await writer.write(dataToSend);
            updateStatus(`✓ Successfully sent ${dataToSend.length} bytes via serial`, "success");
        } finally {
            // Always release the writer
            writer.releaseLock();
        }
        
    } catch (error) {
        updateStatus(`✗ Error sending data: ${error.message}`, "error");
        console.error("Serial send error:", error);
    }
}

async function reportGameStatus(worked) {
    try {
        const file = getFsEntry("project");
        if (!file) {
            throw new Error("No project loaded. Please download a Scratch project first.");
        }
        
        updateStatus("Preparing report data...", "info");
        const details = await getDetails(file);
        const bytes = await getProgramAsBlob(details);
        const data = new Uint8Array(bytes);
        
        const payload = {
            status: worked ? "worked" : "failed",
            projectId: currentProjectInfo?.id || null,
            projectName: currentProjectInfo?.name || null,
            byteLength: data.length,
            programData: uint8ToBase64(data),
            timestamp: new Date().toISOString()
        };
        
        const response = await fetch("/api/game-status", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(payload)
        });
        
        const result = await response.json().catch(() => ({}));
        if (!response.ok) {
            throw new Error(result.error || "Server error while submitting report");
        }
        
        updateStatus(`Thanks for the feedback! Report ID: ${result.id}`, "success");
    } catch (error) {
        updateStatus(`✗ Could not submit report: ${error.message}`, "error");
        console.error("Game status report error:", error);
    }
}

// Helper function to update status display with styling
function updateStatus(message, type = "info") {
    const statusDiv = document.getElementById("status");
    if (statusDiv) {
        statusDiv.textContent = message;
        
        // Remove all status classes
        statusDiv.classList.remove("status-info", "status-success", "status-error", "status-warning");
        
        // Add appropriate class based on type
        if (type === "success") {
            statusDiv.classList.add("status-success");
        } else if (type === "error") {
            statusDiv.classList.add("status-error");
        } else if (type === "warning") {
            statusDiv.classList.add("status-warning");
        } else {
            statusDiv.classList.add("status-info");
        }
    }
    console.log("Status:", message);
}


async function main() {
    let textarea = document.getElementById("projectID");
    const options = {
      // May be called periodically with progress updates.
      onProgress: (type, loaded, total) => {
        // type is 'metadata', 'project', 'assets', or 'compress'
        console.log(type, loaded / total);
      }
    };

    const sampleProjectId = '1238081605';
    const project = await SBDL.downloadProjectFromID(sampleProjectId, options);
    console.log(project);
    addZipToFs(project);
    const defaultName = sanitizeProjectName(project.title, `project_${sampleProjectId}`);
    currentProjectInfo = {
        id: sampleProjectId,
        name: defaultName
    };

    const connectSerialBtn = document.getElementById("connectSerial");
    if (connectSerialBtn) {
        connectSerialBtn.onclick = connectSerial;
    }
    
    const sendProgramDataBtn = document.getElementById("sendProgramData");
    if (sendProgramDataBtn) {
        sendProgramDataBtn.onclick = sendProgramDataViaSerial;
    }
    
    const disconnectSerialBtn = document.getElementById("disconnectSerial");
    if (disconnectSerialBtn) {
        disconnectSerialBtn.onclick = disconnectSerial;
    }
    
    const reportWorkedBtn = document.getElementById("reportGameWorked");
    if (reportWorkedBtn) {
        reportWorkedBtn.onclick = () => reportGameStatus(true);
    }
    
    const reportFailedBtn = document.getElementById("reportGameFailed");
    if (reportFailedBtn) {
        reportFailedBtn.onclick = () => reportGameStatus(false);
    }
    
    const downloadProjectBtn = document.getElementById("downloadProject");
    if (downloadProjectBtn) {
        downloadProjectBtn.onclick = downloadScratchProject;
    }
    
    // Allow Enter key to trigger download
    const projectURLInput = document.getElementById("projectURLInput");
    if (projectURLInput) {
        projectURLInput.addEventListener("keypress", (e) => {
            if (e.key === "Enter") {
                downloadScratchProject();
            }
        });
    }
    
}

main();
