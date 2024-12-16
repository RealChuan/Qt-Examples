# -*- coding: utf-8 -*-


import os
import datetime
import subprocess
import sys

build_list = [
    {
        "qmake": r"C:\Qt\6.8.1\msvc2022_64\bin\qmake.exe",
        "qmake_params": r'"CONFIG+=qtquickcompiler"',
        "jom": r"C:\Qt\Tools\QtCreator\bin\jom\jom.exe",
        "env_bat": r'C:\"Program Files (x86)"\"Microsoft Visual Studio"\2019\Community\VC\Auxiliary\Build\vcvarsall.bat x64',
        "project": r"C:\code\qt-app\qt-app.pro",
        "build_directory": r"C:\work\code\qt-app\build\Desktop_Qt_6_8_1_msvc2022_64bit-Release",
    }
]


class Builder:
    def __init__(self, project, qmake, qmake_params, jom, env_bat, build_directory):
        self.project = project
        self.qmake = qmake
        self.qmake_params = qmake_params
        self.jom = jom
        self.env_bat = env_bat
        self.build_directory = build_directory

        os.makedirs(build_directory, exist_ok=True)

        (self.qmake_path, self.qmake_name) = os.path.split(qmake)
        (self.jom_path, self.jom_name) = os.path.split(jom)
        (self.project_path, self.project_name) = os.path.split(project)

        self.qt_version = self.qmake_path.split("\\")[-3]
        self.compiler_version = self.qmake_path.split("\\")[-2]

    def execute_qmake_cmd_line(self):
        create_qmake_cmd_line = (
            self.qmake + " " + self.project + " -spec win32-msvc " + self.qmake_params
        )
        return True if execute_cmd(create_qmake_cmd_line) else False

    def execute_make_cmd_line(self):
        cmd_line = "{0}".format(self.jom)
        return True if execute_cmd(cmd_line) else False

    def execute_make_clean_cmd_line(self):
        if os.path.exists("Makefile") | os.path.exists("makefile"):
            return True if execute_cmd("{0} clean".format(self.jom)) else False
        return True

    def init_env(self):
        tmp = os.popen("{0} && {1}".format(self.env_bat, "set"))
        env_str = tmp.read()
        tmp.close()

        env_list = str(env_str).split("\n")

        for each in env_list:
            if "=" in each:
                key = each.split("=")[0].lower()
                value = each.split("=")[1]
                os.environ[key] = value

        return True

    def build(self):
        os.chdir(self.build_directory)
        print("Starting build [{0}]".format(os.path.splitext(self.project_name)[0]))

        isOk = True
        status = self.init_env()
        isOk = isOk and status
        print("Init env:   ", "Success" if status else "Failed")

        status = self.execute_make_clean_cmd_line()
        isOk = isOk and status
        print("Make clean: ", "Success" if status else "Failed")

        status = self.execute_qmake_cmd_line()
        isOk = isOk and status
        print("QMake:      ", "Success" if status else "Failed")

        status = self.execute_make_cmd_line()
        isOk = isOk and status
        print("Jom:       ", "Success" if status else "Failed")

        print("End build   ", "Success" if isOk else "Failed")
        return isOk


def build(build_list):
    for each in build_list:
        qmake = each.get("qmake")
        qmake_params = (
            "" if each.get("qmake_params") == None else each.get("qmake_params")
        )
        jom = each.get("jom")
        env_bat = each.get("env_bat")
        project = each.get("project")
        build_directory = each.get("build_directory")

        builder = Builder(project, qmake, qmake_params, jom, env_bat, build_directory)
        if builder.build() == False:
            print("[{0}] Deploy failed!!!".format(datetime.datetime.now()))
            exit(-1)


def execute_cmd(cmd):
    print(cmd)
    result = subprocess.run(
        cmd, encoding="gbk", stdout=sys.stdout, stderr=sys.stderr, shell=True
    )
    if result.returncode != 0:
        print(result.stderr)

    return True if result.returncode == 0 else False


def change_powershell_execution_policy(execution_policy):
    subprocess.run(
        [
            "powershell",
            "-Command",
            f"Set-ExecutionPolicy {execution_policy} -Scope CurrentUser -Force",
        ]
    )


def execute_powershell(script_path, is_script):
    print("Execute powershell script: {0}".format(script_path))
    change_powershell_execution_policy("Unrestricted")
    if is_script:
        command = f"& '{script_path}'"
    else:
        command = script_path

    proc = subprocess.Popen(
        ["powershell", "-Command", command],
        stdout=sys.stdout,
        stderr=sys.stderr,
        shell=True,
    )

    return proc.wait()


if __name__ == "__main__":
    print("[{0}] Start deploy...".format(datetime.datetime.now()))
    build(build_list)
    print("[{0}] Deploy success!!!".format(datetime.datetime.now()))
