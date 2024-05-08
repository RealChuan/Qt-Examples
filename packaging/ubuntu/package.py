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
    copy_exe_cmd = r"cp -rf ./../../code/bin-64/release/.  ./../packet/opt/MyApp/"
    build.execute(copy_exe_cmd)
    copy_exe_cmd = r"cp -rf ./DEBIAN ./../packet/"
    build.execute(copy_exe_cmd)
    copy_exe_cmd = r"cp -rf ./opt ./../packet/"
    build.execute(copy_exe_cmd)
    copy_exe_cmd = r"cp -f logo.png ./../packet/opt/MyApp/"
    build.execute(copy_exe_cmd)


def deploy():
    build.execute(
        "cd ./../packet/opt/MyApp/ \n"
        "export LD_LIBRARY_PATH=/home/runner/work/code/myapp/code/bin-64/release/. \n"
        "'/home/runner/下载/linuxdeployqt-continuous-x86_64.AppImage' MyApp -always-overwrite -no-translations -qmake=/opt/Qt/6.7.0/gcc_64/bin/qmake -unsupported-allow-new-glibc -appimage\n"
    )


def build_deb_and_upload():
    version = "0.0.1"
    current_time = time.strftime("%Y%m%d", time.localtime())
    deb_name = "MyApp{0}_{1}.deb".format(version, current_time)
    out_deb_path = "./../releases/{0}".format(deb_name)

    build.execute("sed -i '/X-AppImage-Version/d' ./../packet/opt/MyApp/MyApp.desktop")
    build.execute("rm -f {0}".format(out_deb_path))
    build.execute(
        "echo 123456 | sudo -S dpkg -b ./../packet/. {0}".format(out_deb_path)
    )
    build.execute("sudo chmod -R 777 ./../releases/")

    # update.json uploadfile
    json_path = "./../releases/update.json"
    utils.generate_json(version, out_deb_path, deb_name, json_path)

    # upload file
    base_url = "http://192.168.1.111:80/apps/wanyoucloud/index.php/61646D696E/admin/Packages/MyApp/Ubuntu/"
    username = "admin"
    password = "password"
    utils.upload_file(base_url + deb_name, username, password, out_deb_path)
    utils.upload_file(base_url + "update.json", username, password, json_path)


def main():
    build_program()

    os.chdir(sys.path[0])
    deploy()
    build_deb_and_upload()


if __name__ == "__main__":
    main()


# dpkg -r MyApp
# dpkg -i MyApp_0.0.1_20230810.deb
# dpkg -i --force-overwrite MyApp_0.0.1_20230810.deb
