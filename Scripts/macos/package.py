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

sign = "D9CD3CA0Axxxxxxxxxxxxxxxxxxxxxxx069E"  # sign code

build.execute(
    "cp -af ./../../bin-64/release/MyApplication.app/ ./../packet/MyApplication.app/")
build.execute("rm -rf ./../releases/MyApplication.app")
build.execute(
    "{0} ./../packet/MyApplication.app -always-overwrite".format(r'/Users/fxy/Qt/6.3.0/clang_64/bin/macdeployqt'))
build.execute(
    "cp -af ./../packet/MyApplication.app ./../releases/MyApplication.app")

build.execute("codesign --options=runtime --timestamp --force --verify --verbose -f -s {0} {1} --deep".format(
    sign, "./../releases/MyApplication.app"))

build.execute(
    "codesign --options=runtime --timestamp --force --verify --verbose -s {0} --entitlements {1} {2}".format(
        sign, "./QtWebEngineProcess.entitlements", "./../releases/MyApplication.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app/Contents/MacOS/QtWebEngineProcess"))


build.execute("codesign --options=runtime --timestamp --force --verify --verbose -f -s {0} {1}".format(
    sign, "./../releases/MyApplication.app/Contents/MacOS/MyApplication"))

build.execute("cp -af ./../../src/app.icns ./../releases/app.icns")
build.execute("cp -af ./dmg.json ./../releases/dmg.json")

miniversion = "0.0.1"
current_time = time.strftime('%Y%m%d', time.localtime())
dmg_name = "MyApplication_V{0}_{1}.dmg".format(miniversion, current_time)
out_dmg_path = "./../releases/{0}".format(dmg_name)
build.execute("rm -rf {0}".format(out_dmg_path))

build.execute("appdmg ./../releases/dmg.json {0}".format(out_dmg_path))
build.execute("codesign --options=runtime --timestamp --force --verify --verbose -f -s {0} {1} --deep".format(
    sign, out_dmg_path))
build.execute(
    '''xcrun altool --notarize-app --primary-bundle-id "bundle-id" --username "username" --password "password" --file {0}'''.format(out_dmg_path))

# sudo spctl --verbose=4 --assess --type execute Dog.app
# 输出如下：
#Dog.app: accepted // 公证后变为accepted
# source=Notarized Developer ID

# update.json uploadfile
json_path = "./../releases/update.json"
generate_json(miniversion, out_dmg_path, dmg_name, json_path)
upload_file(
    json_path, "http://192.168.55.116/webdav/user_1/MyApplication/macos/update.json")
upload_file(
    out_dmg_path, "http://192.168.55.116/webdav/user_1/MyApplication/macos/{}".format(dmg_name))
