#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <QValidator>

class IntValidator : public QIntValidator
{
    Q_OBJECT
public:
    using QIntValidator::QIntValidator;

    auto validate(QString &input, int &pos) const -> State override;
};

class DoubleValidator : public QDoubleValidator
{
    Q_OBJECT
public:
    using QDoubleValidator::QDoubleValidator;

    auto validate(QString &input, int &pos) const -> State override;
};

#endif // VALIDATOR_HPP
