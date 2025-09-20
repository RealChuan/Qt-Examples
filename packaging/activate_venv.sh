#!/bin/bash -ex

VENV_NAME="venv" # 定义虚拟环境目录名

# 检查虚拟环境是否存在
if [ ! -d "$VENV_NAME" ]; then
    echo "创建虚拟环境 '$VENV_NAME'..."
    python3 -m venv "$VENV_NAME" || {
        echo "创建失败"
        exit 1
    }
else
    echo "检测到虚拟环境 '$VENV_NAME' 已存在"
fi

# 自动激活虚拟环境（需通过 source 执行脚本）
echo "激活虚拟环境..."
source "$VENV_NAME/bin/activate"

# 提示用户后续操作
echo "虚拟环境已激活，当前 Python 路径: $(which python)"
