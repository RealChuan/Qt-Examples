# -*- coding: utf-8 -*-

import time
import hashlib
import json
import base64
import requests


def generate_json(version, md5_file_path, file_name, json_path):
    with open(md5_file_path, "rb") as fp:
        data = fp.read()
    md5 = hashlib.md5(data).hexdigest()
    current_time = time.strftime("%Y%m%d", time.localtime())
    dict = {
        "version": version,
        "buildVersion": int(current_time),
        "name": file_name,
        "md5": md5,
    }
    with open(json_path, "w", encoding="utf-8") as f:
        f.write(json.dumps(dict, indent=4))


def upload_file(url, username, password, local_file):
    headers = {"User-Agent": "Mozilla/5.0"}
    # 读取本地文件
    with open(local_file, "rb") as f:
        content = f.read()
    # 构造请求
    headers["Content-Type"] = "application/octet-stream"
    headers["Connection"] = "close"
    headers["Content-Length"] = str(len(content))
    headers["Overwrite"] = "T"
    headers["translate"] = "f"
    headers["Authorization"] = (
        "Basic " + base64.b64encode((username + ":" + password).encode()).decode()
    )
    # 发送请求
    print("Start upload file: {0}, URL: {1}".format(local_file, url))
    r = requests.put(url, headers=headers, data=content, verify=False)
    if r.status_code == 201:
        print("[+] upload file success")
    else:
        print(
            "[-] upload file failed, status code: {0}, reason: {1}".format(
                r.status_code, r.reason
            )
        )
