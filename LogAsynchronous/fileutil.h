#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <QObject>

class FileUtil : public QObject
{
    Q_OBJECT
public:
    explicit FileUtil(QObject *parent = nullptr);
    ~FileUtil() override;

public slots:
    void onWrite(const QString & /*msg*/);

private slots:
    void onFlush();

private:
    auto rollFile(int /*count*/) -> bool;
    void setTimer();

    class FileUtilPrivate;
    QScopedPointer<FileUtilPrivate> d_ptr;
};

#endif // FILEUTIL_H
