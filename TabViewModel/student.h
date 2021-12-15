#ifndef STUDENT_H
#define STUDENT_H

#include <QtCore>

class StudentData : public QSharedData
{
public:
    StudentData() {}
    StudentData(const StudentData &other)
        : QSharedData(other)
        , checked(other.checked)
        , id(other.id)
        , name(other.name)
        , age(other.age)
        , gender(other.gender)
        , achievement(other.achievement)
        , process(other.process)
    {}
    ~StudentData() {}

    bool checked = false;
    quint16 id = 0;
    QString name;
    quint16 age = 0;
    QString gender = QObject::tr("MALE");
    quint16 achievement = 0;
    qint16 process = 50;
};

class Student
{
public:
    Student()
        : d_ptr(new StudentData)
    {}
    Student(bool checked,
            quint16 id,
            const QString &name,
            quint16 age,
            const QString &gender,
            quint16 achievement,
            qint16 process)
        : d_ptr(new StudentData)
    {
        setChecked(checked);
        setId(id);
        setName(name);
        seAge(age);
        setGender(gender);
        seAchievement(achievement);
        seProcess(process);
    }
    Student(const Student &other)
        : d_ptr(other.d_ptr)
    {}

    void setChecked(bool checked) { d_ptr->checked = checked; }
    bool checked() const { return d_ptr->checked; }

    void setId(quint16 id) { d_ptr->id = id; }
    quint16 id() const { return d_ptr->id; }

    void setName(const QString &name) { d_ptr->name = name; }
    QString name() const { return d_ptr->name; }

    void seAge(quint16 age) { d_ptr->age = age; }
    quint16 age() const { return d_ptr->age; }

    void setGender(const QString &gender) { d_ptr->gender = gender; }
    QString gender() const { return d_ptr->gender; }

    void seAchievement(quint16 achievement) { d_ptr->achievement = achievement; }
    quint16 achievement() const { return d_ptr->achievement; }

    void seProcess(qint16 process) { d_ptr->process = process; }
    qint16 process() const { return d_ptr->process; }

private:
    QExplicitlySharedDataPointer<StudentData> d_ptr;
};

Q_DECLARE_METATYPE(Student)

#endif // STUDENT_H
