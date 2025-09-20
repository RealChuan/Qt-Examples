# -*- coding: utf-8 -*-

import argparse
import base64
import hashlib
import json
import logging
import os
import shutil
import sys
import time
from pathlib import Path
from typing import Iterable, Optional, Callable

import paramiko
import requests
from requests.adapters import HTTPAdapter
from urllib3.util.retry import Retry

# ---------------- 日志 ----------------
logging.basicConfig(
    level=logging.INFO,
    format="[%(asctime)s] %(levelname)s | %(message)s",
    datefmt="%H:%M:%S",
)
log = logging.getLogger("uploader")

MB = 1024 * 1024
DEFAULT_CHUNK = 8 * MB


# ---------------- 工具 ----------------
def remove(path: str) -> None:
    """安全删除文件或目录（不存在时静默）"""
    if os.path.isfile(path):
        os.remove(path)
    elif os.path.isdir(path):
        shutil.rmtree(path)


def _format_bytes(n: int, precision: int = 2) -> str:
    value = float(n)  # 用浮点副本做除法
    for unit in ["B", "KB", "MB", "GB", "TB"]:
        if value < 1024:
            return f"{value:.{precision}f} {unit}"
        value /= 1024
    return f"{value:.2f} PB"


def calc_md5(path: str, chunk_size: int = 8 * 1024 * 1024) -> str:
    h = hashlib.md5()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(chunk_size), b""):
            h.update(chunk)
    return h.hexdigest()


def write_json_describe(
    version: str, md5_file_path: str, file_name: str, json_path: str
) -> None:
    md5 = calc_md5(md5_file_path)
    build = int(time.strftime("%Y%m%d"))
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(
            {"version": version, "buildVersion": build, "name": file_name, "md5": md5},
            f,
            indent=4,
            ensure_ascii=False,
        )
    log.info(f"生成描述文件 {json_path}")


def _progress(total: int) -> Callable[[int], None]:
    """返回回调，内部自动累计，上传新文件时重新调用即可清零"""
    transferred = 0

    def callback(chunk_size: int) -> None:
        nonlocal transferred
        transferred += chunk_size
        percent = transferred / total * 100
        bar_len = 30
        bar = "#" * int(percent / 100 * bar_len)
        sys.stdout.write(
            f"\r[{bar:<{bar_len}}] {percent:6.2f}%  "
            f"{_format_bytes(transferred)}/{_format_bytes(total)}"
        )
        if transferred >= total:
            sys.stdout.write("\n")
        sys.stdout.flush()

    return callback


# ---------------- 分块生成器 ----------------
def _chunked_reader(
    path: Path, chunk_size: int, on_progress: Optional[Callable[[int], None]] = None
) -> Iterable[bytes]:
    """供 HTTP 支使用，内部已自动累加"""
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(chunk_size), b""):
            yield chunk
            if on_progress:
                on_progress(len(chunk))


# ---------------- 上传函数 ----------------
def upload_http_put(
    host: str,
    port: int,
    username: str,
    password: str,
    remote_path: str,
    local_file: str,
    chunk_size: int = DEFAULT_CHUNK,
    timeout: int = 30,
) -> bool:
    filepath = Path(local_file)
    url = f"http://{host}:{port}{remote_path}"
    auth = base64.b64encode(f"{username}:{password}".encode()).decode()
    headers = {
        "User-Agent": "Uploader/1.0",
        "Content-Type": "application/octet-stream",
        "Overwrite": "T",
        "translate": "f",
        "Authorization": f"Basic {auth}",
    }

    session = requests.Session()
    retry = Retry(total=3, backoff_factor=1, status_forcelist=[502, 503, 504])
    session.mount("http://", HTTPAdapter(max_retries=retry))
    session.mount("https://", HTTPAdapter(max_retries=retry))

    try:
        total = filepath.stat().st_size
        callback = _progress(total)
        resp = session.put(
            url,
            data=_chunked_reader(filepath, chunk_size, callback),
            headers=headers,
            timeout=timeout,
            verify=False,
        )
        if resp.status_code == 201:
            log.info(f"[HTTP PUT] 上传完成 -> {url}")
            return True
        log.error(f"上传失败 {resp.status_code} {resp.reason}")
        return False
    except Exception as e:
        log.error(f"上传异常 {e}")
        return False


def upload_sftp(
    host: str,
    port: int,
    username: str,
    password: str,
    remote_path: str,
    local_file: str,
    chunk_size: int = DEFAULT_CHUNK,
    timeout: int = 30,
) -> bool:
    filepath = Path(local_file)
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh.connect(
            hostname=host,
            port=port,
            username=username,
            password=password,
            timeout=timeout,
        )
        sftp = ssh.open_sftp()

        total = filepath.stat().st_size
        callback = _progress(total)

        with filepath.open("rb") as fl, sftp.file(remote_path, "wb") as fr:
            for chunk in iter(lambda: fl.read(chunk_size), b""):
                fr.write(chunk)
                callback(len(chunk))  # 仅传增量，内部自动累加

        log.info(f"[SFTP] 上传完成 -> {remote_path}")
        return True
    except Exception as e:
        log.error(f"上传异常 {e}")
        return False
    finally:
        ssh.close()


# ---------------- CLI ----------------
def parse_args():
    p = argparse.ArgumentParser(description="函数式分块上传（带进度，自动清零累计）")
    p.add_argument("-f", "--file", required=True, type=Path, help="本地文件")
    p.add_argument("-v", "--version", required=True, help="版本号")
    p.add_argument(
        "-t", "--type", choices=["http_put", "sftp"], required=True, help="协议"
    )
    p.add_argument("-H", "--host", required=True, help="服务器地址")
    p.add_argument("-P", "--port", type=int, default=22, help="端口")
    p.add_argument("-u", "--username", required=True, help="用户名")
    p.add_argument("-p", "--password", required=True, help="密码")
    p.add_argument("-r", "--remote-dir", required=True, help="远程目录")
    p.add_argument("-j", "--json-output", type=Path, help="描述文件输出路径")
    p.add_argument(
        "-c", "--chunk-size", type=int, default=DEFAULT_CHUNK, help="块大小(byte)"
    )
    return p.parse_args()


def main(argv=None):
    args = parse_args()
    if not args.file.exists():
        log.error(f"文件不存在 {args.file}")
        return False

    # 1. 生成 json
    suffix = os.path.splitext(str(args.file))[1] + ".json"
    json_path = args.json_output or args.file.with_suffix(suffix)
    write_json_describe(args.version, str(args.file), args.file.name, str(json_path))

    # 2. 上传
    ok = False
    if args.type == "http_put":
        ok = upload_http_put(
            args.host,
            args.port,
            args.username,
            args.password,
            args.remote_dir,
            str(args.file),
            args.chunk_size,
        )
    elif args.type == "sftp":
        ok = upload_sftp(
            args.host,
            args.port,
            args.username,
            args.password,
            args.remote_dir,
            str(args.file),
            args.chunk_size,
        )

    if ok:
        log.info("全部完成 ✔")
    else:
        log.error("上传失败 ✘")
    return ok


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 2)
