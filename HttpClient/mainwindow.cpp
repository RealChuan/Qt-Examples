#include "mainwindow.h"
#include "httpclient.h"

#include <QtWidgets>

class MainWindow::MainWindowPrivate
{
public:
    explicit MainWindowPrivate(QWidget *parent)
        : q_ptr(parent)
    {
        urlEdit = new QLineEdit(q_ptr);
        urlEdit->setText("http://www.baidu.com");
        urlEdit->setPlaceholderText(QObject::tr("http://url"));
        requestButton = new QPushButton(QObject::tr("GET"), q_ptr);
        textEdit = new QPlainTextEdit(q_ptr);
        httpClient = new HttpClient(q_ptr);
    }

    QWidget *q_ptr;

    QLineEdit *urlEdit;
    QPushButton *requestButton;
    QPlainTextEdit *textEdit; // QPlainTextEdit
    HttpClient *httpClient;

    QElapsedTimer elapsedTimer;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new MainWindowPrivate(this))
{
    setupUI();
    buildConnect();
}

MainWindow::~MainWindow() {}

void MainWindow::onRequest()
{
    QString url = d_ptr->urlEdit->text();
    if (url.isEmpty() || !QUrl(url).isValid()) {
        return;
    }
    d_ptr->requestButton->setEnabled(false);
    d_ptr->elapsedTimer.start();
    d_ptr->httpClient->get(url);
}

void MainWindow::onFinish()
{
    d_ptr->requestButton->setEnabled(true);
    qDebug() << "onFinish" << d_ptr->elapsedTimer.elapsed();
}

void MainWindow::setupUI()
{
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(d_ptr->urlEdit);
    topLayout->addWidget(d_ptr->requestButton);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addLayout(topLayout);
    layout->addWidget(d_ptr->textEdit);
    setCentralWidget(widget);
    resize(640, 480);
}

void MainWindow::buildConnect()
{
    connect(d_ptr->requestButton, &QPushButton::clicked, this, &MainWindow::onRequest);
    connect(d_ptr->httpClient,
            &HttpClient::readyReady,
            d_ptr->textEdit,
            &QPlainTextEdit::appendPlainText);
    connect(d_ptr->httpClient,
            &HttpClient::error,
            d_ptr->textEdit,
            &QPlainTextEdit::appendPlainText);
    connect(d_ptr->httpClient, &HttpClient::finish, this, &MainWindow::onFinish);
}
