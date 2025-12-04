import { unzipSync } from "https://unpkg.com/fflate/esm/browser.js";
import * as compile from "./utils/compile.js";

// Global variables for ESP32 functionality
let serialPort = null;
let serialReader = null;
let currentProjectInfo = null;


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
async function sendProgramDataViaSerial(bytes) {
    try {
        if (!serialPort) {
            throw new Error("Serial port not connected. Please connect first.");
        }
        
        // Get the program data from getProgramAsBlob
        
        if (!bytes || bytes.length === 0) {
            throw new Error("No program data to send");
        }
        
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

async function reportGameStatus(bytes, worked) {
    try {
        updateStatus("Preparing report data...", "info");
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
