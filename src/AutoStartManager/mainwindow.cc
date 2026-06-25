#include "mainwindow.hpp"
#include "autostartmanager.hpp"

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

// 返回当前平台的自启动机制名称
QString platformName()
{
#if defined(Q_OS_WINDOWS)
    return u"Windows Registry"_s;
#elif defined(Q_OS_MACOS)
    return u"macOS LaunchAgents"_s;
#elif defined(Q_OS_LINUX)
    return u"Linux Auto-start"_s;
#else
    return u"Unknown"_s;
#endif
}

// 显示自启动状态
void showStatus(QLabel *label, bool enabled)
{
    if (enabled) {
        label->setText(u"✅ Auto-start is ENABLED (%1)"_s.arg(platformName()));
        label->setStyleSheet(
            u"padding: 10px; background-color: #e8f5e8; border-radius: 5px; color: #2e7d32;"_s);
    } else {
        label->setText(u"❌ Auto-start is DISABLED (%1)"_s.arg(platformName()));
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
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Title
    QLabel *titleLabel = new QLabel(u"Auto-start Manager"_s, this);
    titleLabel->setStyleSheet(u"font-size: 16px; font-weight: bold; margin: 10px;"_s);
    mainLayout->addWidget(titleLabel);

    // Status display
    QLabel *statusLabel = new QLabel(u"Checking auto-start status..."_s, this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::Box);
    statusLabel->setStyleSheet(u"padding: 10px; background-color: #f0f0f0; border-radius: 5px;"_s);
    mainLayout->addWidget(statusLabel);

    // Auto-start checkbox
    QCheckBox *autoRunCheckBox = new QCheckBox(u"Start automatically at system boot"_s, this);
    mainLayout->addWidget(autoRunCheckBox);

    // Button area
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *refreshButton = new QPushButton(u"Refresh Status"_s, this);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    QPushButton *aboutButton = new QPushButton(u"About"_s, this);
    buttonLayout->addWidget(aboutButton);

    mainLayout->addLayout(buttonLayout);

    // Information
    QLabel *infoLabel = new QLabel(
        u"Note: When auto-start is enabled, the application will automatically launch "_s
            + u"when you log in to the system.\n"_s
            + u"This feature is implemented differently across operating systems."_s,
        this);
    infoLabel->setStyleSheet(u"color: #666; font-size: 12px; margin-top: 20px;"_s);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    setWindowTitle(u"Auto-start Example"_s);
    resize(550, 250);

    // 刷新状态：查询并同步 checkbox 与状态标签
    auto refresh = [statusLabel, autoRunCheckBox]() {
        auto result = Utils::isAutoRunStart();
        if (result.has_value()) {
            showStatus(statusLabel, *result);
            autoRunCheckBox->blockSignals(true);
            autoRunCheckBox->setChecked(*result);
            autoRunCheckBox->blockSignals(false);
        } else {
            showError(statusLabel, result.error());
        }
    };
    refresh();

    // checkbox 切换：确认后应用，失败显示错误并回滚
    connect(autoRunCheckBox,
            &QCheckBox::toggled,
            this,
            [this, statusLabel, autoRunCheckBox, refresh](bool checked) {
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    checked ? u"Enable Auto-start"_s : u"Disable Auto-start"_s,
                    checked ? u"Are you sure you want to enable auto-start? "_s
                                  + u"The application will run automatically when you log in."_s
                            : u"Are you sure you want to disable auto-start? "_s
                                  + u"The application will no longer run automatically."_s,
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No);

                if (reply != QMessageBox::Yes) {
                    autoRunCheckBox->blockSignals(true);
                    autoRunCheckBox->setChecked(!checked);
                    autoRunCheckBox->blockSignals(false);
                    return;
                }

                auto result = Utils::setAutoRunStart(checked);
                if (result.has_value()) {
                    refresh();
                } else {
                    showError(statusLabel, result.error());
                    autoRunCheckBox->blockSignals(true);
                    autoRunCheckBox->setChecked(!checked);
                    autoRunCheckBox->blockSignals(false);
                }
            });

    connect(refreshButton, &QPushButton::clicked, this, [refresh]() { refresh(); });

    connect(aboutButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::about(
            this,
            u"About"_s,
            u"Auto-start Manager Example\n\n"_s
                + u"Demonstrates how to implement cross-platform auto-start management in a Qt application.\n"_s
                + u"Supports Windows, macOS and Linux systems."_s);
    });
}

MainWindow::~MainWindow() = default;
