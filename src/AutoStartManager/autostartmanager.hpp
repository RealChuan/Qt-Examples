#pragma once

#include <expected>

#include <QString>

namespace Utils {

// 查询当前应用是否已配置开机自启动。
// 成功返回当前状态，失败返回错误描述。
[[nodiscard]] std::expected<bool, QString> isAutoRunStart();

// 设置开机自启动。成功返回空，失败返回错误描述。
[[nodiscard]] std::expected<void, QString> setAutoRunStart(bool run);

} // namespace Utils
