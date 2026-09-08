# -*- coding: utf-8 -*-

from flask import Flask, request, jsonify, send_file
import time
import os
import tempfile
from io import BytesIO
import threading

app = Flask(__name__)

# 配置
UPLOAD_FOLDER = os.path.join(tempfile.gettempdir(), "httpclient_test_uploads")
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

app.config["UPLOAD_FOLDER"] = UPLOAD_FOLDER

# 请求计数器，用于测试
request_counter = 0
counter_lock = threading.Lock()


@app.route("/")
def index():
    """根路径，用于简单测试服务器是否运行"""
    return jsonify(
        {
            "status": "ok",
            "message": "Flask test server is running",
            "timestamp": time.time(),
        }
    )


@app.route("/api/health")
def health_check():
    """健康检查端点"""
    return jsonify(
        {
            "status": "healthy",
            "timestamp": time.time(),
            "upload_folder_exists": os.path.exists(app.config["UPLOAD_FOLDER"]),
        }
    )


@app.route("/api/test", methods=["GET", "POST", "DELETE", "PUT", "PATCH"])
def api_test():
    """通用测试端点，支持多种HTTP方法"""
    global request_counter

    with counter_lock:
        request_counter += 1
        current_count = request_counter

    response_data = {
        "message": f"{request.method} request successful",
        "method": request.method,
        "request_id": current_count,
        "timestamp": time.time(),
        "path": request.path,
        "query_args": dict(request.args),
    }

    if request.method == "POST":
        # 对于POST请求，返回接收到的数据
        if request.content_type == "application/json":
            try:
                json_data = request.get_json()
                if json_data:
                    response_data["received_data"] = json_data
                    response_data["data_size"] = len(str(json_data))
            except Exception as e:
                response_data["error"] = f"JSON parsing error: {str(e)}"
        else:
            data = request.get_data()
            response_data["received_data_preview"] = data[:100].decode(
                "utf-8", errors="ignore"
            )
            response_data["data_size"] = len(data)

    elif request.method == "PUT":
        data = request.get_data()
        response_data["content_length"] = len(data)
        if data:
            response_data["data_preview"] = data[:100].decode("utf-8", errors="ignore")

    return jsonify(response_data)


@app.route("/api/timeout")
def api_timeout():
    """模拟超时的端点"""
    delay = request.args.get("delay", 3, type=float)
    time.sleep(delay)
    return jsonify(
        {
            "message": f"Response after {delay} seconds delay",
            "delayed": True,
            "timestamp": time.time(),
        }
    )


@app.route("/api/error")
def api_error():
    """模拟服务器错误"""
    error_type = request.args.get("type", "500")
    if error_type == "500":
        return jsonify({"error": "Internal Server Error"}), 500
    elif error_type == "404":
        return jsonify({"error": "Not Found"}), 404
    elif error_type == "400":
        return jsonify({"error": "Bad Request"}), 400
    else:
        return jsonify({"error": "Unknown Error"}), 500


@app.route("/api/text")
def text_response():
    """Plain text endpoint for testing non-JSON responses"""
    return (
        "This is plain text, not JSON. " * 3,
        200,
        {"Content-Type": "text/plain"},
    )


@app.route("/download")
def download():
    """File download endpoint, supports Range requests for resume testing"""
    file_content = b"This is test file content for download verification. " * 10
    return send_file(
        BytesIO(file_content),
        as_attachment=True,
        download_name="test_file.txt",
        mimetype="text/plain",
        conditional=True,
    )


@app.route("/download-no-range")
def download_no_range():
    """Ignores Range headers: always full content with 200.

    Exercises the client's restart-from-scratch branch (a stale .temp
    must be truncated and rewritten, not spliced). Uses a plain response:
    send_file() defaults to conditional=True in Werkzeug >= 2.1 and would
    honor the Range header."""
    file_content = b"This is test file content for download verification. " * 10
    response = app.response_class(file_content, mimetype="text/plain")
    response.headers.set("Content-Disposition", "attachment", filename="test_file.txt")
    return response


@app.route("/api/echo", methods=["POST", "PUT", "GET"])
def echo():
    """回显请求数据"""
    data = request.get_data()
    headers = dict(request.headers)

    response_data = {
        "echo_data": data.decode("utf-8", errors="ignore") if data else "",
        "content_length": len(data),
        "method": request.method,
        "timestamp": time.time(),
        "headers_count": len(headers),
    }

    # 添加一些特定的header信息
    custom_headers = {}
    for key, value in headers.items():
        if key.lower().startswith("x-"):
            custom_headers[key] = value
    response_data["custom_headers"] = custom_headers

    # 对于JSON请求，尝试解析（is_json 忽略 charset 等参数，
    # 精确比较 "application/json" 会漏掉 "application/json; charset=utf-8"）
    if request.is_json and data:
        try:
            json_data = request.get_json()
            response_data["parsed_json"] = json_data
        except:
            response_data["json_parse_error"] = True

    return jsonify(response_data)


@app.route("/api/headers")
def check_headers():
    """检查自定义header"""
    headers = dict(request.headers)
    custom_headers = {}

    # 提取自定义头
    for key, value in headers.items():
        if key.lower().startswith("x-") or key.lower() in [
            "authorization",
            "user-agent",
        ]:
            custom_headers[key] = value

    return jsonify(
        {
            "headers": custom_headers,
            "method": request.method,
            "all_headers_count": len(headers),
            "timestamp": time.time(),
        }
    )


@app.route("/api/concurrent")
def concurrent_test():
    """用于并发测试的端点"""
    time.sleep(0.1)  # 100ms延迟
    return jsonify(
        {
            "message": "Concurrent request handled",
            "timestamp": time.time(),
            "thread": threading.current_thread().name,
        }
    )


@app.route("/api/upload", methods=["PUT", "POST"])
def upload():
    """文件上传处理"""
    if request.method == "PUT":
        data = request.get_data()
        content_length = len(data)

        # 保存PUT上传的数据
        if content_length > 0:
            filename = f"put_upload_{int(time.time())}.bin"
            filepath = os.path.join(app.config["UPLOAD_FOLDER"], filename)
            with open(filepath, "wb") as f:
                f.write(data)

        return jsonify(
            {
                "message": "PUT upload successful",
                "content_length": content_length,
                "saved_as": filename if content_length > 0 else None,
                "timestamp": time.time(),
            }
        )

    elif request.method == "POST":
        # 处理文件上传或普通POST数据
        if "file" in request.files:
            file = request.files["file"]
            if file.filename:
                filename = f"post_upload_{int(time.time())}_{file.filename}"
                filepath = os.path.join(app.config["UPLOAD_FOLDER"], filename)
                file.save(filepath)
                file_size = os.path.getsize(filepath)

                return jsonify(
                    {
                        "message": "File upload successful",
                        "filename": file.filename,
                        "content_length": file_size,
                        "saved_path": filepath,
                        "timestamp": time.time(),
                    }
                )
            else:
                return jsonify({"error": "No file selected"}), 400
        else:
            data = request.get_data()
            content_length = len(data)
            return jsonify(
                {
                    "message": "POST data received",
                    "content_length": content_length,
                    "timestamp": time.time(),
                }
            )


@app.route("/api/cleanup", methods=["POST", "GET"])
def cleanup_uploads():
    """清理上传的文件"""
    try:
        deleted_count = 0
        if os.path.exists(app.config["UPLOAD_FOLDER"]):
            for filename in os.listdir(app.config["UPLOAD_FOLDER"]):
                filepath = os.path.join(app.config["UPLOAD_FOLDER"], filename)
                if os.path.isfile(filepath):
                    os.unlink(filepath)
                    deleted_count += 1
        return jsonify(
            {
                "message": f"Uploads cleaned up, deleted {deleted_count} files",
                "timestamp": time.time(),
            }
        )
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.errorhandler(404)
def not_found(error):
    return jsonify({"error": "Not found", "path": request.path}), 404


@app.errorhandler(500)
def internal_error(error):
    return jsonify({"error": "Internal server error"}), 500


@app.errorhandler(400)
def bad_request(error):
    return jsonify({"error": "Bad request"}), 400


def print_routes():
    """打印所有可用的路由"""
    print("\nAvailable endpoints:")
    rules = []
    for rule in app.url_map.iter_rules():
        methods = ",".join(sorted(rule.methods))
        rules.append((rule.rule, methods))

    rules.sort()
    for rule, methods in rules:
        print(f"  {methods:15} {rule}")


if __name__ == "__main__":
    print("Starting HttpClient Test Server on http://127.0.0.1:8000")
    print(f"Upload folder: {UPLOAD_FOLDER}")
    print_routes()
    print("\nPress Ctrl+C to stop the server")

    from werkzeug.serving import make_server

    # HTTP 实例交给后台线程，主线程另起 HTTPS（adhoc 自签证书）实例：
    # 覆盖 sslErrors 收集与 ignoreSslErrors 路径；缺 pyOpenSSL 时退化为仅 HTTP
    http_server = make_server("127.0.0.1", 8000, app, threaded=True)
    threading.Thread(target=http_server.serve_forever, daemon=True).start()

    try:
        # 设置为非调试模式，避免双重输出
        app.run(
            host="127.0.0.1", port=8443, ssl_context="adhoc", debug=False, threaded=True
        )
    except Exception as e:
        print(f"HTTPS instance unavailable ({e}); serving HTTP only")
        http_server.serve_forever()
