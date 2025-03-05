# -*- coding: utf-8 -*-

import time
import hashlib
import json
import base64
import requests
import os
import shutil
import paramiko


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


def format_bytes(size, precision=2):
    units = ["B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"]
    unit_index = 0
    while size >= 1024 and unit_index < len(units) - 1:
        size /= 1024.0
        unit_index += 1
    return f"{size:.{precision}f} {units[unit_index]}"


def sftp_upload_file(
    sftp_server,
    sftp_port,
    username,
    password,
    local_file_path,
    remote_file_path,
    chunk_size=10 * 1024 * 1024,
):
    """
    使用Paramiko通过SFTP上传大文件
    :param sftp_server: SFTP服务器地址
    :param sftp_port: SFTP服务器端口
    :param username: 用户名
    :param password: 密码
    :param local_file_path: 本地文件路径
    :param remote_file_path: 远程文件路径
    :param chunk_size: 每次读取的块大小，默认为10MB
    """

    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

    try:
        ssh.connect(
            hostname=sftp_server, port=sftp_port, username=username, password=password
        )
        sftp = ssh.open_sftp()

        if not os.path.isfile(local_file_path):
            raise FileNotFoundError(f"本地文件 {local_file_path} 不存在")

        with open(local_file_path, "rb") as local_file:
            with sftp.open(remote_file_path, "wb") as remote_file:
                total_size = os.path.getsize(local_file_path)
                uploaded_size = 0

                while True:
                    chunk = local_file.read(chunk_size)
                    if not chunk:
                        break
                    remote_file.write(chunk)
                    uploaded_size += len(chunk)

                    print(
                        f"上传进度：{format_bytes(uploaded_size)}/{format_bytes(total_size)} ({uploaded_size / total_size * 100:.2f}%)"
                    )

        print(f"文件 {local_file_path} 已成功上传到 {remote_file_path}")

    except FileNotFoundError as e:
        print(f"错误：{e}")
    except paramiko.AuthenticationException:
        print("错误：认证失败，请检查用户名和密码是否正确")
    except paramiko.SSHException as e:
        print(f"错误：SSH连接失败，原因：{e}")
    except Exception as e:
        print(f"未知错误：{e}")
    finally:
        sftp.close()
        ssh.close()
