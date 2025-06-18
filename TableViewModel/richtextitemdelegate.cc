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
        auto cutPos = doc.documentLayout()->hitTest(QPointF(rect.width() - dotWidth,
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
        auto testX = doc.size().width() - rect.width() + dotWidth;
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
        auto testX = (rect.width() - dotWidth) / 2.0;
        auto cutLeft = doc.documentLayout()->hitTest(QPoint(testX, fm.height() / 2), Qt::FuzzyHit);
        if (cutLeft >= 0) {
            testX += (doc.size().width() - rect.width()) + dotWidth;
            auto cutRight = doc.documentLayout()->hitTest(QPoint(testX, fm.height() / 2),
                                                          Qt::ExactHit);
            if (cutRight >= 0) {
                // 减少缩略的偏移
                auto prefer = doc.documentLayout()->hitTest(QPoint(testX, fm.height() / 2),
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
    auto copyOption = option;
    initStyleOption(&copyOption, index);

    const auto *widget = option.widget;
    auto font = qApp->font();
    auto *style = qApp->style();
    if (widget) {
        font = widget->font();
        style = widget->style();
    }

    QTextDocument doc;
    doc.setHtml(copyOption.text);
    doc.setDefaultFont(font);

    /// Painting item without text
    copyOption.text = QString();

    painter->save();
    painter->setFont(font);
    style->drawControl(QStyle::CE_ItemViewItem, &copyOption, painter, widget);
    painter->restore();

    painter->save();
    QAbstractTextDocumentLayout::PaintContext ctx;
    auto textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &copyOption, widget);

    auto x = textRect.topLeft().x();
    auto fontSize = font.pixelSize() == -1
                        ? (font.pointSize() == -1 ? font.pointSizeF() : font.pointSize())
                        : font.pixelSize();
    auto y = (textRect.topLeft().y() + textRect.bottomLeft().y()) / 2.0 - fontSize;
    painter->translate(x, y);
    auto clipRect = textRect.translated(-textRect.topLeft());
    painter->setClipRect(clipRect);
    ctx.clip = clipRect;

    bool elide = textElide(copyOption.textElideMode, font, doc, clipRect);
    if (!elide) { // 居中
        auto w = QFontMetrics(font).horizontalAdvance(doc.toPlainText());
        auto x = (textRect.width() - w) / 2.0;
        painter->translate(x, 0);
    }
    painter->setFont(font);
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

auto RichTextItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const -> QSize
{
    auto copyOption = option;
    initStyleOption(&copyOption, index);

    QTextDocument doc;
    doc.setHtml(copyOption.text);
    doc.setTextWidth(copyOption.rect.width());
    return {static_cast<int>(doc.idealWidth()), static_cast<int>(doc.size().height())};
}
