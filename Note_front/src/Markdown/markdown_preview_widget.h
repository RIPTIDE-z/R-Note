#pragma once
#include <QWidget>

class QTextBrowser;

class MarkdownPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MarkdownPreviewWidget(QWidget* parent = nullptr);

public slots:
    void setMarkdownText(const QString& text);   // 外部传 md 文本
    void setHtmlDirect(const QString& html);     // 直接显示现成的html

private:
    QTextBrowser* m_browser;
};
