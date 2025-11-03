#include "mainwindow.h"
#include "autostartmanager.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Title
    QLabel *titleLabel = new QLabel("Auto-start Manager", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin: 10px;");
    mainLayout->addWidget(titleLabel);

    // Status display
    QLabel *statusLabel = new QLabel("Checking auto-start status...", this);
    statusLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border-radius: 5px;");
    mainLayout->addWidget(statusLabel);

    // Auto-start checkbox
    QCheckBox *autoRunCheckBox = new QCheckBox("Start automatically at system boot", this);
    mainLayout->addWidget(autoRunCheckBox);

    // Button area
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *refreshButton = new QPushButton("Refresh Status", this);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    QPushButton *aboutButton = new QPushButton("About", this);
    buttonLayout->addWidget(aboutButton);

    mainLayout->addLayout(buttonLayout);

    // Information
    QLabel *infoLabel
        = new QLabel("Note: When auto-start is enabled, the application will automatically launch "
                     "when you log in to the system.\n"
                     "This feature is implemented differently across operating systems.",
                     this);
    infoLabel->setStyleSheet("color: #666; font-size: 12px; margin-top: 20px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    // Set window properties
    setWindowTitle("Auto-start Example");
    resize(550, 250);

    // Helper lambda function to update status label
    auto updateStatusLabel = [](QLabel *label, bool isAutoRun) {
        if (isAutoRun) {
            label->setText(
                "✅ Auto-start is ENABLED - Application will start automatically at login");
            label->setStyleSheet(
                "padding: 10px; background-color: #e8f5e8; border-radius: 5px; color: #2e7d32;");
        } else {
            label->setText("❌ Auto-start is DISABLED - Application will not start automatically");
            label->setStyleSheet(
                "padding: 10px; background-color: #ffebee; border-radius: 5px; color: #c62828;");
        }

        // Add platform information
        QString platformInfo;
#ifdef Q_OS_WINDOWS
        platformInfo = " (Windows Registry)";
#elif defined(Q_OS_MACOS)
        platformInfo = " (macOS LaunchAgents)";
#elif defined(Q_OS_LINUX)
        platformInfo = " (Linux Auto-start)";
#endif

        label->setText(label->text() + platformInfo);
    };

    // Initialize checkbox state
    bool isAutoRun = AutoStartManager::isAutoRunStart();
    autoRunCheckBox->setChecked(isAutoRun);
    updateStatusLabel(statusLabel, isAutoRun);

    // Connect signals and slots using lambdas
    connect(autoRunCheckBox,
            &QCheckBox::toggled,
            this,
            [this, statusLabel, updateStatusLabel](bool checked) {
                // Show confirmation dialog
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    checked ? "Enable Auto-start" : "Disable Auto-start",
                    checked ? "Are you sure you want to enable auto-start? The "
                              "application will run automatically when you log in."
                            : "Are you sure you want to disable auto-start? The "
                              "application will no longer run automatically.",
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No);

                if (reply == QMessageBox::Yes) {
                    // Set auto-start state
                    AutoStartManager::setAutoRunStart(checked);

                    // Update status display
                    updateStatusLabel(statusLabel, checked);

                    // Show operation result
                    if (checked) {
                        QMessageBox::information(this, "Success", "Auto-start has been enabled!");
                    } else {
                        QMessageBox::information(this, "Success", "Auto-start has been disabled!");
                    }
                } else {
                    // User canceled, revert checkbox state
                    QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());
                    if (checkBox) {
                        checkBox->blockSignals(true);
                        checkBox->setChecked(!checked);
                        checkBox->blockSignals(false);
                    }
                }
            });

    connect(refreshButton,
            &QPushButton::clicked,
            this,
            [this, statusLabel, autoRunCheckBox, updateStatusLabel]() {
                bool isAutoRun = AutoStartManager::isAutoRunStart();
                autoRunCheckBox->blockSignals(true);
                autoRunCheckBox->setChecked(isAutoRun);
                autoRunCheckBox->blockSignals(false);
                updateStatusLabel(statusLabel, isAutoRun);
                QMessageBox::information(this, "Refreshed", "Auto-start status has been refreshed!");
            });

    connect(aboutButton, &QPushButton::clicked, this, []() {
        QMessageBox::about(nullptr,
                           "About",
                           "Auto-start Manager Example\n\n"
                           "Demonstrates how to implement cross-platform auto-start management in "
                           "a Qt application.\n"
                           "Supports Windows, macOS and Linux systems.");
    });

    // Initialize the status label
    updateStatusLabel(statusLabel, isAutoRun);
}

MainWindow::~MainWindow() {}
