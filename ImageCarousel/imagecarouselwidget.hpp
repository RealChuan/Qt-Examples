#ifndef IMAGECAROUSELWIDGET_HPP
#define IMAGECAROUSELWIDGET_HPP

#include <QtCore/qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif

class ImageCarouselWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageCarouselWidget(QWidget *parent = nullptr);
    ~ImageCarouselWidget() override;

    void addImage(const QString &filename);
    void addImages(const QStringList &filenames);
    void clearImages();

    void start();
    void stop();

    void setInterval(int msec);
    [[nodiscard]] auto interval() const -> int;

private slots:
    void onImageChanged(int index);

private:
    void buildConnect();

    class ImageCarouselWidgetPrivate;
    QScopedPointer<ImageCarouselWidgetPrivate> d_ptr;
};

#endif // IMAGECAROUSELWIDGET_HPP
