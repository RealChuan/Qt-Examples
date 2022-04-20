# -*- coding: utf-8 -*-

import os
import time
import json
import hashlib
import requests
import build
import sys


def generate_json(version, md5_file_path, file_name, json_path):
    with open(md5_file_path, 'rb') as fp:
        data = fp.read()
    md5 = hashlib.md5(data).hexdigest()
    current_time = time.strftime('%Y%m%d', time.localtime())
    dict = {"version": version, "buildVersion": int(
        current_time), "name": file_name, "md5": md5}
    with open(json_path, 'w', encoding='utf-8') as f:
        f.write(json.dumps(dict, indent=4))


def upload_file(file_path, url):
    with open(file_path, 'rb') as f:
        response = requests.put(url=url, data=f)
        response.raise_for_status()
        print(response.status_code, response.text)


build.build(build.build_list)
os.chdir(sys.path[0])
build.execute(
    "xcopy /s /y \".\\..\\..\\bin-32\\release\\\" \".\\..\\packet\\\"")
build.execute(
    "{0} --force --no-translations .\\..\\packet\\MyApplication.exe".format(r'C:\Qt\6.3.0\msvc2019\bin\windeployqt.exe'))

miniVersion = "0.0.1"
current_time = time.strftime('%Y%m%d', time.localtime())
exe_name = "MyApplication_V{0}_{1}.exe".format(miniVersion, current_time)
out_exe_path = ".\\..\\releases\\{0}".format(exe_name)
build.execute("del /q {0}".format(out_exe_path))
build.execute("del /q .\\..\\releases\\MyApplication_Installation_Package.exe")

build.execute("{0} package.iss".format(
    r'C:\"Program Files (x86)"\"Inno Setup 6"\ISCC.exe'))
os.rename(
    ".\\..\\releases\\MyApplication_Installation_Package.exe", out_exe_path)

# json upload
json_path = ".\\..\\releases\\update.json"
generate_json(miniVersion, out_exe_path, exe_name, json_path)
upload_file(
    json_path, "http://192.168.55.113/webdav/MyApplication/windows/update.json")
upload_file(
    out_exe_path, "http://192.168.55.113/webdav/MyApplication/windows/{}".format(exe_name))
