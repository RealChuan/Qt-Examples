# -*- coding: utf-8 -*-

"""WebDavClient 集成测试用的 WebDAV 假服务器（客户端单测的"可控对端"）。

设计动机：客户端单测的断言面是"发出了什么"（动词/头/体）与"收到特定状态码
时行为是否正确"，因此不用 wsgidav 这类真服务器（错误码无法按需触发、请求
头无法回显），而是沿用 HttpClient 的 test_server.py 模式：echo 端点 +
XML fixture + tmp 目录文件语义 + LOCKS 字典模拟 423。

端点：
  GET  /health                  存活探测
  ANY  /echo                    回显 method/headers/body（自定义动词全支持）
  PROPFIND /dav/dir/            返回 207 multistatus fixture
  PROPFIND /malformed           返回 207 + 非 XML 内容（解析失败路径）
  ANY  /code/<int:code>         任意动词返回指定状态码（错误分类注入）
  /fs/<path:res>                真文件语义：PUT/GET/DELETE/MKCOL/COPY/MOVE
                               + LOCK/UNLOCK（LOCK_TOKEN 固定，PUT/DELETE
                               在锁定且 If 缺失/失配时返回 423）
  POST /__reset                 清空 LOCKS 与文件根目录（测试 cleanup 调用）

运行：python dav_server.py（127.0.0.1:8001）
"""

from flask import Flask, Response, jsonify, request
from urllib.parse import urlparse
import os
import shutil
import tempfile

app = Flask(__name__)

FS_ROOT = os.path.join(tempfile.gettempdir(), "webdav_test_fs")
os.makedirs(FS_ROOT, exist_ok=True)

# LOCK 响应回显的固定令牌（客户端自动提取入表的验证基准）
LOCK_TOKEN = "opaquelocktoken:urn:uuid:e71d4fa8-5d13-11e2-a65f-ff7ec9e8b68c"

# res -> LOCK_TOKEN；PUT/DELETE 在锁定时要求 If 头含令牌
LOCKS = {}

# 自定义动词全放行（Werkzeug 按请求行原样匹配 methods）；
# 显式列出 "OPTIONS" 以禁用 Flask 的自动 OPTIONS 处理
DAV_METHODS = [
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "PATCH",
    "OPTIONS",
    "PROPFIND",
    "PROPPATCH",
    "MKCOL",
    "COPY",
    "MOVE",
    "LOCK",
    "UNLOCK",
    "REPORT",
]

MULTISTATUS_XML = """<?xml version="1.0" encoding="utf-8"?>
<D:multistatus xmlns:D="DAV:">
  <D:response>
    <D:href>/dav/dir/</D:href>
    <D:propstat>
      <D:prop>
        <D:displayname>dir</D:displayname>
        <D:resourcetype><D:collection/></D:resourcetype>
        <D:getlastmodified>Tue, 13 Oct 2026 08:00:00 GMT</D:getlastmodified>
      </D:prop>
      <D:status>HTTP/1.1 200 OK</D:status>
    </D:propstat>
  </D:response>
  <D:response>
    <D:href>/dav/dir/hello.txt</D:href>
    <D:propstat>
      <D:prop>
        <D:displayname>hello.txt</D:displayname>
        <D:resourcetype/>
        <D:getcontentlength>13</D:getcontentlength>
        <D:getcontenttype>text/plain</D:getcontenttype>
        <D:getlastmodified>Tue, 13 Oct 2026 08:30:00 GMT</D:getlastmodified>
      </D:prop>
      <D:status>HTTP/1.1 200 OK</D:status>
    </D:propstat>
    <D:propstat>
      <D:prop><D:getetag/></D:prop>
      <D:status>HTTP/1.1 404 Not Found</D:status>
    </D:propstat>
  </D:response>
</D:multistatus>
"""

LOCKDISCOVERY_XML = f"""<?xml version="1.0" encoding="utf-8"?>
<D:prop xmlns:D="DAV:">
  <D:lockdiscovery>
    <D:activelock>
      <D:locktype><D:write/></D:locktype>
      <D:lockscope><D:exclusive/></D:lockscope>
      <D:depth>infinity</D:depth>
      <D:timeout>Second-600</D:timeout>
      <D:locktoken><D:href>{LOCK_TOKEN}</D:href></D:locktoken>
      <D:lockroot><D:href>/fs/</D:href></D:lockroot>
    </D:activelock>
  </D:lockdiscovery>
</D:prop>
"""


def fs_path(res):
    """URL 资源段 -> 本地路径，带越界（../）防护。"""
    p = os.path.realpath(os.path.join(FS_ROOT, res))
    root = os.path.realpath(FS_ROOT)
    if p != root and not p.startswith(root + os.sep):
        return None
    return p


def dest_res(destination):
    """从 Destination 头（绝对 URL）解析 /fs/ 下的资源段。"""
    if not destination:
        return None
    path = urlparse(destination).path
    prefix = "/fs/"
    if not path.startswith(prefix):
        return None
    return path[len(prefix) :]


def lock_allows(res):
    """res 未被锁，或请求 If 头携带锁令牌时返回 True。"""
    token = LOCKS.get(res)
    if token is None:
        return True
    return token in (request.headers.get("If") or "")


@app.route("/health")
def health():
    return jsonify({"status": "healthy"})


@app.route("/echo", methods=DAV_METHODS)
def echo():
    """回显端点：词汇映射断言面（动词/头/体原样返回）。"""
    body = request.get_data(as_text=True) or ""
    return jsonify(
        {
            "method": request.method,
            "depth": request.headers.get("Depth"),
            "destination": request.headers.get("Destination"),
            "overwrite": request.headers.get("Overwrite"),
            "if": request.headers.get("If"),
            "lock_token": request.headers.get("Lock-Token"),
            "timeout": request.headers.get("Timeout"),
            "content_type": request.content_type,
            "size": len(body.encode("utf-8")),
            "body": body,
            "has_allprop": "allprop" in body,
            "has_lockinfo": "lockinfo" in body,
        }
    )


@app.route("/dav/dir/", methods=["PROPFIND"])
def propfind_dir():
    return Response(MULTISTATUS_XML, status=207, mimetype="application/xml")


@app.route("/malformed", methods=["PROPFIND"])
def propfind_malformed():
    # 207 状态 + 非 XML 内容：触发 check() 的解析失败分支
    return Response("this is not xml at all", status=207, mimetype="text/plain")


@app.route("/code/<int:code>", methods=DAV_METHODS)
def inject_code(code):
    """任意动词返回指定状态码（409/412/423/502 等错误分类注入）。"""
    return jsonify({"code": code, "method": request.method}), code


@app.route(
    "/fs/<path:res>",
    methods=["GET", "PUT", "DELETE", "MKCOL", "COPY", "MOVE", "LOCK", "UNLOCK"],
)
def fs(res):
    p = fs_path(res)
    if p is None:
        return jsonify({"error": "path traversal rejected"}), 400

    if request.method == "LOCK":
        LOCKS[res] = LOCK_TOKEN
        return Response(LOCKDISCOVERY_XML, status=200, mimetype="application/xml")

    if request.method == "UNLOCK":
        if res not in LOCKS:
            return jsonify({"error": "not locked"}), 409
        header = request.headers.get("Lock-Token")
        if header != f"<{LOCK_TOKEN}>":
            return jsonify({"error": "Lock-Token header missing or mismatched"}), 400
        del LOCKS[res]
        return Response(status=204)

    if request.method == "PUT":
        if not lock_allows(res):
            return jsonify({"error": "resource is locked"}), 423
        existed = os.path.exists(p)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        with open(p, "wb") as f:
            f.write(request.get_data())
        return Response(status=204 if existed else 201)

    if request.method == "GET":
        if not os.path.isfile(p):
            return jsonify({"error": "not found"}), 404
        with open(p, "rb") as f:
            return Response(f.read(), status=200, mimetype="application/octet-stream")

    if request.method == "DELETE":
        if not os.path.exists(p):
            return jsonify({"error": "not found"}), 404
        if not lock_allows(res):
            return jsonify({"error": "resource is locked"}), 423
        os.remove(p)
        return Response(status=204)

    if request.method == "MKCOL":
        if os.path.isdir(p):
            return jsonify({"error": "collection already exists"}), 405
        if not os.path.isdir(os.path.dirname(p)):
            # RFC 4918：父集合不存在 -> 409 Conflict
            return jsonify({"error": "parent collection does not exist"}), 409
        os.makedirs(p)
        return Response(status=201)

    if request.method in ("COPY", "MOVE"):
        dres = dest_res(request.headers.get("Destination"))
        if dres is None:
            return (
                jsonify({"error": "Destination header must be an absolute /fs/ URL"}),
                400,
            )
        d = fs_path(dres)
        if d is None:
            return jsonify({"error": "destination traversal rejected"}), 400
        if not os.path.exists(p):
            return jsonify({"error": "source not found"}), 404
        if not lock_allows(res):
            return jsonify({"error": "source is locked"}), 423
        dest_existed = os.path.exists(d)
        if dest_existed and request.headers.get("Overwrite", "T").upper() == "F":
            # RFC 4918：Overwrite=F 撞已有目标 -> 412 Precondition Failed
            return jsonify({"error": "destination exists and Overwrite is F"}), 412
        os.makedirs(os.path.dirname(d), exist_ok=True)
        if request.method == "COPY":
            shutil.copy2(p, d)
        else:
            shutil.move(p, d)
        return Response(status=204 if dest_existed else 201)

    return jsonify({"error": "method not handled"}), 405


@app.route("/__reset", methods=["POST"])
def reset():
    LOCKS.clear()
    for name in os.listdir(FS_ROOT):
        path = os.path.join(FS_ROOT, name)
        if os.path.isdir(path):
            shutil.rmtree(path)
        else:
            os.remove(path)
    return jsonify({"status": "ok"})


def print_routes():
    print("\nAvailable endpoints:")
    rules = []
    for rule in app.url_map.iter_rules():
        methods = ",".join(sorted(rule.methods))
        rules.append((rule.rule, methods))
    rules.sort()
    for rule, methods in rules:
        print(f"  {methods:15} {rule}")


if __name__ == "__main__":
    print("Starting WebDavClient Test Server on http://127.0.0.1:8001")
    print(f"Filesystem root: {FS_ROOT}")
    print_routes()
    print("\nPress Ctrl+C to stop the server")

    from werkzeug.serving import make_server

    server = make_server("127.0.0.1", 8001, app, threaded=True)
    server.serve_forever()
