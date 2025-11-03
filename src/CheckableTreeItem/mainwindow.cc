#include "mainwindow.hpp"
#include "checkabletreeitem.hpp"

#include <QtWidgets>

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
    QLabel *loadingLabel = new QLabel("Scanning file system, please wait...", centralWidget);
    loadingLabel->setAlignment(Qt::AlignCenter);
    mainLayout->insertWidget(2, loadingLabel); // 插入到树视图上方

    // 使用单次定时器延迟构建，确保UI先显示
    QTimer::singleShot(100, [=]() {
        loadingLabel->setText("Building root directory...");

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
        rootPaths.append(qMakePair(QDir::homePath(), "Home Directory"));
        rootPaths.append(qMakePair("/", "Root Directory"));
        rootPaths.append(qMakePair("/etc", "System Configuration"));
        rootPaths.append(qMakePair("/usr", "User Programs"));
#endif

        progressBar->setVisible(true);
        statusLabel->setVisible(true);

        int totalPaths = rootPaths.size();
        int processed = 0;

        // 清空模型
        model->clear();

        // 构建根节点
        CheckableTreeItem *rootItem = new CheckableTreeItem("Local File System");
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
                if (entryName.startsWith(".") || entryName == "System32" || entryName == "Windows"
                    || entryName == "proc" || entryName == "sys" || entryName == "dev") {
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

            statusLabel->setText(QString("Scanning: %1").arg(path));
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
        statusLabel->setText(QString("Loaded %1 directory items").arg(totalItems));
        statusLabel->setVisible(true);

        // 初始计数
        int checkedCount = countCheckedItems(rootItem);
        countLabel->setText(QString("Checked Items: %1").arg(checkedCount));
    });
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("File System Checkbox Tree");
    resize(700, 650);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 添加标题
    QLabel *titleLabel = new QLabel("Local File System Hierarchical Checkboxes", this);
    mainLayout->addWidget(titleLabel);

    // 创建说明标签
    QLabel *descriptionLabel
        = new QLabel("This example uses local real file paths to build a tree structure, "
                     "demonstrating recursive checkbox functionality:\n"
                     "- Maximum depth limited to 5 levels to prevent excessive data\n"
                     "- Supports real directory structure of file system\n"
                     "- Automatically skips directories without access permissions",
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

    QLabel *statusLabel = new QLabel("Building file tree...", this);
    statusLabel->setVisible(false);

    // 创建操作按钮组
    QGroupBox *controlGroup = new QGroupBox("Operation Control", this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);

    QPushButton *selectAllBtn = new QPushButton("Select All", this);
    QPushButton *deselectAllBtn = new QPushButton("Deselect All", this);
    QPushButton *expandAllBtn = new QPushButton("Expand All", this);
    QPushButton *collapseAllBtn = new QPushButton("Collapse All", this);
    QPushButton *refreshBtn = new QPushButton("Refresh", this);

    controlLayout->addWidget(selectAllBtn);
    controlLayout->addWidget(deselectAllBtn);
    controlLayout->addWidget(expandAllBtn);
    controlLayout->addWidget(collapseAllBtn);
    controlLayout->addWidget(refreshBtn);

    // 统计显示
    QLabel *countLabel = new QLabel("Checked Items: 0", this);

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
    auto buildFileSystemTree = [=]() {
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
        countLabel->setText(QString("Checked Items: %1").arg(checkedCount));
    });

    connect(deselectAllBtn, &QPushButton::clicked, [model, countLabel]() {
        for (int i = 0; i < model->rowCount(); i++) {
            QStandardItem *rootItem = model->item(i);
            if (rootItem && rootItem->isCheckable()) {
                rootItem->setData(Qt::Unchecked, Qt::CheckStateRole);
            }
        }
        // 更新计数
        countLabel->setText("Checked Items: 0");
    });

    connect(expandAllBtn, &QPushButton::clicked, [treeView]() { treeView->expandAll(); });

    connect(collapseAllBtn, &QPushButton::clicked, [treeView]() { treeView->collapseAll(); });

    connect(refreshBtn, &QPushButton::clicked, [=]() { buildFileSystemTree(); });

    // 连接模型数据变化信号来更新计数
    connect(model, &QStandardItemModel::itemChanged, [=](QStandardItem *item) {
        Q_UNUSED(item)
        if (model->rowCount() > 0) {
            int checkedCount = countCheckedItems(model->item(0));
            countLabel->setText(QString("Checked Items: %1").arg(checkedCount));
        }
    });
}

MainWindow::~MainWindow() {}
