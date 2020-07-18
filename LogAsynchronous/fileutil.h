#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <QObject>

class FileUtilPrivate;
class FileUtil : public QObject
{
    Q_OBJECT
public:
    explicit FileUtil(qint64 days = 30, QObject *parent = nullptr);
    ~FileUtil();

public slots:
    void onWrite(const QString&);

private:
    void newDir(const QString &);
    QString getFileName(qint64 *now) const;
    bool rollFile(int);
    void autoDelFile();

    QScopedPointer<FileUtilPrivate> d;
};

#endif // FILEUTIL_H
