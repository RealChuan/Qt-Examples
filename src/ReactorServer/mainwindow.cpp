#include "mainwindow.h"
#include "accepter.h"

#include <QNetworkInterface>
#include <QtWidgets>

class MainWindowPrivate
{
public:
    explicit MainWindowPrivate(QMainWindow* owner) : owner(owner){
        ipBox = new QComboBox(owner);
        portEdit = new QLineEdit(owner);
        threadSpinBox = new QSpinBox(owner);
        threadSpinBox->setRange(0, 2147483647);
        threadSpinBox->setValue(QThreadPool::globalInstance()->maxThreadCount() * 2);
        listenBtn = new QPushButton(QObject::tr("Listen"), owner);
        listenBtn->setCheckable(true);
        messageEdit = new QTextEdit(owner);
        messageEdit->document()->setMaximumBlockCount(1000);    //1000 max
        currentConnections = new QLabel("0", owner);
        historyMaxConnections = new QLabel("0", owner);
    }
    QMainWindow *owner;
    QComboBox *ipBox;
    QLineEdit *portEdit;
    QSpinBox *threadSpinBox;
    QPushButton *listenBtn;
    QTextEdit *messageEdit;
    QLabel *currentConnections;
    QLabel *historyMaxConnections;
    Accepter *accepter;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new MainWindowPrivate(this))
{
    setupUI();
    initParam();
    buildConnect();
    qDebug() << "MainWindows: " << QThread::currentThreadId();
}

MainWindow::~MainWindow()
{
    delete d_ptr;
}

void MainWindow::onListen(bool state)
{
    if(state){
        d_ptr->listenBtn->setChecked(false);
        QString port = d_ptr->portEdit->text();
        if(port.isEmpty()){
            QMessageBox::warning(this, tr("Warning!"),
                                 tr("Port is empty!"), QMessageBox::Ok);
            return;
        }
        int num = d_ptr->threadSpinBox->value();
        if(num == 0)
            d_ptr->messageEdit->append(tr("Single thread mode reactor!"));
        else
            d_ptr->messageEdit->append(tr("Multithreaded (master-slave) mode reactor!"));
        d_ptr->accepter = new Accepter(quint16(port.toUInt()), num, this);
        connect(d_ptr->accepter, &Accepter::message, d_ptr->messageEdit, &QTextEdit::append, Qt::UniqueConnection);
        connect(d_ptr->accepter, &Accepter::maxCount, this, &MainWindow::onMaxCount, Qt::UniqueConnection);
        connect(d_ptr->accepter, &Accepter::clientCount, this, &MainWindow::onCount, Qt::UniqueConnection);
        d_ptr->accepter->start();
        bool ok = d_ptr->accepter->isRunning();
        d_ptr->ipBox->setEnabled(!ok);
        d_ptr->portEdit->setEnabled(!ok);
        d_ptr->threadSpinBox->setEnabled(!ok);
        d_ptr->listenBtn->setChecked(ok);
        QString text = ok? tr("Disconnect") : tr("Listen");
        d_ptr->listenBtn->setText(text);
    }
    else if(d_ptr->accepter->isRunning()){
        delete d_ptr->accepter;
        d_ptr->accepter = nullptr;
        d_ptr->ipBox->setEnabled(true);
        d_ptr->portEdit->setEnabled(true);
        d_ptr->threadSpinBox->setEnabled(true);
        d_ptr->listenBtn->setChecked(false);
        d_ptr->listenBtn->setText(tr("Listen"));
    }
}

void MainWindow::onMaxCount(QAtomicInt count)
{
    d_ptr->historyMaxConnections->setNum(count);
}

void MainWindow::onCount(QAtomicInt count)
{
    d_ptr->currentConnections->setNum(count);
}

void MainWindow::setupUI()
{
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(new QLabel(tr("Local IP: "), this));
    topLayout->addWidget(d_ptr->ipBox);
    topLayout->addWidget(new QLabel(tr("Local Port: "), this));
    topLayout->addWidget(d_ptr->portEdit);
    topLayout->addWidget(new QLabel(tr("SubReactor Nums: "), this));
    topLayout->addWidget(d_ptr->threadSpinBox);
    topLayout->addWidget(d_ptr->listenBtn);

    QHBoxLayout *midLayout = new QHBoxLayout;
    midLayout->addWidget(new QLabel(tr("Current Connections: "), this));
    midLayout->addWidget(d_ptr->currentConnections);
    midLayout->addStretch(1);
    midLayout->addWidget(new QLabel(tr("History max Connections: "), this));
    midLayout->addWidget(d_ptr->historyMaxConnections);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(topLayout);
    layout->addLayout(midLayout);
    layout->addWidget(d_ptr->messageEdit);

    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);

    setCentralWidget(frame);
}

void MainWindow::initParam()
{
    QStringList ipListBox;
    QList<QHostAddress> ipList = QNetworkInterface::allAddresses(); //获得IP
    foreach(QHostAddress address, ipList){
        if(address.protocol() == QAbstractSocket::IPv4Protocol){
            ipListBox << address.toString();
        }
    }
    d_ptr->ipBox->clear();
    d_ptr->ipBox->addItems(ipListBox);
    d_ptr->portEdit->setValidator(new QIntValidator(0, 65536, d_ptr->portEdit));
    d_ptr->portEdit->setText("65533");
}

void MainWindow::buildConnect()
{
    connect(d_ptr->listenBtn, &QPushButton::clicked, this, &MainWindow::onListen);
}

