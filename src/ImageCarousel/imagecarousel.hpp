#pragma once

#include <QGraphicsView>

class ImageCarousel : public QGraphicsView
{
public:
    explicit ImageCarousel(QWidget *parent = nullptr);
    ~ImageCarousel();

public slots:
    void onPrevious();
    void onNext();

private slots:
    void onClicked();

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    class ImageViewerPrivate;
    QScopedPointer<ImageViewerPrivate> d_ptr;
};
