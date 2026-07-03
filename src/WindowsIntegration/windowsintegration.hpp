#pragma once

#include <QString>

#include <expected>

namespace Utils {

// 查询当前应用是否已注册到 Windows "此电脑"命名空间。
// 成功返回当前状态，失败返回错误描述。
[[nodiscard]] std::expected<bool, QString> isRegisteredInMyComputer(const QString &guid);

// 设置是否在 Windows "此电脑"中显示应用图标。
// 成功返回空，失败返回错误描述。
[[nodiscard]] std::expected<void, QString> setRegisteredInMyComputer(bool registerApp,
                                                                     const QString &guid);

} // namespace Utils
