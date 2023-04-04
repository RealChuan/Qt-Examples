#ifndef FILEINFO_HPP
#define FILEINFO_HPP

#include <QAbstractFileIconProvider>
#include <QIcon>
#include <QtCore>

auto bytesToString(qint64 bytes) -> QString;

class FileInfoData : public QSharedData
{
public:
    FileInfoData() = default;
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

    explicit FileInfo(const QFileInfo &fileInfo)
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

    ~FileInfo() = default;

    auto operator==(const FileInfo &other) const -> bool
    {
        return d_ptr->fileInfo == other.d_ptr->fileInfo;
    }
    auto operator!=(const FileInfo &other) const -> bool { return !(*this == other); }

    void setFileInfo(const QFileInfo &fileInfo) { d_ptr->fileInfo = fileInfo; }
    [[nodiscard]] QFileInfo fileInfo() const { return d_ptr->fileInfo; }

    void setIcon(const QIcon &icon) { d_ptr->icon = icon; }
    [[nodiscard]] QIcon icon() const { return d_ptr->icon; }

    void setType(const QString &type) { d_ptr->type = type; }
    [[nodiscard]] auto type() const -> QString { return d_ptr->type; }

    void setModifyTime(const QString &modifyTime) { d_ptr->modifyTime = modifyTime; }
    [[nodiscard]] auto modifyTime() const -> QString { return d_ptr->modifyTime; }

    void setSize(const QString &size) { d_ptr->size = size; }
    [[nodiscard]] auto size() const -> QString { return d_ptr->size; }

    void setChecked(bool checked) { d_ptr->checked = checked; }
    [[nodiscard]] auto checked() const -> bool { return d_ptr->checked; }

private:
    QExplicitlySharedDataPointer<FileInfoData> d_ptr;
};

Q_DECLARE_METATYPE(FileInfo)

#endif // FILEINFO_HPP
