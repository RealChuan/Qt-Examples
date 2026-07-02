#pragma once

#include <QWidget>

#include <memory>

class NavigationProgressBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(
        QStringList messageList READ messageList WRITE setMessageList NOTIFY messageListChanged)
    Q_PROPERTY(int step READ step WRITE setStep NOTIFY stepChanged)
    Q_PROPERTY(int maxStep READ maxStep NOTIFY maxStepChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY
                   backgroundColorChanged)
    Q_PROPERTY(QColor currentBackgroundColor READ currentBackgroundColor WRITE
                   setCurrentBackgroundColor NOTIFY currentBackgroundColorChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY
                   foregroundColorChanged)
    Q_PROPERTY(QFont textFont READ textFont WRITE setTextFont NOTIFY textFontChanged)
    Q_PROPERTY(QFont dateFont READ dateFont WRITE setDateFont NOTIFY dateFontChanged)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)

public:
    explicit NavigationProgressBar(QWidget *parent = nullptr);
    ~NavigationProgressBar() override;

    NavigationProgressBar(const NavigationProgressBar &) = delete;
    auto operator=(const NavigationProgressBar &) -> NavigationProgressBar & = delete;
    NavigationProgressBar(NavigationProgressBar &&) = delete;
    auto operator=(NavigationProgressBar &&) -> NavigationProgressBar & = delete;

    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

    auto setMessageList(const QStringList &list) -> void;
    [[nodiscard]] auto messageList() const -> QStringList;

    auto setStep(int step) -> void;
    [[nodiscard]] auto step() const -> int;
    [[nodiscard]] auto maxStep() const -> int;

    auto setBackgroundColor(const QColor &color) -> void;
    [[nodiscard]] auto backgroundColor() const -> QColor;

    auto setCurrentBackgroundColor(const QColor &color) -> void;
    [[nodiscard]] auto currentBackgroundColor() const -> QColor;

    auto setForegroundColor(const QColor &color) -> void;
    [[nodiscard]] auto foregroundColor() const -> QColor;

    auto setTextFont(const QFont &font) -> void;
    [[nodiscard]] auto textFont() const -> QFont;

    auto setDateFont(const QFont &font) -> void;
    [[nodiscard]] auto dateFont() const -> QFont;

    auto setSpacing(int spacing) -> void;
    [[nodiscard]] auto spacing() const -> int;

    [[nodiscard]] auto dateAt(int step) const -> QString;
    auto setDateAt(int step, const QString &date) -> void;

public slots:
    auto reset() -> void;
    auto next() -> void;
    auto previous() -> void;

signals:
    void stepChanged(int step);
    void progressCompleted();
    void messageListChanged();
    void maxStepChanged(int maxStep);
    void backgroundColorChanged(const QColor &color);
    void currentBackgroundColorChanged(const QColor &color);
    void foregroundColorChanged(const QColor &color);
    void textFontChanged(const QFont &font);
    void dateFontChanged(const QFont &font);
    void spacingChanged(int spacing);

protected:
    auto paintEvent(QPaintEvent *event) -> void override;
    auto resizeEvent(QResizeEvent *event) -> void override;

private:
    auto drawCompleteProgress(QPainter &painter) -> void;
    auto rebuildDateList() -> void;
    auto invalidateCache() -> void;
    auto rebuildCache() -> void;

    class NavigationProgressBarPrivate;
    std::unique_ptr<NavigationProgressBarPrivate> d_ptr;
};
