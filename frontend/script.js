
const API_URL = "http://127.0.0.1:5000";

const fileInput = document.getElementById("fileInput");
const compressBtn = document.getElementById("compressBtn");
const decompressBtn = document.getElementById("decompressBtn");

const originalSize = document.getElementById("originalSize");
const compressedSize = document.getElementById("compressedSize");
const spaceSaved = document.getElementById("spaceSaved");
const ratio = document.getElementById("ratio");

const treeContainer = document.getElementById("treeContainer");
const codeContainer = document.getElementById("codeContainer");

function formatSize(bytes) {
    if (bytes < 1024) return bytes + " Bytes";
    if (bytes < 1024 * 1024)
        return (bytes / 1024).toFixed(2) + " KB";
    return (bytes / (1024 * 1024)).toFixed(2) + " MB";
}

fileInput.addEventListener("change", () => {
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

    compressBtn.disabled = true;
    compressBtn.textContent = "Compressing...";

    const formData = new FormData();
    formData.append("file", file);

    try {
        const response = await fetch(`${API_URL}/api/compress`, {
            method: "POST",
            body: formData
        });

        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || "Compression failed");
        }

        originalSize.textContent = formatSize(data.original_size);
        compressedSize.textContent = formatSize(data.compressed_size);
        spaceSaved.textContent =
            formatSize(Math.abs(data.space_saved)) +
            (data.space_saved < 0 ? " extra" : " saved");

        ratio.textContent = data.compression_ratio + ":1";

        treeContainer.textContent = "Compression successful!";
        codeContainer.textContent =
            `Space saving: ${data.saving_percentage}%`;

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

        alert("File compressed successfully!");

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

    const formData = new FormData();
    formData.append("file", file);

    try {
        const response = await fetch(`${API_URL}/api/decompress`, {
            method: "POST",
            body: formData
        });

        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || "Decompression failed");
        }

        treeContainer.textContent = "Decompression successful!";
        codeContainer.textContent = data.message;

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

        alert("File decompressed successfully!");

    } catch (error) {
        alert("Error: " + error.message);
    } finally {
        decompressBtn.disabled = false;
        decompressBtn.textContent = "Decompress File";
    }
});

