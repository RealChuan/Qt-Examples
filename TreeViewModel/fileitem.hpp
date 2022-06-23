#ifndef FILEITEM_HPP
#define FILEITEM_HPP

#include "treemodel.hpp"

class QFileInfo;
class FileInfo;

class FileItem : public TreeItem
{
public:
    FileItem(const QFileInfo &fileInfo, bool depth = false);
    ~FileItem();

    QVariant data(int column, int role) const override;
    bool setData(int column, const QVariant &data, int role) override;
    Qt::ItemFlags flags(int column) const override;

    FileInfo fileInfo() const;

private:
    void init();

    struct FileItemPrivate;
    QScopedPointer<FileItemPrivate> d_ptr;
};

#endif // FILEITEM_HPP
