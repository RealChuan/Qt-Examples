# -*- coding: utf-8 -*-

import time
import hashlib
import json
import base64
import requests
import os
import shutil


def remove(path):
    if os.path.exists(path):
        if os.path.isfile(path):
            os.remove(path)
        else:
            shutil.rmtree(path)


def generate_json(version, md5_file_path, file_name, json_path):
    with open(md5_file_path, "rb") as fp:
        md5 = hashlib.md5(fp.read()).hexdigest()
    current_time = time.strftime("%Y%m%d", time.localtime())
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(
            {
                "version": version,
                "buildVersion": int(current_time),
                "name": file_name,
                "md5": md5,
            },
            f,
            indent=4,
        )


def upload_file(url, username, password, local_file):
    headers = {
        "User-Agent": "Mozilla/5.0",
        "Content-Type": "application/octet-stream",
        "Connection": "close",
        "Overwrite": "T",
        "translate": "f",
        "Authorization": "Basic "
        + base64.b64encode(f"{username}:{password}".encode()).decode(),
    }
    with open(local_file, "rb") as f:
        headers["Content-Length"] = str(os.path.getsize(local_file))
        with requests.Session() as session:
            response = session.put(url, headers=headers, data=f, verify=False)
    if response.status_code == 201:
        print(f"[+] Upload file '{local_file}' success")
    else:
        print(
            f"[-] Upload file '{local_file}' failed, status code: {response.status_code}, reason: {response.reason}"
        )
