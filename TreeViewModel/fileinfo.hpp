#ifndef FILEINFO_HPP
#define FILEINFO_HPP

#include <QAbstractFileIconProvider>
#include <QIcon>
#include <QtCore>

QString bytesToString(qint64 bytes);

class FileInfoData : public QSharedData
{
public:
    FileInfoData() {}
    FileInfoData(const FileInfoData &other)
        : QSharedData(other)
        , fileInfo(other.fileInfo)
        , icon(other.icon)
        , modifyTime(other.modifyTime)
        , type(other.type)
        , size(other.size)
        , checked(other.checked)
    {}

    QFileInfo fileInfo;
    QIcon icon;
    QString modifyTime;
    QString type;
    QString size;

    bool checked = false;
};

class FileInfo
{
public:
    FileInfo()
        : d_ptr(new FileInfoData)
    {}

    FileInfo(const QFileInfo &fileInfo)
        : d_ptr(new FileInfoData)
    {
        setFileInfo(fileInfo);

        QAbstractFileIconProvider provider;
        setIcon(provider.icon(fileInfo));
        setModifyTime(fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
        setType(provider.type(fileInfo));
        setSize(fileInfo.isDir() ? "-" : bytesToString(fileInfo.size()));
    }

    FileInfo(const FileInfo &other)
        : d_ptr(other.d_ptr)
    {}

    ~FileInfo() {}

    bool operator==(const FileInfo &other) const
    {
        return d_ptr->fileInfo == other.d_ptr->fileInfo;
    }
    bool operator!=(const FileInfo &other) const { return !(*this == other); }

    void setFileInfo(const QFileInfo &fileInfo) { d_ptr->fileInfo = fileInfo; }
    QFileInfo fileInfo() const { return d_ptr->fileInfo; }

    void setIcon(const QIcon &icon) { d_ptr->icon = icon; }
    QIcon icon() const { return d_ptr->icon; }

    void setType(const QString &type) { d_ptr->type = type; }
    QString type() const { return d_ptr->type; }

    void setModifyTime(const QString &modifyTime) { d_ptr->modifyTime = modifyTime; }
    QString modifyTime() const { return d_ptr->modifyTime; }

    void setSize(const QString &size) { d_ptr->size = size; }
    QString size() const { return d_ptr->size; }

    void setChecked(bool checked) { d_ptr->checked = checked; }
    bool checked() const { return d_ptr->checked; }

private:
    QExplicitlySharedDataPointer<FileInfoData> d_ptr;
};

Q_DECLARE_METATYPE(FileInfo)

#endif // FILEINFO_HPP
