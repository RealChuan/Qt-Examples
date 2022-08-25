#include "validator.hpp"

QValidator::State IntValidator::validate(QString &input, int &pos) const
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

QValidator::State DoubleValidator::validate(QString &input, int &pos) const
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
