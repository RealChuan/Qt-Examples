#ifndef IMAGECAROUSEL_HPP
#define IMAGECAROUSEL_HPP

// qt image carousel control class

#include <QGraphicsView>

class ImageCarousel : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ImageCarousel(QWidget *parent = nullptr);
    ~ImageCarousel() override;

    auto addImage(const QString &filename) -> bool;
    auto addImages(const QStringList &filenames) -> int;
    void clearImages();

    [[nodiscard]] auto imageCount() const -> int;

    void start();
    void stop();

    void setInterval(int msec);
    [[nodiscard]] auto interval() const -> int;

public slots:
    void onImageChanged(int index);

signals:
    void imageChanged(int index);

private slots:
    void next();

private:
    void resetItem();

    class ImageCarouselPrivate;
    QScopedPointer<ImageCarouselPrivate> d_ptr;
};

#endif // IMAGECAROUSEL_HPP
