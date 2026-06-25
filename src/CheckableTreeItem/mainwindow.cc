#include "mainwindow.hpp"
#include "checkabletreeitem.hpp"

#include <QtWidgets>

using namespace Qt::StringLiterals;

namespace {

void buildFileSystemTreeImpl(QStandardItemModel *model,
                             QTreeView *treeView,
                             QProgressBar *progressBar,
                             QLabel *statusLabel,
                             QLabel *countLabel,
                             QVBoxLayout *mainLayout,
                             QWidget *centralWidget,
                             std::function<int(QStandardItem *)> countTreeItems,
                             std::function<int(QStandardItem *)> countCheckedItems)
{
    // 显示加载状态
    QLabel *loadingLabel = new QLabel(u"Scanning file system, please wait..."_s, centralWidget);
    loadingLabel->setAlignment(Qt::AlignCenter);
    mainLayout->insertWidget(2, loadingLabel); // 插入到树视图上方

    // 使用单次定时器延迟构建，确保UI先显示
    QMetaObject::invokeMethod(
        qApp,
        [model,
         treeView,
         progressBar,
         statusLabel,
         countLabel,
         loadingLabel,
         countTreeItems,
         countCheckedItems]() {
            loadingLabel->setText(u"Building root directory..."_s);

            // 获取根目录路径（跨平台）
            QList<QPair<QString, QString>> rootPaths;

#ifdef Q_OS_WIN
            // Windows系统：获取所有驱动器
            QFileInfoList drives = QDir::drives();
            for (const QFileInfo &drive : drives) {
                rootPaths.append(qMakePair(drive.absoluteFilePath(), drive.absoluteFilePath()));
            }
#else
            // Unix/Linux/Mac系统：使用用户主目录和常见系统目录
            rootPaths.append(qMakePair(QDir::homePath(), u"Home Directory"_s));
            rootPaths.append(qMakePair(u"/"_s, u"Root Directory"_s));
            rootPaths.append(qMakePair(u"/etc"_s, u"System Configuration"_s));
            rootPaths.append(qMakePair(u"/usr"_s, u"User Programs"_s));
#endif

            progressBar->setVisible(true);
            statusLabel->setVisible(true);

            int totalPaths = rootPaths.size();
            int processed = 0;

            // 清空模型
            model->clear();

            // 构建根节点
            CheckableTreeItem *rootItem = new CheckableTreeItem(u"Local File System"_s);
            model->appendRow(rootItem);

            // 递归构建文件树函数
            std::function<void(CheckableTreeItem *, const QString &, int)> buildTree;
            buildTree = [&](CheckableTreeItem *parentItem, const QString &path, int depth) {
                if (depth >= 5) { // 限制最大层级为5层
                    return;
                }

                QDir dir(path);
                if (!dir.exists()) {
                    return;
                }

                // 设置过滤器：只显示目录，不显示隐藏文件和系统文件
                dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
                dir.setSorting(QDir::Name | QDir::IgnoreCase);

                QFileInfoList entries;
                try {
                    entries = dir.entryInfoList();
                } catch (...) {
                    // 跳过无权限访问的目录
                    return;
                }

                // 限制每个目录下最多显示10个子项，避免过多
                int maxChildren = 10;
                int count = 0;

                for (const QFileInfo &entry : entries) {
                    if (count >= maxChildren)
                        break;

                    QString entryName = entry.fileName();
                    QString fullPath = entry.absoluteFilePath();

                    // 跳过一些系统目录和无关目录
                    if (entryName.startsWith(u'.') || entryName == u"System32"_s
                        || entryName == u"Windows"_s || entryName == u"proc"_s
                        || entryName == u"sys"_s || entryName == u"dev"_s) {
                        continue;
                    }

                    CheckableTreeItem *item = new CheckableTreeItem(entryName);
                    item->setToolTip(fullPath);
                    parentItem->appendRow(item);

                    // 递归构建子目录（深度+1）
                    buildTree(item, fullPath, depth + 1);

                    count++;
                }
            };

            // 为每个根路径构建子树
            for (const auto &rootPair : rootPaths) {
                QString path = rootPair.first;
                QString displayName = rootPair.second;

                statusLabel->setText(u"Scanning: %1"_s.arg(path));
                qApp->processEvents(); // 处理事件，更新UI

                CheckableTreeItem *driveItem = new CheckableTreeItem(displayName);
                driveItem->setToolTip(path);
                rootItem->appendRow(driveItem);

                buildTree(driveItem, path, 1);

                processed++;
                progressBar->setValue((processed * 100) / totalPaths);
                qApp->processEvents(); // 处理事件，更新UI
            }

            // 完成构建，隐藏进度条和加载标签
            progressBar->setVisible(false);
            statusLabel->setVisible(false);
            loadingLabel->setVisible(false);
            loadingLabel->deleteLater();

            // 展开根节点
            treeView->expand(rootItem->index());

            // 显示统计信息
            int totalItems = countTreeItems(rootItem);
            statusLabel->setText(u"Loaded %1 directory items"_s.arg(totalItems));
            statusLabel->setVisible(true);

            // 初始计数
            int checkedCount = countCheckedItems(rootItem);
            countLabel->setText(u"Checked Items: %1"_s.arg(checkedCount));
        },
        Qt::QueuedConnection);
}

} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(u"File System Checkbox Tree"_s);
    resize(700, 650);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 添加标题
    QLabel *titleLabel = new QLabel(u"Local File System Hierarchical Checkboxes"_s, this);
    mainLayout->addWidget(titleLabel);

    // 创建说明标签
    QLabel *descriptionLabel
        = new QLabel(u"This example uses local real file paths to build a tree structure, "
                     "demonstrating recursive checkbox functionality:\n"
                     "- Maximum depth limited to 5 levels to prevent excessive data\n"
                     "- Supports real directory structure of file system\n"
                     "- Automatically skips directories without access permissions"_s,
                     this);
    descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(descriptionLabel);

    // 创建树状视图和模型
    QStandardItemModel *model = new QStandardItemModel(this);
    QTreeView *treeView = new QTreeView(this);
    treeView->setModel(model);
    treeView->setHeaderHidden(true);

    // 创建进度条和状态标签
    QProgressBar *progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setVisible(false);

    QLabel *statusLabel = new QLabel(u"Building file tree..."_s, this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::Box);
    statusLabel->setVisible(false);

    // 创建操作按钮组
    QGroupBox *controlGroup = new QGroupBox(u"Operation Control"_s, this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);

    QPushButton *selectAllBtn = new QPushButton(u"Select All"_s, this);
    QPushButton *deselectAllBtn = new QPushButton(u"Deselect All"_s, this);
    QPushButton *expandAllBtn = new QPushButton(u"Expand All"_s, this);
    QPushButton *collapseAllBtn = new QPushButton(u"Collapse All"_s, this);
    QPushButton *refreshBtn = new QPushButton(u"Refresh"_s, this);

    controlLayout->addWidget(selectAllBtn);
    controlLayout->addWidget(deselectAllBtn);
    controlLayout->addWidget(expandAllBtn);
    controlLayout->addWidget(collapseAllBtn);
    controlLayout->addWidget(refreshBtn);

    // 统计显示
    QLabel *countLabel = new QLabel(u"Checked Items: 0"_s, this);

    // 将组件添加到主布局
    mainLayout->addWidget(treeView);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(controlGroup);
    mainLayout->addWidget(countLabel);

    // Helper function: count tree node items
    auto countTreeItems = [](QStandardItem *root) -> int {
        std::function<int(QStandardItem *)> countFunc;
        countFunc = [&](QStandardItem *item) -> int {
            int count = 1; // 当前节点
            for (int i = 0; i < item->rowCount(); i++) {
                count += countFunc(item->child(i));
            }
            return count;
        };
        return countFunc(root);
    };

    // 辅助函数：统计选中节点数量
    auto countCheckedItems = [](QStandardItem *root) -> int {
        std::function<int(QStandardItem *)> countFunc;
        countFunc = [&](QStandardItem *item) -> int {
            int count = 0;
            if (item->isCheckable() && item->checkState() == Qt::Checked) {
                count = 1;
            }
            for (int i = 0; i < item->rowCount(); i++) {
                count += countFunc(item->child(i));
            }
            return count;
        };
        return countFunc(root);
    };

    // 构建文件系统树的函数
    auto buildFileSystemTree = [model,
                                treeView,
                                progressBar,
                                statusLabel,
                                countLabel,
                                mainLayout,
                                centralWidget,
                                countTreeItems,
                                countCheckedItems]() {
        return buildFileSystemTreeImpl(model,
                                       treeView,
                                       progressBar,
                                       statusLabel,
                                       countLabel,
                                       mainLayout,
                                       centralWidget,
                                       countTreeItems,
                                       countCheckedItems);
    };

    // 初始构建文件系统树
    buildFileSystemTree();

    // 连接按钮信号
    connect(selectAllBtn, &QPushButton::clicked, [model, countLabel, countCheckedItems]() {
        for (int i = 0; i < model->rowCount(); i++) {
            QStandardItem *rootItem = model->item(i);
            if (rootItem && rootItem->isCheckable()) {
                rootItem->setData(Qt::Checked, Qt::CheckStateRole);
            }
        }
        // 更新计数
        int checkedCount = countCheckedItems(model->item(0));
        countLabel->setText(u"Checked Items: %1"_s.arg(checkedCount));
    });

    connect(deselectAllBtn, &QPushButton::clicked, [model, countLabel]() {
        for (int i = 0; i < model->rowCount(); i++) {
            QStandardItem *rootItem = model->item(i);
            if (rootItem && rootItem->isCheckable()) {
                rootItem->setData(Qt::Unchecked, Qt::CheckStateRole);
            }
        }
        // 更新计数
        countLabel->setText(u"Checked Items: 0"_s);
    });

    connect(expandAllBtn, &QPushButton::clicked, this, [treeView]() { treeView->expandAll(); });

    connect(collapseAllBtn, &QPushButton::clicked, this, [treeView]() { treeView->collapseAll(); });

    connect(refreshBtn, &QPushButton::clicked, this, [buildFileSystemTree]() {
        buildFileSystemTree();
    });

    // 连接模型数据变化信号来更新计数
    connect(model,
            &QStandardItemModel::itemChanged,
            this,
            [model, countLabel, countCheckedItems]([[maybe_unused]] QStandardItem *item) {
                if (model->rowCount() > 0) {
                    int checkedCount = countCheckedItems(model->item(0));
                    countLabel->setText(u"Checked Items: %1"_s.arg(checkedCount));
                }
            });
}
