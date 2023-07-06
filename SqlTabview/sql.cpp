#include "sql.h"

#include <QMessageBox>
#include <QtSql>

static const auto STUDENTS_SQL = QLatin1String(R"(
                      CREATE TABLE "Students" (
                      "ID" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
                      "Name" text,
                      "Age" integer,
                      "Gender" text,
                      "Achievment" TEXT
                      )
                      )");

static const auto INSERT_STUDENTS_SQL = QLatin1String(R"(
                      INSERT INTO "Students"(Name, Age, Gender, Achievment) VALUES (?, ?, ?, ?)
                      )");

static const auto CATALOG_INDEX_STUDENTS_SQL = QLatin1String(R"(
                      CREATE UNIQUE INDEX "Name"
                      ON "Students" (
                      "Name" COLLATE BINARY ASC
                      )
                      )");

Sql::Sql(QObject *parent)
    : QObject(parent)
{}

bool Sql::searchSQLite(QWidget *parent)
{
    if (QSqlDatabase::drivers().contains("QSQLITE")) {
        return true;
    }
    QMessageBox::critical(parent,
                          tr("Unable to load database"),
                          tr("This demo needs the SQLITE driver"));
    return false;
}

QSqlError Sql::createOrOpenSqlite()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("tabview.db");

    if (!db.open()) {
        return db.lastError();
    }
    QStringList tables = db.tables();
    if (tables.contains("Students", Qt::CaseInsensitive)) {
        return QSqlError();
    }

    QSqlQuery q;
    if (!q.exec(STUDENTS_SQL)) {
        return q.lastError();
    }
    if (!q.prepare(INSERT_STUDENTS_SQL)) {
        return q.lastError();
    }

    addStudent(q, QLatin1String("Jason"), 15, QLatin1String("MALE"), 66);
    addStudent(q, QLatin1String("Lily"), 13, QLatin1String("FEMALE"), 85);
    addStudent(q, QLatin1String("Odin"), 16, QLatin1String("MALE"), 68);
    addStudent(q, QLatin1String("Nieo"), 14, QLatin1String("MALE"), 77);
    addStudent(q, QLatin1String("Willion"), 12, QLatin1String("MALE"), 89);

    if (!q.exec(CATALOG_INDEX_STUDENTS_SQL)) {
        return q.lastError();
    }

    db.close();

    return QSqlError();
}

void Sql::addStudent(
    QSqlQuery &q, const QString &name, int age, const QString &gender, int achievment)
{
    q.addBindValue(name);
    q.addBindValue(age);
    q.addBindValue(gender);
    q.addBindValue(achievment);
    q.exec();
}
