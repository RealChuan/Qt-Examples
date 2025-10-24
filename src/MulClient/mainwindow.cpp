#include "mainwindow.h"
#include "tcpclientthread.h"

#include <QtWidgets>

class MainWindowPrivate
{
public:
    explicit MainWindowPrivate(QWidget *owner)
        : owner(owner)
    {
        ipEdit = new QLineEdit(owner);
        ipEdit->setText("127.0.0.1");
        QRegularExpression regExp(
            "^((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)$");
        QRegularExpressionValidator *validator = new QRegularExpressionValidator(regExp, ipEdit);
        ipEdit->setValidator(validator);
        portSpinBox = new QSpinBox(owner);
        portSpinBox->setRange(1, 65536);
        portSpinBox->setValue(65533);
        numSpinBox = new QSpinBox(owner);
        numSpinBox->setRange(1, 2147483647);
        numSpinBox->setValue(QThreadPool::globalInstance()->maxThreadCount() * 2);
        timeSpinBox = new QSpinBox(owner);
        timeSpinBox->setSuffix(" ms");
        timeSpinBox->setRange(1, 1000000);
        timeSpinBox->setSingleStep(50);
        timeSpinBox->setValue(1000);
        connectBtn = new QPushButton(QObject::tr("Connect"), owner);
        connectBtn->setCheckable(true);
        messageEdit = new QTextEdit(owner);
        messageEdit->document()->setMaximumBlockCount(1000);
        sendTime = new QTimer(owner);
    }
    QWidget *owner;
    QLineEdit *ipEdit;
    QSpinBox *portSpinBox;
    QSpinBox *numSpinBox;
    QSpinBox *timeSpinBox;
    QPushButton *connectBtn;
    QTextEdit *messageEdit;

    QMap<int, TcpClientThread *> clientMap;
    QTimer *sendTime;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new MainWindowPrivate(this))
{
    setupUI();
    buildConnect();
}

MainWindow::~MainWindow()
{
    clearAll();
    delete d_ptr;
}

void MainWindow::onConnect(bool state)
{
    if (state) {
        d_ptr->connectBtn->setChecked(false);
        QString ip = d_ptr->ipEdit->text();
        if (ip.isEmpty()) {
            warningBox(tr("Please fill in the correct IP address!"), d_ptr->ipEdit);
            return;
        }
        int port = d_ptr->portSpinBox->value();
        if (port <= 0 || port > 65536) {
            warningBox(tr("Please fill in the correct port address!"), d_ptr->portSpinBox);
            return;
        }
        int num = d_ptr->numSpinBox->value();
        if (num <= 0) {
            warningBox(tr("Please fill in the correct number of clients (sub threads)!"),
                       d_ptr->numSpinBox);
            return;
        }
        int time = d_ptr->timeSpinBox->value();
        if (time <= 0) {
            warningBox(tr("Please enter the correct timeout!"), d_ptr->numSpinBox);
            return;
        }
        clearAll();
        d_ptr->sendTime->setInterval(time);
        for (int i = 0; i < num; i++) {
            TcpClientThread *thread = new TcpClientThread(ip, quint16(port), i, this);
            d_ptr->clientMap.insert(i, thread);
            QString str = QString::number(i + 1) + tr(" Clients (threads) start running");
            d_ptr->messageEdit->append(str);
            connect(thread, &TcpClientThread::quitThread, this, &MainWindow::removeOne);
            thread->start();
        }
        d_ptr->sendTime->start();
        changeControlState(false);
        d_ptr->connectBtn->setText(tr("Connected"));
        d_ptr->connectBtn->setChecked(true);
    } else {
        d_ptr->sendTime->stop();
        clearAll();
        changeControlState(true);
        d_ptr->connectBtn->setText(tr("Connect"));
        d_ptr->connectBtn->setChecked(false);
    }
}

void MainWindow::onWrite()
{
    QString str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QByteArray buf = str.toLatin1() + " Hello World!";
    QMap<int, TcpClientThread *>::iterator it;
    for (it = d_ptr->clientMap.begin(); it != d_ptr->clientMap.end(); it++) {
        if (it.value()->isRunning())
            emit it.value()->writeToServer(buf);
    }
}

void MainWindow::removeOne(const int index)
{
    delete d_ptr->clientMap.take(index);
    if (d_ptr->clientMap.isEmpty()) {
        d_ptr->sendTime->stop();
        changeControlState(true);
        d_ptr->connectBtn->setText(tr("Connect"));
        d_ptr->connectBtn->setChecked(false);
    }
}

void MainWindow::warningBox(const QString &str, QWidget *w)
{
    QMessageBox::warning(this, tr("Warining"), str, QMessageBox::Ok);
    w->setFocus();
}

void MainWindow::buildConnect()
{
    connect(d_ptr->connectBtn, &QPushButton::clicked, this, &MainWindow::onConnect);
    connect(d_ptr->sendTime, &QTimer::timeout, this, &MainWindow::onWrite);
}

void MainWindow::setupUI()
{
    QGridLayout *gLayout = new QGridLayout;
    gLayout->addWidget(new QLabel(tr("Sever IP: "), this), 0, 0);
    gLayout->addWidget(d_ptr->ipEdit, 0, 1);
    gLayout->addWidget(new QLabel(tr("Server Port: "), this), 0, 2);
    gLayout->addWidget(d_ptr->portSpinBox, 0, 3);
    gLayout->addWidget(new QLabel(tr("Client(Threaths) Numbers: "), this), 1, 0);
    gLayout->addWidget(d_ptr->numSpinBox, 1, 1);
    gLayout->addWidget(new QLabel(tr("Timeout: "), this), 1, 2);
    gLayout->addWidget(d_ptr->timeSpinBox, 1, 3);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addLayout(gLayout);
    btnLayout->addWidget(d_ptr->connectBtn);

    QFrame *frame = new QFrame(this);
    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->addLayout(btnLayout);
    layout->addWidget(d_ptr->messageEdit);

    setCentralWidget(frame);
}

void MainWindow::changeControlState(bool state)
{
    d_ptr->ipEdit->setEnabled(state);
    d_ptr->portSpinBox->setEnabled(state);
    d_ptr->numSpinBox->setEnabled(state);
    d_ptr->timeSpinBox->setEnabled(state);
}

void MainWindow::clearAll()
{
    if (d_ptr->clientMap.isEmpty())
        return;
    qDeleteAll(d_ptr->clientMap);
    d_ptr->clientMap.clear();
}
