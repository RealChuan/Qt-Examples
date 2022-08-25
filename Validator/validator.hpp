#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <QValidator>

class IntValidator : public QIntValidator
{
    Q_OBJECT
public:
    using QIntValidator::QIntValidator;

    State validate(QString &input, int &pos) const override;
};

class DoubleValidator : public QDoubleValidator
{
    Q_OBJECT
public:
    using QDoubleValidator::QDoubleValidator;

    State validate(QString &input, int &pos) const override;
};

#endif // VALIDATOR_HPP
