// CustomTitleBar.cpp
#include "title_bar.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStyle>
#include <QToolButton>

CustomTitleBar::CustomTitleBar(QWidget* window, QWidget* parent)
    : QWidget(parent),
      m_window(window)
{
    // 让背景可用 QSS 填充
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setText(window ? window->windowTitle() : QStringLiteral("Window"));
    m_titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_btnMin = new QToolButton(this);
    m_btnMax = new QToolButton(this);
    m_btnClose = new QToolButton(this);

    for (QToolButton* btn : {m_btnMin, m_btnMax, m_btnClose}) {
        btn->setCursor(Qt::ArrowCursor);
        btn->setAutoRaise(true);  // 扁平风格
        btn->setFocusPolicy(Qt::NoFocus);
    }

    // 使用系统标准标题栏图标，而不是简单字符
    QStyle* st = style();
    m_btnMin->setIcon(st->standardIcon(QStyle::SP_TitleBarMinButton));
    m_btnMax->setIcon(st->standardIcon(QStyle::SP_TitleBarMaxButton));
    m_btnClose->setIcon(st->standardIcon(QStyle::SP_TitleBarCloseButton));

    // Alt+F4 快捷键
    m_btnClose->setShortcut(QKeySequence(Qt::ALT | Qt::Key_F4));

    connect(m_btnMin, &QToolButton::clicked, this, &CustomTitleBar::onClickedMinimize);
    connect(m_btnMax, &QToolButton::clicked, this, &CustomTitleBar::onClickedMaximizeRestore);
    connect(m_btnClose, &QToolButton::clicked, this, &CustomTitleBar::onClickedClose);

    auto* hl = new QHBoxLayout(this);
    hl->setContentsMargins(8, 0, 0, 0);  // 左边留一点空
    hl->setSpacing(0);
    hl->addWidget(m_titleLabel);
    hl->addWidget(m_btnMin);
    hl->addWidget(m_btnMax);
    hl->addWidget(m_btnClose);
    setLayout(hl);

    // 跟踪窗口标题 / 状态变化
    if (m_window) {
        m_window->installEventFilter(this);
    }

    updateMaximizeRestoreIcon();
}

QSize CustomTitleBar::sizeHint() const
{
    // 这里给一个默认高度，实际可以用 QSS 改 min-height
    return {200, 32};
}

void CustomTitleBar::setTitle(const QString& title)
{
    m_titleLabel->setText(title);
    if (m_window) {
        m_window->setWindowTitle(title);
    }
}

QString CustomTitleBar::title() const { return m_titleLabel->text(); }

// ============== 鼠标拖动窗口 ==============

void CustomTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_leftPressed = true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_dragStartGlobalPos = event->globalPosition().toPoint();
#else
        m_dragStartGlobalPos = event->globalPos();
#endif
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (m_leftPressed && m_window) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QPoint globalPos = event->globalPosition().toPoint();
#else
        QPoint globalPos = event->globalPos();
#endif
        QPoint delta = globalPos - m_dragStartGlobalPos;
        m_dragStartGlobalPos = globalPos;
        m_window->move(m_window->pos() + delta);
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_leftPressed = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        onClickedMaximizeRestore();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

// ============== 按钮行为 ==============

void CustomTitleBar::onClickedMinimize()
{
    if (m_window) {
        m_window->showMinimized();
    }
    emit minimizeRequested();
}

void CustomTitleBar::onClickedMaximizeRestore()
{
    if (!m_window) {
        return;
    }
    if (m_window->isMaximized()) {
        m_window->showNormal();
        emit restoreRequested();
    } else {
        m_window->showMaximized();
        emit maximizeRequested();
    }
    updateMaximizeRestoreIcon();
}

void CustomTitleBar::onClickedClose()
{
    if (m_window) {
        m_window->close();
    }
    emit closeRequested();
}

void CustomTitleBar::updateMaximizeRestoreIcon()
{
    if (!m_window) {
        return;
    }
    QStyle* st = style();
    if (m_window->isMaximized()) {
        m_btnMax->setIcon(st->standardIcon(QStyle::SP_TitleBarNormalButton));
    } else {
        m_btnMax->setIcon(st->standardIcon(QStyle::SP_TitleBarMaxButton));
    }
}

// ============== 跟踪窗口事件（标题变化、状态变化） ==============

bool CustomTitleBar::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_window) {
        switch (event->type()) {
            case QEvent::WindowTitleChange:
                m_titleLabel->setText(m_window->windowTitle());
                break;
            case QEvent::WindowStateChange:
                updateMaximizeRestoreIcon();
                break;
            default:
                break;
        }
    }
    return QWidget::eventFilter(watched, event);
}
