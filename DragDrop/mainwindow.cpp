#include "mainwindow.h"
#include "draglistwidget.h"
#include "droplistwidget.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setAcceptDrops(true);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;
    QString fileName = urls.first().toLocalFile();
    if (fileName.isEmpty())
        return;
    textEdit->append(tr("File Path: "));
    textEdit->append(fileName);
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        textEdit->append(file.errorString());
        return;
    }
    QString str = file.readAll();
    file.close();
    textEdit->append(tr("Document content: "));
    textEdit->append(str);
}

void MainWindow::setupUI()
{
    DragListWidget *dragListWidget = new DragListWidget(this);
    DropListWidget *dropListWidget = new DropListWidget(this);

    QHBoxLayout *dLayout = new QHBoxLayout;
    dLayout->addWidget(dragListWidget);
    dLayout->addWidget(dropListWidget);
    QGroupBox *groupBox = new QGroupBox(tr("Move to the right"), this);
    groupBox->setLayout(dLayout);

    textEdit = new QTextEdit(this);
    textEdit->setPlaceholderText(tr("Drag text file into text box."));
    textEdit->setAcceptDrops(false);
    QPushButton *button = new QPushButton(tr("Clear"), this);
    QHBoxLayout *textLayout = new QHBoxLayout;
    textLayout->addWidget(textEdit);
    textLayout->addWidget(button);
    QGroupBox *textBox = new QGroupBox(tr("Text reader"), this);
    textBox->setLayout(textLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(groupBox);
    layout->addWidget(textBox);
    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);
    setCentralWidget(frame);

    connect(button, &QPushButton::clicked, textEdit, &QTextEdit::clear);
}

