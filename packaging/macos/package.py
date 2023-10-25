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
    build.execute("cp -af ./../../src/bin-64/release/MyApp.app/ ./../packet/MyApp.app/")
    build.execute("rm -rf ./../releases/MyApp.app")


def deploy(program_path):
    macdeployqt = r"/Users/fxy/Qt/6.6.0/clang_64/bin/macdeployqt"
    build.execute("{0} {1} -always-overwrite".format(macdeployqt, program_path))


def sign():
    certificate = "Developer ID Application: ******(******)"
    entitlements = "./entitlements.plist"
    build.execute("plutil -convert xml1 {0}".format(entitlements))
    build.execute(
        'security unlock-keychain -p "password" "$HOME/Library/Keychains/login.keychain"'
    )
    build.execute(
        'codesign --options=runtime --timestamp --deep --force --verify --verbose --sign "{0}" "{1}"'.format(
            certificate, "./../releases/MyApp.app"
        )
    )
    build.execute(
        'codesign --options=runtime --timestamp --force --verify --verbose --sign "{0}" --entitlements "{1}" "{2}"'.format(
            certificate,
            entitlements,
            "./../releases/MyApp.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app/Contents/MacOS/QtWebEngineProcess",
        )
    )
    build.execute(
        'codesign --options=runtime --timestamp --force --verify --verbose --sign "{0}" "{1}"'.format(
            certificate, "./../releases/MyApp.app/Contents/MacOS/MyApp"
        )
    )


def main():
    build_program()

    os.chdir(sys.path[0])
    deploy(sys.path[0] + "/../packet/MyApp.app")
    build.execute("cp -af ./../packet/MyApp.app ./../releases/MyApp.app")
    sign()

    version = "0.0.1"
    current_time = time.strftime("%Y%m%d", time.localtime())
    dmg_name = "MyApp{0}_{1}.dmg".format(version, current_time)
    out_dmg_path = "./../releases/{0}".format(dmg_name)
    build.execute("cp -f ./../../src/app.icns ./../releases/app.icns")
    build.execute("cp -f ./dmg.json ./../releases/dmg.json")
    build.execute("rm -f {0}".format(out_dmg_path))
    build.execute("appdmg ./../releases/dmg.json {0}".format(out_dmg_path))

    build.execute(
        """xcrun altool --notarize-app --primary-bundle-id "com.youth.myapp" --username "1070753498@qq.com" --password "password" --file {0}""".format(
            out_dmg_path
        )
    )

    # generate update.json
    json_path = "./../releases/update.json"
    utils.generate_json(
        version,
        out_dmg_path,
        dmg_name,
        json_path,
    )

    # upload file
    utils.upload_file(
        "http://192.168.1.111:80/webdav/admin/Packages/MyApp/MacOS/{0}".format(
            dmg_name
        ),
        "admin",
        "password",
        out_dmg_path,
    )
    utils.upload_file(
        "http://192.168.1.111:80/webdav/admin/Packages/MyApp/MacOS/update.json",
        "admin",
        "password.",
        json_path,
    )


if __name__ == "__main__":
    main()

# sudo spctl -vvv --assess --type execute /Applications/MyApp.app
# 输出如下：
# /Applications/MyApp.app: accepted
# source=Notarized Developer ID
# origin=Developer ID Application: ******(******)
