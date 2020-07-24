#ifndef STUDENTTABLE_H
#define STUDENTTABLE_H

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QTableView>

class Student : public QObject
{
public:
    Student(QObject *parent = nullptr) : QObject(parent){}
    Student(quint16 id, const QString &name, quint16 age,
            QString gender, quint16 achievement, QObject *parent = nullptr)
        : QObject(parent), m_id(id), m_name(name), m_age(age),
          m_gender(gender), m_achievement(achievement){}

    void setID(quint16 id)                      { m_id = id; }
    void setName(const QString &name)           { m_name = name; }
    void setAge(quint16 age)                    { m_age = age; }
    void setGender(QString gender)              { m_gender = gender; }
    void setAchievement(quint16 achievement)    { m_achievement = achievement; }

    quint16 Id() const              { return m_id; }
    QString name() const            { return m_name; }
    quint16 age() const             { return m_age; }
    QString  gender() const         { return m_gender; }
    quint16 achievement() const     { return m_achievement; }

private:
    quint16 m_id            = 77;
    QString m_name          = "Jhon";
    quint16 m_age           = 88;
    QString  m_gender       = "MALE";
    quint16 m_achievement   = 99;
};

class StuedentTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit StuedentTableModel(QObject *parent = nullptr) : QAbstractTableModel(parent){}
    int rowCount(const QModelIndex&  = QModelIndex()) const { return m_students.count(); }
    int columnCount(const QModelIndex&  = QModelIndex()) const { return 5; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void setStudents(const QList<Student*>& students);

private:
    QList<Student*> m_students;
};

class StudentsTable : public QTableView
{
public:
    StudentsTable(QWidget *parent = nullptr);

    void setStudents(const QList<Student*>& students);

private:
    StuedentTableModel *stuModel;
};

class EditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit EditDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

protected:
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget* editor,const QModelIndex& index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const  QModelIndex &index) const Q_DECL_OVERRIDE;
};

#endif // STUDENTTABLE_H
