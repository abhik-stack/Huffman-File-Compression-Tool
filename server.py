
from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
from werkzeug.utils import secure_filename
import subprocess
import os
import uuid
import csv

app = Flask(__name__, static_folder="frontend", static_url_path="")
CORS(app)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
UPLOAD_DIR = os.path.join(BASE_DIR, "uploads")

os.makedirs(UPLOAD_DIR, exist_ok=True)

@app.route("/")
def home():
    return app.send_static_file("index.html")

@app.route("/test", methods=["GET"])
def test():
    return jsonify({
        "status": "success",
        "message": "Backend connected successfully!"
    })

@app.route("/api/compress", methods=["POST"])
def compress():

    if "file" not in request.files:
        return jsonify({"error": "No file uploaded"}), 400

    uploaded_file = request.files["file"]
    if not uploaded_file.filename:
        return jsonify({"error": "No filename provided"}), 400

    allowed_extensions = {".txt", ".csv", ".log"}

    filename = secure_filename(uploaded_file.filename)
    extension = os.path.splitext(filename)[1].lower()

    if extension not in allowed_extensions:
        return jsonify({
        "error": "Unsupported file type. Please upload .txt, .csv, or .log files."
    }), 400
    uploaded_file.seek(0, os.SEEK_END)
    file_size = uploaded_file.tell()
    uploaded_file.seek(0)

    if file_size == 0:
        return jsonify({"error": "The uploaded file is empty."}), 400

    if file_size > 10 * 1024 * 1024:
        return jsonify({"error": "File size must be under 10 MB."}), 400

    if not uploaded_file.filename:
        return jsonify({"error": "No filename provided"}), 400

    safe_name = secure_filename(uploaded_file.filename)

    unique_name = uuid.uuid4().hex + "_" + safe_name

    input_path = os.path.join(UPLOAD_DIR, unique_name)
    compressed_path = input_path + ".huff"

    uploaded_file.save(input_path)
    print("DECOMPRESS UPLOAD SIZE:", os.path.getsize(input_path))
    print("DECOMPRESS UPLOAD PATH:", input_path)

    try:
        result = subprocess.run(
            [os.path.join(BASE_DIR, "compressor")],
            input=f"1\n{input_path}\n",
            text=True,
            capture_output=True,
            cwd=BASE_DIR,
            timeout=60
        )

        if result.returncode != 0 or not os.path.exists(compressed_path):
            return jsonify({
                "error": "Compression failed",
                "details": result.stdout + result.stderr
            }), 500

        original_size = os.path.getsize(input_path)
        compressed_size = os.path.getsize(compressed_path)

        saved = original_size - compressed_size

        percentage = (
            (saved / original_size) * 100
            if original_size > 0 else 0
        )

        ratio = (
            original_size / compressed_size
            if compressed_size > 0 else 0
        )

        return jsonify({
            "status": "success",
            "message": "File compressed successfully!",
            "original_size": original_size,
            "compressed_size": compressed_size,
            "space_saved": saved,
            "saving_percentage": round(percentage, 2),
            "compression_ratio": round(ratio, 2),
            "download_url": f"/api/download/{os.path.basename(compressed_path)}"
        })

    except subprocess.TimeoutExpired:
        return jsonify({"error": "Compression timed out"}), 500

    except Exception as error:
        return jsonify({"error": str(error)}), 500

@app.route("/api/decompress", methods=["POST"])
def decompress():

    if "file" not in request.files:
        return jsonify({"error": "No file uploaded"}), 400

    uploaded_file = request.files["file"]

    if not uploaded_file.filename:
        return jsonify({"error": "No filename provided"}), 400

    safe_name = secure_filename(uploaded_file.filename)

    if not safe_name.lower().endswith(".huff"):
        return jsonify({"error": "Please upload a .huff file"}), 400

    unique_name = uuid.uuid4().hex + "_" + safe_name

    input_path = os.path.join(UPLOAD_DIR, unique_name)

    output_name = unique_name[:-5]
    output_path = os.path.join(UPLOAD_DIR, output_name)

    uploaded_file.save(input_path)

    try:
        result = subprocess.run(
            [os.path.join(BASE_DIR, "compressor")],
            input=f"2\n{input_path}\n{output_path}\n",
            text=True,
            capture_output=True,
            cwd=BASE_DIR,
            timeout=60
        )

        if result.returncode != 0 or not os.path.exists(output_path):
            return jsonify({
                "error": "Decompression failed",
                "details": result.stdout + result.stderr
            }), 500

        return jsonify({
            "status": "success",
            "message": "File decompressed successfully!",
            "download_url": f"/api/download/{output_name}"
        })

    except subprocess.TimeoutExpired:
        return jsonify({"error": "Decompression timed out"}), 500

    except Exception as error:
        return jsonify({"error": str(error)}), 500

@app.route("/api/download/<filename>")
def download(filename):
    return send_from_directory(
        UPLOAD_DIR,
        filename,
        as_attachment=True
    )
@app.route("/api/tree")
def get_huffman_tree():
    return send_from_directory(
        BASE_DIR,
        "huffman_tree.png"
    )
@app.route("/api/codes")
def get_huffman_codes():
    csv_path = os.path.join(BASE_DIR, "huffman_codes.csv")

    if not os.path.exists(csv_path):
        return jsonify({"error": "Huffman codes not found"}), 404

    with open(csv_path, "r", newline="", encoding="utf-8") as file:
        reader = csv.DictReader(file)
        codes = list(reader)

    return jsonify(codes)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", 5000)))
