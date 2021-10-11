#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <QObject>

struct FileUtilPrivate;
class FileUtil : public QObject
{
    Q_OBJECT
public:
    explicit FileUtil(qint64 days = 30, QObject *parent = nullptr);
    ~FileUtil();

public slots:
    void onWrite(const QString &);

private slots:
    void onFlush();

private:
    QString getFileName(qint64 *now) const;
    bool rollFile(int);
    void autoDelFile();
    void setTimer();

    QScopedPointer<FileUtilPrivate> d_ptr;
};

#endif // FILEUTIL_H
