#include "mainwindow.hpp"
#include "buttoniconstatemanager.hpp"

#include <QtWidgets>

class HoverEventFilter : public QObject
{
public:
    HoverEventFilter(QWidget *widget, const QString &name, QLabel *statusLabel)
        : QObject(widget)
        , m_name(name)
        , m_statusLabel(statusLabel)
    {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Enter) {
            m_statusLabel->setText(tr("Hovering over %1").arg(m_name));
        } else if (event->type() == QEvent::Leave) {
            QAbstractButton *btn = qobject_cast<QAbstractButton *>(obj);
            if (btn) {
                QString state = btn->isChecked() ? tr("checked") : tr("unchecked");
                m_statusLabel->setText(tr("%1 is %2").arg(m_name).arg(state));
            }
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QString m_name;
    QLabel *m_statusLabel;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建主控件和布局
    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(centralWidget);

    // 创建示例按钮1 - 普通按钮
    auto *button1 = new QPushButton(tr("Normal PushButton"), this);
    button1->setCheckable(true);
    auto *iconManager1 = new ButtonIconStateManager(button1);
    iconManager1->setNormalIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
    iconManager1->setHoverIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
    iconManager1->setPressedIcon(style()->standardIcon(QStyle::SP_MessageBoxCritical));
    iconManager1->setCheckedIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));

    // 创建示例按钮2 - 工具按钮
    auto *button2 = new QToolButton(this);
    button2->setText(tr("Tool Button"));
    button2->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button2->setCheckable(true);
    auto *iconManager2 = new ButtonIconStateManager(button2);
    iconManager2->setNormalIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    iconManager2->setHoverIcon(style()->standardIcon(QStyle::SP_DesktopIcon));
    iconManager2->setPressedIcon(style()->standardIcon(QStyle::SP_DriveHDIcon));
    iconManager2->setCheckedIcon(style()->standardIcon(QStyle::SP_DriveCDIcon));

    // 创建示例按钮3 - 仅图标的按钮
    auto *button3 = new QPushButton(this);
    button3->setCheckable(true);
    auto *iconManager3 = new ButtonIconStateManager(button3);
    iconManager3->setNormalIcon(style()->standardIcon(QStyle::SP_DialogOkButton));
    iconManager3->setHoverIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    iconManager3->setPressedIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    iconManager3->setCheckedIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));

    // 创建示例按钮4 - 单选按钮
    auto *button4 = new QPushButton(tr("Radio Style Button"), this);
    button4->setCheckable(true);
    button4->setAutoExclusive(true); // 模拟单选按钮行为
    auto *iconManager4 = new ButtonIconStateManager(button4);
    iconManager4->setNormalIcon(style()->standardIcon(QStyle::SP_CommandLink));
    iconManager4->setHoverIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    iconManager4->setPressedIcon(style()->standardIcon(QStyle::SP_MediaStop));
    iconManager4->setCheckedIcon(style()->standardIcon(QStyle::SP_DialogYesButton));

    // 创建示例按钮5 - 另一个单选按钮（与button4组成一组）
    auto *button5 = new QPushButton(tr("Another Radio Button"), this);
    button5->setCheckable(true);
    button5->setAutoExclusive(true);
    auto *iconManager5 = new ButtonIconStateManager(button5);
    iconManager5->setNormalIcon(style()->standardIcon(QStyle::SP_CommandLink));
    iconManager5->setHoverIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    iconManager5->setPressedIcon(style()->standardIcon(QStyle::SP_MediaPause));
    iconManager5->setCheckedIcon(style()->standardIcon(QStyle::SP_DialogNoButton));

    // 创建控制面板
    auto *controlGroup = new QGroupBox(tr("Controls"), this);
    auto *controlLayout = new QVBoxLayout(controlGroup);

    // 禁用状态控制
    auto *disableCheckbox = new QCheckBox(tr("Disable All Buttons"), this);

    // 重置状态按钮
    auto *resetButton = new QPushButton(tr("Reset All Buttons"), this);

    // 状态显示标签
    auto *statusLabel = new QLabel(tr("Button states will be shown here"), this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFrameStyle(QFrame::Box);

    // 组装控制面板
    controlLayout->addWidget(disableCheckbox);
    controlLayout->addWidget(resetButton);
    controlLayout->addWidget(statusLabel);

    // 按钮说明标签
    auto *descriptionLabel = new QLabel(
        tr("Hover over buttons to see hover icons, click to see pressed/checked icons"), this);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setAlignment(Qt::AlignCenter);

    // 组装主布局
    mainLayout->addWidget(button1);
    mainLayout->addWidget(button2);

    // 创建水平布局用于按钮3和说明
    auto *horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(button3);
    horizontalLayout->addWidget(descriptionLabel);
    mainLayout->addLayout(horizontalLayout);

    mainLayout->addWidget(button4);
    mainLayout->addWidget(button5);
    mainLayout->addWidget(controlGroup);

    // 设置中央控件
    setCentralWidget(centralWidget);
    resize(500, 350);
    setWindowTitle(tr("ButtonIconStateManager Example"));

    // 连接信号和槽

    // 禁用状态控制
    connect(disableCheckbox,
            &QCheckBox::toggled,
            this,
            [button1, button2, button3, button4, button5, statusLabel](bool checked) {
                button1->setDisabled(checked);
                button2->setDisabled(checked);
                button3->setDisabled(checked);
                button4->setDisabled(checked);
                button5->setDisabled(checked);

                if (checked) {
                    statusLabel->setText(tr("All buttons are disabled"));
                } else {
                    statusLabel->setText(tr("All buttons are enabled"));
                }
            });

    // 重置按钮状态
    connect(resetButton,
            &QPushButton::clicked,
            this,
            [button1, button2, button3, button4, button5, statusLabel]() {
                button1->setChecked(false);
                button2->setChecked(false);
                button3->setChecked(false);
                button4->setChecked(false);
                button5->setChecked(false);
                statusLabel->setText(tr("All buttons reset to normal state"));
            });

    // 按钮状态变化信号
    auto updateStatus = [statusLabel](const QString &name, bool checked) {
        QString state = checked ? tr("checked") : tr("unchecked");
        statusLabel->setText(tr("%1 is %2").arg(name).arg(state));
    };

    connect(button1, &QPushButton::toggled, this, [updateStatus](bool checked) {
        updateStatus(tr("Normal PushButton"), checked);
    });

    connect(button2, &QToolButton::toggled, this, [updateStatus](bool checked) {
        updateStatus(tr("Tool Button"), checked);
    });

    connect(button3, &QPushButton::toggled, this, [updateStatus](bool checked) {
        updateStatus(tr("Icon Button"), checked);
    });

    connect(button4, &QPushButton::toggled, this, [updateStatus](bool checked) {
        updateStatus(tr("Radio Style Button 1"), checked);
    });

    connect(button5, &QPushButton::toggled, this, [updateStatus](bool checked) {
        updateStatus(tr("Radio Style Button 2"), checked);
    });

    // 鼠标进入/离开事件的状态显示
    auto installHoverHandler = [statusLabel](QWidget *widget, const QString &name) {
        widget->installEventFilter(new HoverEventFilter(widget, name, statusLabel));
    };

    // 为每个按钮安装悬停事件过滤器
    installHoverHandler(button1, tr("Normal PushButton"));
    installHoverHandler(button2, tr("Tool Button"));
    installHoverHandler(button3, tr("Icon Button"));
    installHoverHandler(button4, tr("Radio Style Button 1"));
    installHoverHandler(button5, tr("Radio Style Button 2"));
}

MainWindow::~MainWindow() {}
