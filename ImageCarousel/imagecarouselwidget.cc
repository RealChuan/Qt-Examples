#include "imagecarouselwidget.hpp"
#include "imagecarousel.hpp"

#include <QtWidgets>

class ImageCarouselWidget::ImageCarouselWidgetPrivate
{
public:
    ImageCarouselWidgetPrivate(ImageCarouselWidget *q)
        : q_ptr{q}
    {
        carousel = new ImageCarousel{q};
        btnGroup = new QButtonGroup{q};
        btnGroup->setExclusive(true);
        layout = new QVBoxLayout{q};

        layout->addWidget(carousel);
    }
    ~ImageCarouselWidgetPrivate() = default;

    void resetBtnWidget()
    {
        auto btns = btnGroup->buttons();
        for (auto btn : btns) {
            btnGroup->removeButton(btn);
            delete btn;
        }
        if (!btnWidget.isNull()) {
            delete btnWidget.data();
        }
        btnWidget = new QWidget(q_ptr);
        layout->addWidget(btnWidget.data());
        auto btnLayout = new QHBoxLayout{btnWidget.data()};
        btnLayout->addStretch();
        auto count = carousel->imageCount();
        for (auto i = 0; i < count; ++i) {
            auto btn = new QRadioButton{btnWidget.data()};
            btnLayout->addWidget(btn);
            btnGroup->addButton(btn, i);
        }
        btnLayout->addStretch();
    }

    ImageCarouselWidget *q_ptr;

    ImageCarousel *carousel;
    QPointer<QWidget> btnWidget;
    QButtonGroup *btnGroup;
    QVBoxLayout *layout;
};

ImageCarouselWidget::ImageCarouselWidget(QWidget *parent)
    : QWidget{parent}
    , d_ptr{new ImageCarouselWidgetPrivate{this}}
{
    buildConnect();
}

ImageCarouselWidget::~ImageCarouselWidget() = default;

void ImageCarouselWidget::addImage(const QString &filename)
{
    d_ptr->carousel->addImage(filename);

    d_ptr->resetBtnWidget();
}

void ImageCarouselWidget::addImages(const QStringList &filenames)
{
    d_ptr->carousel->addImages(filenames);

    d_ptr->resetBtnWidget();
}

void ImageCarouselWidget::clearImages()
{
    d_ptr->carousel->clearImages();

    d_ptr->resetBtnWidget();
}

void ImageCarouselWidget::start()
{
    d_ptr->carousel->start();
}

void ImageCarouselWidget::stop()
{
    d_ptr->carousel->stop();
}

void ImageCarouselWidget::setInterval(int msec)
{
    d_ptr->carousel->setInterval(msec);
}

auto ImageCarouselWidget::interval() const -> int
{
    return d_ptr->carousel->interval();
}

void ImageCarouselWidget::onImageChanged(int index)
{
    auto btn = d_ptr->btnGroup->button(index);
    if (btn) {
        btn->setChecked(true);
    }
}

void ImageCarouselWidget::buildConnect()
{
    connect(d_ptr->carousel,
            &ImageCarousel::imageChanged,
            this,
            &ImageCarouselWidget::onImageChanged);
    connect(d_ptr->btnGroup,
            &QButtonGroup::idClicked,
            d_ptr->carousel,
            &ImageCarousel::onImageChanged);
}
