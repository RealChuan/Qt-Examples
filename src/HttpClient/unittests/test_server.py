# -*- coding: utf-8 -*-

from flask import Flask, request, jsonify, send_file
import time
from io import BytesIO

app = Flask(__name__)


@app.route("/api/test", methods=["GET", "POST", "DELETE"])
def api_test():
    if request.method == "GET":
        return jsonify(
            {"message": "GET request successful", "path": request.path, "method": "GET"}
        )

    elif request.method == "POST":
        data = request.get_json()
        return jsonify(
            {
                "message": "POST request successful",
                "received_data": data,
                "method": "POST",
            }
        )

    elif request.method == "DELETE":
        return jsonify({"message": "DELETE request successful", "method": "DELETE"})


@app.route("/api/timeout")
def api_timeout():
    # 模拟超时，睡眠3秒
    time.sleep(3)
    return jsonify({"message": "This should timeout"})


@app.route("/api/error")
def api_error():
    # 模拟服务器错误
    return jsonify({"error": "Internal Server Error"}), 500


@app.route("/download")
def download():
    # 文件下载
    file_content = b"This is test file content for download"
    return send_file(
        BytesIO(file_content), as_attachment=True, download_name="test_file.txt"
    )


@app.route("/api/upload", methods=["PUT", "POST"])
def upload():
    if request.method == "PUT":
        content_length = len(request.get_data())
        return jsonify(
            {"message": "PUT upload successful", "content_length": content_length}
        )

    elif request.method == "POST":
        # 检查是表单上传还是普通POST
        if "file" in request.files:
            file = request.files["file"]
            return jsonify(
                {
                    "message": "File upload successful",
                    "filename": file.filename,
                    "content_length": len(file.read()),
                }
            )
        else:
            content_length = len(request.get_data())
            return jsonify(
                {"message": "POST upload successful", "content_length": content_length}
            )


@app.route("/api/headers")
def check_headers():
    # 测试自定义header
    headers = dict(request.headers)
    return jsonify({"headers": headers, "method": request.method})


@app.errorhandler(404)
def not_found(error):
    return jsonify({"error": "Not found"}), 404


if __name__ == "__main__":
    # 设置为调试模式，这样在代码修改时会自动重启
    app.run(host="127.0.0.1", port=8000, debug=False)  # debug=False避免双重输出
