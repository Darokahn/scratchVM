from flask import Flask, request, send_from_directory
import os

app = Flask(__name__)
UPLOAD_DIR = "upload"

# Ensure the upload directory exists
os.makedirs(UPLOAD_DIR, exist_ok=True)

# Serve any file requested
@app.route('/<path:filename>', methods=['GET'])
def serve_file(filename):
    return send_from_directory('.', filename)

# Handle file uploads
@app.route('/upload/<path:filename>', methods=['POST'])
def upload_file(filename):
    filepath = os.path.join(UPLOAD_DIR, filename)
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with open(filepath, 'wb') as f:
        f.write(request.data)
    return f"Saved {filename}", 200

if __name__ == "__main__":
    app.run(debug=True)
