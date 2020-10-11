#ifndef STUDENT_H
#define STUDENT_H

#include <QtCore>

class Student
{
public:
    Student() {}
    Student(quint16 id, const QString &name, quint16 age,
            QString gender, quint16 achievement)
        : m_id(id), m_name(name), m_age(age),
          m_gender(gender), m_achievement(achievement){}

    void setchecked(bool checked) { m_checked = checked; }
    void setID(quint16 id) { m_id = id; }
    void setName(const QString &name) { m_name = name; }
    void setAge(quint16 age) { m_age = age; }
    void setGender(QString gender) { m_gender = gender; }
    void setAchievement(quint16 achievement) { m_achievement = achievement; }

    bool checked() const { return m_checked; }
    quint16 Id() const { return m_id; }
    QString name() const { return m_name; }
    quint16 age() const { return m_age; }
    QString  gender() const { return m_gender; }
    quint16 achievement() const { return m_achievement; }

private:
    bool m_checked = false;
    quint16 m_id = 0;
    QString m_name;
    quint16 m_age = 0;
    QString m_gender = "MALE";
    quint16 m_achievement= 0;
};

#endif // STUDENT_H
