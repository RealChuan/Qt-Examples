#include "mainwindow.hpp"
#include "windowsintegration.hpp"

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
    QLabel *titleLabel = new QLabel("Windows Integration Manager", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin: 10px;");
    mainLayout->addWidget(titleLabel);

    // Status display
    QLabel *statusLabel = new QLabel("Checking Windows integration status...", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::Box);
    statusLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border-radius: 5px;");
    mainLayout->addWidget(statusLabel);

    // My Computer integration checkbox
    QCheckBox *myComputerCheckBox = new QCheckBox("Show application icon in 'This PC'", this);
    mainLayout->addWidget(myComputerCheckBox);

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
        = new QLabel("Note: When enabled, the application icon will appear in Windows 'This PC' "
                     "alongside your drives.\n"
                     "You may need to restart File Explorer or log out and back in to see changes.",
                     this);
    infoLabel->setStyleSheet("color: #666; font-size: 12px; margin-top: 20px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    // Set window properties
    setWindowTitle("Windows Integration Example");
    resize(600, 280);

    // Helper lambda function to update status label
    auto updateStatusLabel = [](QLabel *label, bool isRegistered) {
        if (isRegistered) {
            label->setText(
                "✅ My Computer integration is ENABLED - Application icon appears in 'This PC'");
            label->setStyleSheet(
                "padding: 10px; background-color: #e8f5e8; border-radius: 5px; color: #2e7d32;");
        } else {
            label->setText("❌ My Computer integration is DISABLED - No icon in 'This PC'");
            label->setStyleSheet(
                "padding: 10px; background-color: #ffebee; border-radius: 5px; color: #c62828;");
        }

        label->setText(label->text() + " (Windows Registry)");
    };

    static const auto GUID = "{789183FC-5E6B-4948-8C18-7AF0BBE527A1}";
    // Initialize checkbox state
    bool isRegistered = Utils::isRegisteredInMyComputer(GUID);
    myComputerCheckBox->setChecked(isRegistered);
    updateStatusLabel(statusLabel, isRegistered);

    // Connect signals and slots using lambdas
    connect(myComputerCheckBox,
            &QCheckBox::toggled,
            this,
            [this, statusLabel, updateStatusLabel](bool checked) {
                // Show confirmation dialog
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    checked ? "Enable My Computer Integration" : "Disable My Computer Integration",
                    checked
                        ? "Are you sure you want to add the application icon to 'This PC'? "
                          "The icon will appear alongside your drives."
                        : "Are you sure you want to remove the application icon from 'This PC'?",
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No);

                if (reply == QMessageBox::Yes) {
                    // Set integration state
                    bool success = Utils::setRegisteredInMyComputer(checked, GUID);

                    // Update status display
                    updateStatusLabel(statusLabel, checked);

                    // Show operation result
                    if (success) {
                        if (checked) {
                            QMessageBox::information(this,
                                                     "Success",
                                                     "Application has been added to 'This PC'!\n\n"
                                                     "Note: You may need to:\n"
                                                     "• Restart File Explorer, or\n"
                                                     "• Log out and back in, or\n"
                                                     "• Press F5 to refresh\n"
                                                     "to see the changes.");
                        } else {
                            QMessageBox::information(
                                this,
                                "Success",
                                "Application has been removed from 'This PC'!\n\n"
                                "Note: You may need to refresh File Explorer to see the changes.");
                        }
                    } else {
                        QMessageBox::warning(this,
                                             "Operation Failed",
                                             "Failed to update Windows integration settings.\n"
                                             "Please try again or check application permissions.");

                        // Revert checkbox on failure
                        QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());
                        if (checkBox) {
                            checkBox->blockSignals(true);
                            checkBox->setChecked(!checked);
                            checkBox->blockSignals(false);
                        }
                        updateStatusLabel(statusLabel, !checked);
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
            [this, statusLabel, myComputerCheckBox, updateStatusLabel]() {
                bool isRegistered = Utils::isRegisteredInMyComputer(GUID);
                myComputerCheckBox->blockSignals(true);
                myComputerCheckBox->setChecked(isRegistered);
                myComputerCheckBox->blockSignals(false);
                updateStatusLabel(statusLabel, isRegistered);
                QMessageBox::information(this,
                                         "Refreshed",
                                         "Windows integration status has been refreshed!");
            });

    connect(aboutButton, &QPushButton::clicked, this, []() {
        QMessageBox::about(
            nullptr,
            "About Windows Integration",
            "Windows Integration Manager\n\n"
            "Demonstrates how to integrate Qt applications with Windows Shell.\n"
            "This feature adds application icons to Windows 'This PC' for quick access.\n\n"
            "Uses Windows Registry to create shell namespace extensions.");
    });
}

MainWindow::~MainWindow() {}
