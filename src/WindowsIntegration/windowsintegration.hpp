#pragma once

#include <QString>

namespace Utils {

bool isRegisteredInMyComputer(const QString &guid);

bool setRegisteredInMyComputer(bool registerApp, const QString &guid);

} // namespace Utils
