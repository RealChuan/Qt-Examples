#include "validator.hpp"

auto IntValidator::validate(QString &input, int &pos) const -> QValidator::State
{
    const State originalRes = QIntValidator::validate(input, pos);
    if (originalRes != Intermediate || input.isEmpty()) {
        return originalRes;
    }

    const auto extracted = locale().toLongLong(input);
    if (extracted > 0) {
        if (extracted > top() && -extracted < bottom()) {
            return Invalid;
        }
    } else if (extracted < bottom()) {
        return Invalid;
    }
    return originalRes;
}

auto DoubleValidator::validate(QString &input, int &pos) const -> QValidator::State
{
    const State originalRes = QDoubleValidator::validate(input, pos);
    if (originalRes != Intermediate || input.isEmpty()) {
        return originalRes;
    }

    const auto extracted = locale().toDouble(input);
    if (extracted > 0) {
        if (extracted > top() && -extracted < bottom()) {
            return Invalid;
        }
    } else if (extracted < bottom()) {
        return Invalid;
    }
    return originalRes;
}
