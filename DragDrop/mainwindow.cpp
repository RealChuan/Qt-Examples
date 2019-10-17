#include "mainwindow.h"
#include "acceptwidget.h"
#include "menwidget.h"

#include <QFrame>
#include <QLayout>
#include <QGroupBox>
#include <QMimeData>
#include <QDropEvent>
#include <QPushButton>
#include <QDebug>

#pragma execution_character_set("utf-8")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QFrame *frame=new QFrame(this);
    QVBoxLayout *layout=new QVBoxLayout(frame);

    QGroupBox *GBox_1=new QGroupBox("向右复制",frame);
    QHBoxLayout *layout_1=new QHBoxLayout();
    acceptWidget *acc=new acceptWidget(this);
    menWidget *menu=new menWidget(this);
    layout_1->addWidget(menu);
    layout_1->addWidget(acc);
    GBox_1->setLayout(layout_1);

    QGroupBox *GBox_2=new QGroupBox("文本阅读器",frame);
    QHBoxLayout *layout_2=new QHBoxLayout();
    textEdit=new QTextEdit();
    textEdit->setPlaceholderText("把文本文件拖入框内");
    textEdit->setAcceptDrops(false);
    setAcceptDrops(true);
    QPushButton *clearBtn=new QPushButton("清除");
    connect(clearBtn,&QPushButton::clicked,[this]
    {
        textEdit->clear();
    });
    layout_2->addWidget(textEdit);
    layout_2->addWidget(clearBtn);
    GBox_2->setLayout(layout_2);

    layout->addWidget(GBox_1);
    layout->addWidget(GBox_2);

    setCentralWidget(frame);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
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
    textEdit->append("文件路径");
    textEdit->append(fileName);
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        textEdit->append(file.errorString());
        return;
    }
    QString str=file.readAll();
    file.close();
    textEdit->append("文件内容");
    textEdit->append(str);
}

