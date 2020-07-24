#ifndef STUDENTMODEL_H
#define STUDENTMODEL_H

#include "treeitem.h"

#include <QTreeView>

class StudentItem : public TreeItem
{
public:
    StudentItem(const quint16 id, const QString &name, const quint16 age,
                const QString gender, const quint16 achievement)
        : m_id(id)
        , m_name(name)
        , m_age(age)
        , m_gender(gender)
        , m_achievement(achievement)
    {}

    void setID(quint16 id)                      { m_id = id; }
    void setName(const QString &name)           { m_name = name; }
    void setAge(quint16 age)                    { m_age = age; }
    void setGender(QString gender)              { m_gender = gender; }
    void setAchievement(quint16 achievement)    { m_achievement = achievement; }

    quint16 Id() const              { return m_id; }
    QString name() const            { return m_name; }
    quint16 age() const             { return m_age; }
    QString gender() const          { return m_gender; }
    quint16 achievement() const     { return m_achievement; }

    int columnCount() const { return 5; }
    QVariant data(int column, int role) const override;
    bool setData(int column, const QVariant &data, int role) override;
    Qt::ItemFlags flags(int column) const override;

private:
    quint16 m_id            = 77;
    QString m_name          = "Jhon";
    quint16 m_age           = 88;
    QString m_gender        = "MALE";
    quint16 m_achievement   = 99;
    QObject *owner;
};

Q_DECLARE_METATYPE(QVector<StudentItem>)

typedef QVector<StudentItem> StuVec;
typedef QPair<StudentItem, StuVec> ClassPair;

class TreeView : public QTreeView
{
public:
    TreeView(QWidget *parent = nullptr);
    void updateValueTree(const QVector<ClassPair>& cp);

private:
    void init();

    BaseTreeModel *m_treeModel;
};

#endif // STUDENTMODEL_H
