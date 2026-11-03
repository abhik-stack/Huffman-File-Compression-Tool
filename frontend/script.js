
const API_URL = "";

const fileInput = document.getElementById("fileInput");
const compressBtn = document.getElementById("compressBtn");
const decompressBtn = document.getElementById("decompressBtn");

const originalSize = document.getElementById("originalSize");
const compressedSize = document.getElementById("compressedSize");
const spaceSaved = document.getElementById("spaceSaved");
const ratio = document.getElementById("ratio");

const treeContainer = document.getElementById("treeContainer");
const codeContainer = document.getElementById("codeContainer");
const progressSection = document.getElementById("progressSection");
const progressText = document.getElementById("progressText");
const progressFill = document.getElementById("progressFill");

function showProgress(message) {
    progressSection.style.display = "block";
    progressText.textContent = message;
    progressFill.style.width = "35%";
}

function updateProgress(message) {
    progressText.textContent = message;
    progressFill.style.width = "75%";
}

function completeProgress(message) {
    progressText.textContent = message;
    progressFill.style.width = "100%";
}
const statusMessage = document.createElement("p");
statusMessage.id = "statusMessage";
statusMessage.style.marginTop = "15px";
statusMessage.style.fontWeight = "bold";
statusMessage.style.color = "#a5b4fc";

compressBtn.insertAdjacentElement("afterend", statusMessage);

function formatSize(bytes) {
    if (bytes < 1024) return bytes + " Bytes";
    if (bytes < 1024 * 1024)
        return (bytes / 1024).toFixed(2) + " KB";
    return (bytes / (1024 * 1024)).toFixed(2) + " MB";
}
function displayHuffmanTree() {
    const img = document.createElement("img");

    img.src = `${API_URL}/api/tree?t=${Date.now()}`;
    img.alt = "Huffman Tree Visualization";

    img.style.width = "100%";
    img.style.height = "auto";
    img.style.maxHeight = "500px";
    img.style.objectFit = "contain";
    img.style.borderRadius = "10px";

    treeContainer.innerHTML = "";
    treeContainer.appendChild(img);
}
async function displayHuffmanCodes() {
    try {
        const response = await fetch(`${API_URL}/api/codes`);

        if (!response.ok) {
            throw new Error("Could not load Huffman codes");
        }

        const codes = await response.json();

        if (!codes.length) {
            codeContainer.textContent = "No Huffman codes available.";
            return;
        }

        const table = document.createElement("table");
        table.style.width = "100%";
        table.style.borderCollapse = "collapse";
        table.style.textAlign = "left";

        const headers = Object.keys(codes[0]);
        const thead = document.createElement("thead");
        const headerRow = document.createElement("tr");

        headers.forEach(header => {
            const th = document.createElement("th");
            th.textContent = header;
            th.style.padding = "12px";
            th.style.color = "#a5b4fc";
            headerRow.appendChild(th);
        });

        thead.appendChild(headerRow);
        table.appendChild(thead);

        const tbody = document.createElement("tbody");

        codes.forEach(item => {
            const row = document.createElement("tr");

            headers.forEach(header => {
                const td = document.createElement("td");
                td.textContent = item[header];
                td.style.padding = "10px 12px";
                td.style.borderTop = "1px solid #2a344d";
                row.appendChild(td);
            });

            tbody.appendChild(row);
        });

        table.appendChild(tbody);

        codeContainer.innerHTML = "";
        codeContainer.appendChild(table);

    } catch (error) {
        codeContainer.textContent = error.message;
    }
}

fileInput.addEventListener("change", () => {
    progressSection.style.display = "none";
    progressFill.style.width = "0%";

    originalSize.textContent = "--";
    compressedSize.textContent = "--";
    spaceSaved.textContent = "--";
    ratio.textContent = "--";

    treeContainer.textContent = "Ready for compression";
    codeContainer.textContent = "Waiting for Huffman encoding";
    const file = fileInput.files[0];

    if (file) {
        originalSize.textContent = formatSize(file.size);
        compressedSize.textContent = "--";
        spaceSaved.textContent = "--";
        ratio.textContent = "--";
        treeContainer.textContent = "Ready for compression";
        codeContainer.textContent = "Waiting for Huffman encoding";
    }
});

compressBtn.addEventListener("click", async () => {
    const file = fileInput.files[0];

    if (!file) {
        alert("Please select a file first!");
        return;
    }
    const allowedExtensions = [".txt", ".csv", ".log"];

if (!allowedExtensions.some(ext =>
    file.name.toLowerCase().endsWith(ext)
)) {
    alert("Please select a supported file (.txt, .csv, .log).");
    return;
}

if (file.size === 0) {
    alert("The selected file is empty.");
    return;
}

if (file.size > 10 * 1024 * 1024) {
    alert("File size must be under 10 MB.");
    return;
}

    compressBtn.disabled = true;
    compressBtn.textContent = "Compressing...";
    decompressBtn.disabled = true;
    compressBtn.disabled = true;
    statusMessage.textContent = "⏳ Compressing your file...";
    showProgress("Compressing your file...");

    const formData = new FormData();
    formData.append("file", file);

    try {
        const response = await fetch(`${API_URL}/api/compress`, {
            method: "POST",
            body: formData
        });

        const data = await response.json();
        updateProgress("Generating Huffman tree and codes...");
        statusMessage.textContent = "✓ Compression completed successfully!";
        completeProgress("Compression completed successfully!");

        if (!response.ok) {
            throw new Error(data.error || "Compression failed");
        }

        originalSize.textContent = formatSize(data.original_size);
        compressedSize.textContent = formatSize(data.compressed_size);
        spaceSaved.textContent =
            formatSize(Math.abs(data.space_saved)) +
            (data.space_saved < 0 ? " extra" : " saved");

        ratio.textContent = data.compression_ratio + ":1";

        displayHuffmanTree();
        await displayHuffmanCodes();

        let downloadLink = document.getElementById("downloadLink");

        if (!downloadLink) {
            downloadLink = document.createElement("a");
            downloadLink.id = "downloadLink";
            downloadLink.textContent = "Download Compressed File";
            downloadLink.style.display = "inline-block";
            downloadLink.style.marginTop = "20px";
            downloadLink.style.color = "#8b9cff";
            downloadLink.style.fontWeight = "bold";

            compressBtn.insertAdjacentElement("afterend", downloadLink);
        }

        downloadLink.href = API_URL + data.download_url;
        downloadLink.download = "";
        downloadLink.target = "_blank";

        statusMessage.textContent = "✓ Compression completed successfully!";

    } catch (error) {
        alert("Error: " + error.message);
    } finally {
        compressBtn.disabled = false;
        compressBtn.textContent = "Compress File";
    }
});


decompressBtn.addEventListener("click", async () => {
    const file = fileInput.files[0];

    if (!file) {
        alert("Please select a .huff file first!");
        return;
    }

    if (!file.name.toLowerCase().endsWith(".huff")) {
        alert("Please select a valid .huff file!");
        return;
    }

    decompressBtn.disabled = true;
    decompressBtn.textContent = "Decompressing...";
    showProgress("Decompressing your file...");

    const formData = new FormData();
    formData.append("file", file);

    try {
        const response = await fetch(`${API_URL}/api/decompress`, {
            method: "POST",
            body: formData
        });

        const data = await response.json();
        updateProgress("Preparing the restored file...");

        if (!response.ok) {
            throw new Error(data.error || "Decompression failed");
        }

        treeContainer.textContent = "Decompression successful!";
        codeContainer.textContent = data.message;

        originalSize.textContent = "--";
        compressedSize.textContent = "--";
        spaceSaved.textContent = "--";
        ratio.textContent = "--";

        let downloadLink = document.getElementById("downloadLink");

        if (!downloadLink) {
            downloadLink = document.createElement("a");
            downloadLink.id = "downloadLink";
            downloadLink.style.display = "inline-block";
            downloadLink.style.marginTop = "20px";
            downloadLink.style.color = "#8b9cff";
            downloadLink.style.fontWeight = "bold";

            decompressBtn.insertAdjacentElement("afterend", downloadLink);
        }

        downloadLink.textContent = "Download Restored File";
        downloadLink.href = API_URL + data.download_url;
        downloadLink.download = "";
        downloadLink.target = "_blank";
        completeProgress("Decompression completed successfully!");

    } catch (error) {
        alert("Error: " + error.message);
        progressText.textContent = "Decompression failed.";
        progressFill.style.width = "100%";
    } finally {
    compressBtn.disabled = false;
    decompressBtn.disabled = false;

    compressBtn.textContent = "Compress File";
    decompressBtn.textContent = "Decompress File";
}
});

