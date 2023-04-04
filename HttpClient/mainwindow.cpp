#include "mainwindow.h"
#include "httpclient.h"
//#include "httpclientthread.h"

#include <QtConcurrent>
#include <QtWidgets>

class MainWindowPrivate
{
public:
    explicit MainWindowPrivate(QWidget *parent)
        : owner(parent)
    {
        urlEdit = new QLineEdit(owner);
        urlEdit->setText("http://www.baidu.com");
        urlEdit->setPlaceholderText(QObject::tr("http://url"));
        requestButton = new QPushButton(QObject::tr("Request"), owner);
        textEdit = new QTextEdit(owner);
        //httpClientThread = new HttpClientThread(owner);
        httpClient = new HttpClient(owner);
    }
    QWidget *owner;
    QLineEdit *urlEdit;
    QPushButton *requestButton;
    QTextEdit *textEdit; // QPlainTextEdit
    //HttpClientThread *httpClientThread;
    HttpClient *httpClient;
    QByteArray bytes;
    QTimer timer;

    QElapsedTimer elapsedTimer;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d(new MainWindowPrivate(this))
{
    setupUI();
    buildConnect();
}

MainWindow::~MainWindow() {}

void MainWindow::onRequest()
{
    QString url = d->urlEdit->text();
    if (url.isEmpty() || !QUrl(url).isValid())
        return;
    d->requestButton->setEnabled(false);
    d->elapsedTimer.start();
    d->timer.start(10);
    // emit d->httpClientThread->get(url);
    d->httpClient->get(url);
}

void MainWindow::onFinish()
{
    d->requestButton->setEnabled(true);
    qDebug() << "onFinish" << d->elapsedTimer.elapsed();
}

void MainWindow::onAppendBytes(const QByteArray &bytes)
{
    d->bytes.append(bytes);
}

void MainWindow::onAppendText()
{
    if (d->bytes.isEmpty())
        return;

    if (d->bytes.size() > 2000) {
        d->textEdit->append(d->bytes.left(2000));
        d->bytes.remove(0, 2000);
    } else {
        d->textEdit->append(d->bytes);
        d->bytes.clear();
    }

    if (d->bytes.isEmpty() && !d->requestButton->isChecked())
        d->timer.stop();
}

void MainWindow::setupUI()
{
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(d->urlEdit);
    topLayout->addWidget(d->requestButton);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addLayout(topLayout);
    layout->addWidget(d->textEdit);
    setCentralWidget(widget);
    resize(640, 480);
}

void MainWindow::buildConnect()
{
    connect(&d->timer, &QTimer::timeout, this, &MainWindow::onAppendText);

    connect(d->requestButton, &QPushButton::clicked, this, &MainWindow::onRequest);
    connect(d->httpClient, &HttpClient::readyReady, this, &MainWindow::onAppendBytes);
    //connect(d->httpClient, &HttpClient::readyReady, d->textEdit, &QPlainTextEdit::appendPlainText);
    connect(d->httpClient, &HttpClient::error, d->textEdit, &QTextEdit::append);
    connect(d->httpClient, &HttpClient::finish, this, &MainWindow::onFinish);
    //    connect(d->httpClientThread, &HttpClientThread::readyReady, d->textEdit, &QPlainTextEdit::appendPlainText);
    //    connect(d->httpClientThread, &HttpClientThread::error, d->textEdit, &QPlainTextEdit::appendPlainText);
    //    connect(d->httpClientThread, &HttpClientThread::finish, this, &MainWindow::onFinish);
}
