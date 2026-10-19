#!/usr/bin/env python3
"""WsgiDAV 真实服务器（RFC 4918 符合性测试夹具）。

用法：
    pip install "wsgidav>=4"
    python wsgidav_server.py     # 监听 127.0.0.1:8002，匿名访问

与 dav_server.py（Flask 故障注入）互补：本服务器是标准实现——真实 207
XML 结构、真实锁管理器（Class 2）、真实状态码语义；异常路径（部分失败
207 / 畸形 XML / 状态码注入）由 dav_server.py 覆盖。每次启动创建全新
临时共享目录，测试数据不落残留。

客户端套件：webdavclient_conformance_unittest.cc（服务器不可达时整批跳过）。
若日志显示 401，检查 simple_dc.user_mapping（匿名放行未生效）。
"""

import tempfile

try:
    from wsgidav.fs_dav_provider import FilesystemProvider
    from wsgidav.wsgidav_app import WsgiDAVApp
except ImportError as e:
    raise SystemExit(f"缺少依赖：{e}\n安装：pip install 'wsgidav>=4'")

HOST = "127.0.0.1"
PORT = 8002


def serve(app):
    """运行 WSGI 服务器（阻塞）：cheroot（wsgidav 默认携带）优先，Werkzeug 兜底。

    不能用 wsgiref.simple_server：其 wsgi.input 不按 Content-Length 截断，
    wsgidav 按块读请求体会死锁——服务端等满一块、客户端等响应，连接无
    EOF，小体积 PUT 必现挂死（实测 11 字节 PUT 双方互等直至超时）。
    """
    try:
        from cheroot import wsgi

        wsgi.Server((HOST, PORT), app).start()
        return
    except ImportError:
        pass
    from werkzeug.serving import run_simple

    run_simple(HOST, PORT, app)  # LimitedStream 按 Content-Length 截断，无此问题


def main():
    # 全新临时共享目录：服务器重启即干净，测试无需逐用例清理
    share = tempfile.mkdtemp(prefix="wsgidav_conformance_")
    config = {
        "host": HOST,
        "port": PORT,
        "provider_mapping": {"/": FilesystemProvider(share)},
        # 匿名访问：user_mapping 为 True 的共享放行匿名（不发起 401 挑战）。
        # 注意 domain_controller: None 在 4.3.x 只是回退到 SimpleDomainController，
        # 并不禁用认证
        "simple_dc": {"user_mapping": {"*": True}},
        "lock_storage": True,  # Class 2：LOCK/UNLOCK/If 语义
        # property_manager 已废弃：死属性存储默认启用，无需配置
    }
    app = WsgiDAVApp(config)
    print(f"WsgiDAV conformance server on http://{HOST}:{PORT} (share: {share})")
    serve(app)


if __name__ == "__main__":
    main()
