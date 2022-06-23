#include "fileitem.hpp"
#include "fileinfo.hpp"

#include <QFileIconProvider>
#include <QFileInfo>

struct FileItem::FileItemPrivate
{
    FileInfo fileInfo;
};

FileItem::FileItem(const QFileInfo &fileInfo, bool depth)
    : d_ptr(new FileItemPrivate)
{
    d_ptr->fileInfo = FileInfo(fileInfo);

    if (!depth || !fileInfo.isDir()) {
        return;
    }
    auto itemList = QDir(fileInfo.absoluteFilePath())
                        .entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
                                       QDir::DirsFirst);
    //qDebug() << fileInfo.absoluteFilePath();
    for (const auto &item : qAsConst(itemList)) {
        appendChild(new FileItem(item, depth));
    }
}

FileItem::~FileItem() {}

QVariant FileItem::data(int column, int role) const
{
    switch (role) {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignVCenter);
    case Qt::CheckStateRole:
        if (column == 0) {
            return d_ptr->fileInfo.checked() ? Qt::Checked : Qt::Unchecked;
        }
        break;
    case Qt::DecorationRole:
        if (column == 1) {
            return QIcon(d_ptr->fileInfo.icon());
        }
        break;
    case Qt::WhatsThisRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
    case Qt::DisplayRole: {
        switch (column) {
        case 1: return d_ptr->fileInfo.fileInfo().fileName();
        case 2: return d_ptr->fileInfo.fileInfo().absoluteFilePath();
        case 3: return d_ptr->fileInfo.modifyTime();
        case 4: return d_ptr->fileInfo.type();
        case 5: return d_ptr->fileInfo.size();
        default: break;
        }
    } break;
    case Qt::UserRole: return QVariant::fromValue(d_ptr->fileInfo);
    default: break;
    }
    return QVariant();
}

bool FileItem::setData(int column, const QVariant &data, int role)
{
    switch (role) {
    case Qt::CheckStateRole:
        switch (column) {
        case 0: d_ptr->fileInfo.setChecked(data.toBool()); return true;
        default: break;
        }
        break;
    default: break;
    }
    return false;
}

Qt::ItemFlags FileItem::flags(int column) const
{
    Qt::ItemFlags flags = TreeItem::flags(column);
    if (column == 0) {
        flags |= Qt::ItemIsUserCheckable;
    } else if (column == 1) {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

FileInfo FileItem::fileInfo() const
{
    return d_ptr->fileInfo;
}
