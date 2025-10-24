#ifndef TASK_HPP
#define TASK_HPP

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

class Task : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    ~Task() override;

public slots:
    void onDo(const QString &text);
};

#endif // TASK_HPP
