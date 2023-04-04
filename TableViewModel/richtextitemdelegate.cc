#include "richtextitemdelegate.hpp"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QPainter>
#include <QTextBlock>
#include <QTextDocument>

auto textElide(Qt::TextElideMode textElideMode,
               const QFont &font,
               QTextDocument &doc,
               const QRect &rect) -> bool
{
    QFontMetrics fm(font);
    int dotWidth = fm.horizontalAdvance("...");
    switch (textElideMode) {
    case Qt::ElideRight: {
        qreal cutPos = doc.documentLayout()->hitTest(QPointF(rect.width() - dotWidth,
                                                             fm.height() / 2),
                                                     Qt::ExactHit);
        if (cutPos >= 0) {
            QTextCursor cursor(&doc);
            cursor.setPosition(cutPos);
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            cursor.insertText("...", cursor.block().charFormat());
            return true;
        }
    } break;
    case Qt::ElideLeft: {
        qreal testX = doc.size().width() - rect.width() + dotWidth;
        int cutPos = doc.documentLayout()->hitTest(QPointF(testX, fm.height() / 2), Qt::ExactHit);
        if (cutPos >= 0) {
            QTextCursor cursor(&doc);
            cursor.setPosition(0);
            // 坐标位置为字符位置，所以需要 +1
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, cutPos + 1);
            cursor.insertText("...", cursor.block().charFormat());
            return true;
        }
    } break;
    case Qt::ElideMiddle: {
        qreal testX = (rect.width() - dotWidth) / 2.0;
        int cutLeft = doc.documentLayout()->hitTest(QPoint(testX, fm.height() / 2), Qt::FuzzyHit);
        if (cutLeft >= 0) {
            testX += (doc.size().width() - rect.width()) + dotWidth;
            int cutRight = doc.documentLayout()->hitTest(QPoint(testX, fm.height() / 2),
                                                         Qt::ExactHit);
            if (cutRight >= 0) {
                // 减少缩略的偏移
                int prefer = doc.documentLayout()->hitTest(QPoint(testX, fm.height() / 2),
                                                           Qt::FuzzyHit);
                cutRight = cutRight == prefer ? (cutRight + 1) : prefer;
                QTextCursor cursor(&doc);
                cursor.setPosition(cutLeft);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, cutRight - cutLeft);
                cursor.insertText("...", cursor.block().charFormat());
                return true;
            }
        }
    } break;
    default: break;
    }
    return false;
}

RichTextItemDelegate::RichTextItemDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void RichTextItemDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing
                            | QPainter::SmoothPixmapTransform);

    QFont font = viewOption.widget ? viewOption.widget->font() : QApplication::font();
    QStyle *style = viewOption.widget ? viewOption.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(viewOption.text);
    painter->setFont(font);
    doc.setDefaultFont(font);
    //    QTextOption textOption;
    //    textOption.setAlignment(Qt::AlignCenter);
    //    doc.setDefaultTextOption(textOption);

    /// Painting item without text
    viewOption.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    //    if (optionV4.state & QStyle::State_Selected)
    //        ctx.palette.setColor(QPalette::Text,
    //                             optionV4.palette.color(QPalette::Active, QPalette::HighlightedText));

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText,
                                           &viewOption,
                                           viewOption.widget);
    painter->save();
    //painter->translate(textRect.topLeft());
    qreal x = textRect.topLeft().x();
    qreal fontSize = font.pixelSize() == -1
                         ? (font.pointSize() == -1 ? font.pointSizeF() : font.pointSize())
                         : font.pixelSize();
    qreal y = (textRect.topLeft().y() + textRect.bottomLeft().y()) / 2.0 - fontSize;
    painter->translate(x, y);
    QRect clipRect = textRect.translated(-textRect.topLeft());
    painter->setClipRect(clipRect);
    ctx.clip = clipRect;

    bool elide = textElide(textElideMode, font, doc, clipRect);
    if (!elide) { // 居中
        qreal w = QFontMetrics(font).horizontalAdvance(doc.toPlainText());
        qreal x = (textRect.width() - w) / 2.0;
        //qDebug() << w << x << textRect.width();
        painter->translate(x, 0);
    }
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

auto RichTextItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const -> QSize
{
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());
    return {static_cast<int>(doc.idealWidth()), static_cast<int>(doc.size().height())};
}
