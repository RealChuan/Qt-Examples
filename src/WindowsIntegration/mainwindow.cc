#include "mainwindow.hpp"
#include "windowsintegration.hpp"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include <expected>

using namespace Qt::StringLiterals;

namespace {

// 显示"此电脑"集成状态
void showStatus(QLabel *label, bool isRegistered)
{
    if (isRegistered) {
        label->setText(
            u"✅ My Computer integration is ENABLED - Application icon appears in 'This PC' (Windows Registry)"_s);
        label->setStyleSheet(
            u"padding: 10px; background-color: #e8f5e8; border-radius: 5px; color: #2e7d32;"_s);
    } else {
        label->setText(
            u"❌ My Computer integration is DISABLED - No icon in 'This PC' (Windows Registry)"_s);
        label->setStyleSheet(
            u"padding: 10px; background-color: #ffebee; border-radius: 5px; color: #c62828;"_s);
    }
}

// 显示错误信息
void showError(QLabel *label, const QString &error)
{
    label->setText(u"⚠ Error: %1"_s.arg(error));
    label->setStyleSheet(
        u"padding: 10px; background-color: #ffebee; border-radius: 5px; color: #c62828;"_s);
}

} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);

    // 标题
    auto *titleLabel = new QLabel(tr("Windows Integration Manager"), this);
    titleLabel->setStyleSheet(u"font-size: 16px; font-weight: bold; margin: 10px;"_s);
    mainLayout->addWidget(titleLabel);

    // 状态显示
    auto *statusLabel = new QLabel(tr("Checking Windows integration status..."), this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::Box);
    statusLabel->setStyleSheet(u"padding: 10px; background-color: #f0f0f0; border-radius: 5px;"_s);
    mainLayout->addWidget(statusLabel);

    // "此电脑"集成复选框
    auto *myComputerCheckBox = new QCheckBox(tr("Show application icon in 'This PC'"), this);
    mainLayout->addWidget(myComputerCheckBox);

    // 按钮区域
    auto *buttonLayout = new QHBoxLayout();
    auto *refreshButton = new QPushButton(tr("Refresh Status"), this);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    auto *aboutButton = new QPushButton(tr("About"), this);
    buttonLayout->addWidget(aboutButton);

    mainLayout->addLayout(buttonLayout);

    // 提示信息
    auto *infoLabel = new QLabel(
        tr("Note: When enabled, the application icon will appear in Windows 'This PC' "
           "alongside your drives.\n"
           "You may need to restart File Explorer or log out and back in to see changes."),
        this);
    infoLabel->setStyleSheet(u"color: #666; font-size: 12px; margin-top: 20px;"_s);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    setWindowTitle(tr("Windows Integration Example"));
    resize(600, 280);

    static const auto GUID = u"{789183FC-5E6B-4948-8C18-7AF0BBE527A1}"_s;

    // 刷新状态：查询注册表并同步 checkbox 与状态标签
    auto refresh = [statusLabel, myComputerCheckBox]() {
        auto result = Utils::isRegisteredInMyComputer(GUID);
        if (result.has_value()) {
            showStatus(statusLabel, *result);
            myComputerCheckBox->blockSignals(true);
            myComputerCheckBox->setChecked(*result);
            myComputerCheckBox->blockSignals(false);
        } else {
            showError(statusLabel, result.error());
        }
    };
    refresh();

    // checkbox 切换：确认后应用，失败显示错误并回滚
    connect(
        myComputerCheckBox,
        &QCheckBox::toggled,
        this,
        [this, statusLabel, myComputerCheckBox, refresh](bool checked) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                checked ? tr("Enable My Computer Integration")
                        : tr("Disable My Computer Integration"),
                checked
                    ? tr("Are you sure you want to add the application icon to 'This PC'? "
                         "The icon will appear alongside your drives.")
                    : tr("Are you sure you want to remove the application icon from 'This PC'?"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);

            if (reply != QMessageBox::Yes) {
                myComputerCheckBox->blockSignals(true);
                myComputerCheckBox->setChecked(!checked);
                myComputerCheckBox->blockSignals(false);
                return;
            }

            auto result = Utils::setRegisteredInMyComputer(checked, GUID);
            if (result.has_value()) {
                refresh();

                if (checked) {
                    QMessageBox::information(this,
                                             tr("Success"),
                                             tr("Application has been added to 'This PC'!\n\n"
                                                "Note: You may need to:\n"
                                                "• Restart File Explorer, or\n"
                                                "• Log out and back in, or\n"
                                                "• Press F5 to refresh\n"
                                                "to see the changes."));
                } else {
                    QMessageBox::information(
                        this,
                        tr("Success"),
                        tr("Application has been removed from 'This PC'!\n\n"
                           "Note: You may need to refresh File Explorer to see the changes."));
                }
            } else {
                showError(statusLabel, result.error());
                myComputerCheckBox->blockSignals(true);
                myComputerCheckBox->setChecked(!checked);
                myComputerCheckBox->blockSignals(false);
            }
        });

    connect(refreshButton, &QPushButton::clicked, this, [refresh]() { refresh(); });

    connect(aboutButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::about(
            this,
            tr("About"),
            tr("Windows Integration Manager\n\n"
               "Demonstrates how to integrate Qt applications with Windows Shell.\n"
               "This feature adds application icons to Windows 'This PC' for quick access.\n\n"
               "Uses Windows Registry to create shell namespace extensions."));
    });
}
