# -*- coding: utf-8 -*-

import os
import time
import build
import sys


sys.path.append(sys.path[0] + "/../")
import utils


def build_program():
    build.build(build.build_list)

    os.chdir(sys.path[0])
    build.execute('xcopy /s /y ".\\..\\..\\src\\bin-32\\release\\" ".\\..\\packet\\"')


def deploy(program_path):
    windeployqt = r"C:\Qt\6.6.1\msvc2019_64\bin\windeployqt.exe"
    deploy_cmd = (
        f'{windeployqt} --force --no-translations --compiler-runtime "{program_path}"'
    )
    build.execute(deploy_cmd)


def package_iss(iss_path):
    iscc_exe = r'"C:\Program Files (x86)\Inno Setup 6\ISCC.exe"'
    package_cmd = f"{iscc_exe} {iss_path}"
    build.execute(package_cmd)


def upload_file():
    version = "0.0.1"
    current_time = time.strftime("%Y%m%d", time.localtime())
    exe_name = "MyApp_V{0}_{1}.exe".format(version, current_time)
    out_exe_path = ".\\..\\releases\\{0}".format(exe_name)

    build.execute("del /q {0}".format(out_exe_path))
    build.execute("del /q .\\..\\releases\\MyApp_Installation_Package.exe")
    os.rename(".\\..\\releases\\MyApp_Installation_Package.exe", out_exe_path)

    # generate update.json
    json_path = ".\\..\\releases\\update.json"
    utils.generate_json(version, out_exe_path, exe_name, json_path)

    # upload file
    base_url = "http://192.168.1.111:80/webdav/admin/Packages/MyApp/Windows/"
    username = "admin"
    password = "password"
    utils.upload_file(base_url + exe_name, username, password, out_exe_path)
    utils.upload_file(base_url + "update.json", username, password, json_path)


def main():
    build_program()
    deploy(sys.path[0] + "/../packet/MyApp.exe")
    package_iss(sys.path[0] + "/package.iss")
    upload_file()


if __name__ == "__main__":
    main()
