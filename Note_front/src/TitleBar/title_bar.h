// CustomTitleBar.h
#pragma once

#include <QWidget>

class QLabel;
class QToolButton;

class CustomTitleBar : public QWidget
{
    Q_OBJECT
public:
    // window 是被控制的顶层窗口（QMainWindow / QWidget 都行）
    explicit CustomTitleBar(QWidget* window, QWidget* parent = nullptr);

    void setTitle(const QString& title);
    QString title() const;

    QSize sizeHint() const override;

signals:
    void minimizeRequested();
    void maximizeRequested();
    void restoreRequested();
    void closeRequested();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onClickedMinimize();
    void onClickedMaximizeRestore();
    void onClickedClose();

private:
    void updateMaximizeRestoreIcon();

private:
    QWidget* m_window = nullptr;     // 被控制的窗口
    QLabel* m_titleLabel = nullptr;
    QToolButton* m_btnMin = nullptr;
    QToolButton* m_btnMax = nullptr;
    QToolButton* m_btnClose = nullptr;

    bool   m_leftPressed = false;
    QPoint m_dragStartGlobalPos;
};
