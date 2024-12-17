#include "loadingindicator.hpp"

#include <QtWidgets>

class LoadingIndicator::LoadingIndicatorPrivate
{
public:
    explicit LoadingIndicatorPrivate(LoadingIndicator *q)
        : q_ptr(q)
        , movie(new QMovie(":/gif/resource/loading.gif", {}, q_ptr))
    {
        movie->setCacheMode(QMovie::CacheAll);
    }

    void setupUI()
    {
        auto *label = new QLabel(q_ptr);
        label->setFixedSize(50, 50);
        label->setScaledContents(true);
        label->setMovie(movie);
        movie->setScaledSize(label->size());

        auto *layout = new QHBoxLayout(q_ptr);
        layout->addWidget(label, 0, Qt::AlignCenter);
    }

    LoadingIndicator *q_ptr;

    QMovie *movie;
};

LoadingIndicator::LoadingIndicator(QWidget *parent)
    : QWidget{parent}
    , d_ptr(new LoadingIndicatorPrivate(this))
{
    d_ptr->setupUI();
    QMetaObject::invokeMethod(this, [this] { showLoading(); }, Qt::QueuedConnection);
}

LoadingIndicator::~LoadingIndicator() {}

void LoadingIndicator::showLoading(QWidget *parent)
{
    if (parent == nullptr) {
        parent = parentWidget();
    }
    if (parent == nullptr) {
        parent = qApp->activeWindow();
    }
    setParent(parent);
    resize(parent->size());

    show();
    raise();
    d_ptr->movie->start();
}

void LoadingIndicator::hideLoading()
{
    d_ptr->movie->stop();
    hide();
}
