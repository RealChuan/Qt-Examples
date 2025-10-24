#include "task.hpp"
#include "functions.hpp"

#include <QDebug>

Task::~Task()
{
    qDebug() << "~Task";
}

void Task::onDo(const QString &text)
{
    qDebug() << text << Functions::getCurrentThreadIDStr();
}
