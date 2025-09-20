# -*- coding: utf-8 -*-
"""
通用 SFTP 上传脚本
支持平台：windows_x86、macos_x86_64、macos_aarch64、ubuntu_x86_64、ubuntu_aarch64
用法：
    python upload.py windows_x86
    python upload.py macos_x86_64
    ...
"""

import os
import time
import argparse

import utils

# -------------------- 配置区 --------------------
APP_NAME = "Qt-App"
VERSION = "0.0.1"
BUILD_DATE = time.strftime("%Y%m%d", time.localtime())

# 本地 releases 目录
RELEASES_PATH = os.path.join(os.path.dirname(__file__), "releases")

# 远端公共根目录
REMOTE_ROOT = "/mnt/data/homes/admin/packages/qt-app"

# 各平台配置
PLATFORMS = {
    "windows_x86": {
        "src_file": f"{APP_NAME}_installer.exe",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_x86.exe",
        "remote": f"{REMOTE_ROOT}/windows",
    },
    "macos_x86_64": {
        "src_file": f"{APP_NAME}.dmg",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_x86_64.dmg",
        "remote": f"{REMOTE_ROOT}/macos/x86_64",
    },
    "macos_aarch64": {
        "src_file": f"{APP_NAME}.dmg",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_aarch64.dmg",
        "remote": f"{REMOTE_ROOT}/macos/aarch64",
    },
    "ubuntu_x86_64": {
        "src_file": f"{APP_NAME}.deb",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_x86_64.deb",
        "remote": f"{REMOTE_ROOT}/ubuntu/x86_64",
    },
    "ubuntu_aarch64": {
        "src_file": f"{APP_NAME}.deb",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_aarch64.deb",
        "remote": f"{REMOTE_ROOT}/ubuntu/aarch64",
    },
}

# SFTP 连接信息
SFTP_SERVER = "192.168.1.111"
SFTP_PORT = 22
SFTP_USER = "root"
SFTP_PWD = "123456"
# -------------------- 逻辑区 --------------------


def upload_platform(platform_key: str) -> None:
    if platform_key not in PLATFORMS:
        raise ValueError(f"未知的 platform_key: {platform_key}")

    cfg = PLATFORMS[platform_key]

    src_path = os.path.join(RELEASES_PATH, cfg["src_file"])
    dst_name = cfg["dst_tpl"]
    dst_path = os.path.join(RELEASES_PATH, dst_name)

    # 若目标文件已存在，先删除（utils.remove 内部已做存在性检查）
    utils.remove(dst_path)
    # 重命名
    os.rename(src_path, dst_path)

    # 生成 update.json
    json_path = os.path.join(RELEASES_PATH, "update.json")
    utils.write_json_describe(VERSION, dst_path, dst_name, json_path)

    # 上传主安装包
    utils.upload_sftp(
        SFTP_SERVER,
        SFTP_PORT,
        SFTP_USER,
        SFTP_PWD,
        f"{cfg['remote']}/{dst_name}",
        dst_path,
    )

    # 上传 update.json
    utils.upload_sftp(
        SFTP_SERVER,
        SFTP_PORT,
        SFTP_USER,
        SFTP_PWD,
        f"{cfg['remote']}/update.json",
        json_path,
    )

    print(f"[{platform_key}] 上传完成 ✔")


def main() -> None:
    parser = argparse.ArgumentParser(description="通用 SFTP 上传脚本")
    parser.add_argument(
        "platform",
        choices=list(PLATFORMS.keys()),
        help="要上传的平台标识",
    )
    args = parser.parse_args()
    upload_platform(args.platform)


if __name__ == "__main__":
    main()

# MacOS
# sudo spctl -vvv --assess --type execute /Applications/Qt-App.app
# 输出如下：
# /Applications/Qt-App.app: accepted
# source=Notarized Developer ID
# origin=Developer ID Application: ********(*******)

# Ubuntu
# dpkg -r Qt-App
# dpkg -i Qt-App_0.0.1_20250810.deb
# dpkg -i --force-overwrite Qt-App_0.0.1_20250810.deb
