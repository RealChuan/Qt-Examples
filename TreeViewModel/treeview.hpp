#ifndef TREEVIEW_HPP
#define TREEVIEW_HPP

#include <QTreeView>

class TreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit TreeView(QWidget *parent = nullptr);

    [[nodiscard]] auto selectedIndexes() const -> QModelIndexList override;

    [[nodiscard]] auto allIndexes() const -> QModelIndexList;

    virtual auto pageSize() -> int;

signals:
    void selectedChanged();
    void fetchMore();

public slots:
    void onCheckAll(bool);
    void onResize();

private slots:
    void onFetchMore(int value);

protected:
    void showEvent(QShowEvent *event) override;

protected:
    int m_pageSize = 20;
    int m_currentMaxCount = 0;

private:
    void setupUI();
    void buildConnect();
};

#endif // TREEVIEW_HPP
