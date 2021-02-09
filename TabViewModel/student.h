#ifndef STUDENT_H
#define STUDENT_H

#include <QtCore>

struct Student
{
    bool checked = false;
    quint16 id = 0;
    QString name;
    quint16 age = 0;
    QString gender = QObject::tr("MALE");
    quint16 achievement= 0;
    qint16 process = 50;
};

Q_DECLARE_METATYPE(Student);
Q_DECLARE_METATYPE(Student*);

#endif // STUDENT_H
