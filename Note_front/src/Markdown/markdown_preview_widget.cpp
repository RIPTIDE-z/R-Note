#include "markdown_preview_widget.h"
#include <QVBoxLayout>
#include <QTextBrowser>
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
}

void MarkdownPreviewWidget::setMarkdownText(const QString& text)
{
    const QString html = MdRender::MarkdowntoHtml(text);
    m_browser->setHtml(html);
}

void MarkdownPreviewWidget::setHtmlDirect(const QString& html)
{
    m_browser->setHtml(html);
}
