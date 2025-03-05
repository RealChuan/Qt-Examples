#!/bin/bash -ex

cd "$(dirname "$0")"
source utils.sh

cd ../..
project_root=$PWD
echo "Project root: ${project_root}"

echo "Start compiling..."
qmake="/Users/runner/Qt/6.9.0/macos/bin/qmake"
build_dir="${project_root}/build/Desktop_Qt_6_8_1_macosbit-Release"

rm -rf ${build_dir}
mkdir -p ${build_dir}
cd ${build_dir}
${qmake} ${project_root} -spec macx-clang "CONFIG+=qtquickcompiler"
make -j $(sysctl -n hw.ncpu)
echo "Complied successfully"

cd ${project_root}

packet_dir="${project_root}/packaging/packet"
release_dir="${project_root}/packaging/releases"
mkdir -p ${packet_dir}
mkdir -p ${release_dir}

cp -af -v ${project_root}/bin-64/Release/Qt-App.app ${packet_dir}/
delete_file_or_dir "${release_dir}/Qt-App.app"

# deploy Qt-App
macdeployqt="/Users/fxy/Qt/6.9.0/macos/bin/macdeployqt"
${macdeployqt} ${packet_dir}/Qt-App.app -always-overwrite
cp -af -v ${packet_dir}/Qt-App.app ${release_dir}/

# sign Qt-App
certificate="Developer ID Application: ******(******)"
entitlements="${project_root}/packaging/macos/entitlements.plist"
process_plist "${entitlements}"
security unlock-keychain -p "123456" "$HOME/Library/Keychains/login.keychain"
codesign --options=runtime --timestamp --deep --force --verify --verbose --sign \
    "${certificate}" ${release_dir}/Qt-App.app
sign_list=(
    "${release_dir}/Qt-App.app/Contents/MacOS/CrashReport"
)
for item in ${sign_list[@]}; do
    codesign --options=runtime --timestamp --force --verify --verbose --sign \
        "${certificate}" --entitlements ${entitlements} ${item}
done
codesign --options=runtime --timestamp --force --verify --verbose --sign \
    "${certificate}" "${release_dir}/Qt-App.app/Contents/MacOS/Qt-App"

certificate="Developer ID Installer: ******(******)"
version="0.0.1"
pkg_name="Qt-App.pkg"
out_pkg_path="${release_dir}/${pkg_name}"
dmg_name="Qt-App.dmg"
out_dmg_path="${release_dir}/${dmg_name}"

delete_file_or_dir "${release_dir}/output"
delete_file_or_dir "${out_pkg_path}"
delete_file_or_dir "${pkg_tmp_path}"
delete_file_or_dir "${out_dmg_path}"

mkdir -p ${release_dir}/output

# create pkg
mkdir -p ${packet_dir}/output
# process_plist "${project_dir}/packaging/macos/distribution.xml"
sudo chmod -R +x ${project_dir}/packaging/macos/scripts
pkgbuild --root ${release_dir}/Qt-App.app --identifier org.Qt-App.client \
    --version ${version} \
    --scripts ${project_root}/packaging/macos/scripts \
    --ownership recommended ${release_dir}/output/output2.pkg \
    --install-location /Applications/Qt-App.app
pkg_tmp_path="${release_dir}/final.pkg"
productbuild --distribution ${project_root}/packaging/macos/distribution.xml \
    --resources resources --package-path ${release_dir}/output \
    --version ${version} ${pkg_tmp_path}
# sign pkg
productsign --sign "${certificate}" ${pkg_tmp_path} ${out_pkg_path}
delete_file_or_dir "${pkg_tmp_path}"
notarize_app "${out_pkg_path}"

# install appdmg
brew install npm
npm install -g appdmg

# create dmg
cp -f -v ${project_root}/src/apps/Qt-App/logo.icns ${release_dir}/app.icns
cp -f -v ${project_root}/packaging/macos/dmg.json ${release_dir}/dmg.json
appdmg ${release_dir}/dmg.json ${out_dmg_path}
notarize_app "${out_dmg_path}"

cd "$(dirname "$0")"
./package.py
