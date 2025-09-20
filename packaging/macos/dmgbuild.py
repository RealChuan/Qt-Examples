# -*- coding: utf-8 -*-
"""
dmgbuild 规范配置文件
用法：
    dmgbuild -s ./packaging/macos/dmgbuild.py "Qt-App" bin-64/RelWithDebInfo/Qt-App.dmg
"""

import os.path as P
import plistlib

# --------------------------------------------------------------------------- #
#  唯一需要手工维护的「配置区」—— 改这里即可，其余代码不用动
# --------------------------------------------------------------------------- #

APP_NAME = "Qt-App"  # 主 App 名称（不含 .app 后缀）

CONFIG = dict(
    app=f"{APP_NAME}.app",  # 主 App 名称（支持 defines 覆盖）
    format="UDZO",  # 镜像格式
    size=None,  # 留空让 dmgbuild 自动计算
    volume_label=APP_NAME,  # 卷标
    background="builtin-arrow",  # 背景
    default_view="icon-view",  # 初始视图
    compression_level=9,  # 压缩等级
    internet_enable=True,  # 网络挂载
    window_rect=((100, 100), (640, 360)),  # (x, y), (w, h)
    icon_size=128,  # 图标大小
    grid_spacing=100,  # 网格间距
    arrange_by=None,  # 排序方式
    scroll_position=(0, 0),
    label_pos="bottom",  # 文字位置
    text_size=16,
    show_icon_preview=False,
    # 图标坐标
    icon_locations={
        f"{APP_NAME}.app": (140, 120),
        "Applications": (500, 120),
    },
)
# --------------------------------------------------------------------------- #

# 允许命令行 -D key=value 覆盖
CONFIG.update({k: v for k, v in (defines or {}).items() if k in CONFIG})


# 自动解析 App 图标
def _icon_from_app(app_path):
    plist = plistlib.load(open(P.join(app_path, "Contents", "Info.plist"), "rb"))
    name = plist["CFBundleIconFile"]
    return P.join(
        app_path,
        "Contents",
        "Resources",
        name if name.endswith(".icns") else name + ".icns",
    )


application = CONFIG.pop("app")
icon = _icon_from_app(application)

# dmgbuild 需要的全局变量
locals().update(
    {
        "application": application,
        "format": CONFIG.pop("format"),
        "size": CONFIG.pop("size"),
        "files": [application],
        "symlinks": {"Applications": "/Applications"},
        "badge_icon": icon,
        "icon_locations": CONFIG.pop("icon_locations"),
        "background": CONFIG.pop("background"),
        "volume_label": CONFIG.pop("volume_label"),
        "applications_link": True,
        "internet_enable": CONFIG.pop("internet_enable"),
        "compression_level": CONFIG.pop("compression_level"),
        "window_rect": CONFIG.pop("window_rect")[:2],
        "default_view": CONFIG.pop("default_view"),
        "show_icon_preview": CONFIG.pop("show_icon_preview"),
        "arrange_by": CONFIG.pop("arrange_by"),
        "grid_offset": (0, 0),
        "grid_spacing": CONFIG.pop("grid_spacing"),
        "scroll_position": CONFIG.pop("scroll_position"),
        "label_pos": CONFIG.pop("label_pos"),
        "text_size": CONFIG.pop("text_size"),
        "icon_size": CONFIG.pop("icon_size"),
        "include_icon_view_settings": "auto",
        "include_list_view_settings": "auto",
    }
)
