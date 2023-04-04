#pragma once

#include <QtCore>

class DisplayData : public QSharedData
{
public:
    DisplayData() = default;
    DisplayData(const DisplayData &other)
        : QSharedData(other)
        , checked(other.checked)
        , title(other.title)
        , number(other.number)
        , state(other.state)
        , process(other.process)
        , rating(other.rating)
        , richText(other.richText)
    {}
    ~DisplayData() = default;

    bool checked = false;
    QString title = QObject::tr("title");
    qint16 number = QRandomGenerator::global()->bounded(100);
    QString state = QObject::tr("close");
    qint16 process = 50;
    int rating = 3;
    QString richText = QObject::tr("RichText");
};

class DisplayInfo
{
public:
    DisplayInfo()
        : d_ptr(new DisplayData)
    {}

    DisplayInfo(bool checked,
                const QString &title,
                const qint16 number,
                const QString &state,
                qint16 process,
                int rating,
                const QString &richText)
        : d_ptr(new DisplayData)
    {
        setChecked(checked);
        setTitle(title);
        setNumber(number);
        setState(state);
        setProcess(process);
        setRating(rating);
        setRichText(richText);
    }

    DisplayInfo(const DisplayInfo &other)
        : d_ptr(other.d_ptr)
    {}

    void setChecked(bool checked) { d_ptr->checked = checked; }
    [[nodiscard]] auto checked() const -> bool { return d_ptr->checked; }

    void setTitle(const QString &title) { d_ptr->title = title; }
    [[nodiscard]] auto title() const -> QString { return d_ptr->title; }

    void setNumber(const qint16 number) { d_ptr->number = number; }
    [[nodiscard]] auto number() const -> qint16 { return d_ptr->number; }

    void setState(const QString &state) { d_ptr->state = state; }
    [[nodiscard]] auto state() const -> QString { return d_ptr->state; }

    void setProcess(qint16 process) { d_ptr->process = process; }
    [[nodiscard]] auto process() const -> qint16 { return d_ptr->process; }

    void setRating(int rating) { d_ptr->rating = rating; }
    [[nodiscard]] auto rating() const -> int { return d_ptr->rating; }

    void setRichText(const QString &richText) { d_ptr->richText = richText; }
    [[nodiscard]] auto richText() const -> QString { return d_ptr->richText; }

private:
    QExplicitlySharedDataPointer<DisplayData> d_ptr;
};

Q_DECLARE_METATYPE(DisplayInfo)
