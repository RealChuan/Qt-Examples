# -*- coding: utf-8 -*-

import os
import time
import sys

sys.path.append(sys.path[0] + "/../")
import utils


def rename_and_upload():
    version = "0.0.1"
    current_time = time.strftime("%Y%m%d", time.localtime())
    dmg_path = "{0}/../releases/Qt-App.dmg".format(sys.path[0])
    dmg_name = "Qt-App_{0}_{1}_x86_64.dmg".format(version, current_time)
    out_dmg_path = "{0}/../releases/{1}".format(sys.path[0], dmg_name)
    utils.removeFile(out_dmg_path)
    os.rename(dmg_path, out_dmg_path)

    json_path = "{0}/../releases/update.json".format(sys.path[0])
    utils.generate_json(version, out_dmg_path, dmg_name, json_path)

    base_url = "http://192.168.1.111:80/webdav/index.php/admin/packages/Qt-App/macos/"
    username = "admin"
    password = "password"
    utils.upload_file(base_url + dmg_name, username, password, out_dmg_path)
    utils.upload_file(base_url + "update.json", username, password, json_path)


def main():
    rename_and_upload()


if __name__ == "__main__":
    main()


# sudo spctl -vvv --assess --type execute /Applications/Qt-App.app
# 输出如下：
# /Applications/Qt-App.app: accepted
# source=Notarized Developer ID
# origin=Developer ID Application: ******(******)
