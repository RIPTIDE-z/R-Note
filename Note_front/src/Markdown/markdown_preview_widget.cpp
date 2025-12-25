#include "markdown_preview_widget.h"

#include <QTextBrowser>
#include <QVBoxLayout>

#include "markdown_renderer.h"

MarkdownPreviewWidget::MarkdownPreviewWidget(QWidget* parent)
    : QWidget(parent),
      m_browser(new QTextBrowser(this))
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_browser);

    m_browser->document()->setIndentWidth(10);
    m_browser->setOpenExternalLinks(true);

    // 隐藏滚动条（仍然可以用滚轮滚动）
    m_browser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_browser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFont baseFont;
    baseFont.setFamily("JetBrains Mono");
    baseFont.setPointSize(13);
    baseFont.setStyleHint(QFont::Monospace);
    m_browser->setFont(baseFont);

    // 设置预览背景色（Base 对 QTextBrowser 的 viewport 生效）
    QPalette pal = m_browser->palette();
    pal.setColor(QPalette::Base, QColor("#2a2a2a"));  // 和 HTML body 同色
    pal.setColor(QPalette::Text, QColor("#E0E0E0"));
    m_browser->setPalette(pal);

    m_browser->setFrameStyle(QFrame::NoFrame);
}

void MarkdownPreviewWidget::setMarkdownText(const QString& text)
{
    const QString html = MdRender::MarkdowntoHtml(text);
    m_browser->setHtml(html);
}

void MarkdownPreviewWidget::setHtmlDirect(const QString& html) { m_browser->setHtml(html); }
