#ifndef FILEITEM_HPP
#define FILEITEM_HPP

#include "treemodel.hpp"

class QFileInfo;
class FileInfo;

class FileItem : public TreeItem
{
public:
    explicit FileItem(const QFileInfo &fileInfo, bool depth = false);
    ~FileItem() override;

    [[nodiscard]] auto data(int column, int role) const -> QVariant override;
    auto setData(int column, const QVariant &data, int role) -> bool override;
    [[nodiscard]] auto flags(int column) const -> Qt::ItemFlags override;

    [[nodiscard]] auto fileInfo() const -> FileInfo;

private:
    void init();

    struct FileItemPrivate;
    QScopedPointer<FileItemPrivate> d_ptr;
};

#endif // FILEITEM_HPP
