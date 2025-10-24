#pragma once

#include <QWidget>

class LoadingIndicator : public QWidget
{
    Q_OBJECT
public:
    explicit LoadingIndicator(QWidget *parent = nullptr);
    ~LoadingIndicator();

    void showLoading(QWidget *parent = nullptr);
    void hideLoading();

private:
    class LoadingIndicatorPrivate;
    QScopedPointer<LoadingIndicatorPrivate> d_ptr;
};
