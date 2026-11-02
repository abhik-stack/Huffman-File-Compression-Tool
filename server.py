
from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
from werkzeug.utils import secure_filename
import subprocess
import os
import uuid

app = Flask(__name__)
CORS(app)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
UPLOAD_DIR = os.path.join(BASE_DIR, "uploads")

os.makedirs(UPLOAD_DIR, exist_ok=True)

@app.route("/")
def home():
    return "Huffman Compression Backend is Running!"

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

    safe_name = secure_filename(uploaded_file.filename)

    unique_name = uuid.uuid4().hex + "_" + safe_name

    input_path = os.path.join(UPLOAD_DIR, unique_name)
    compressed_path = input_path + ".huff"

    uploaded_file.save(input_path)

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

if __name__ == "__main__":
    app.run(debug=True, port=5000)
