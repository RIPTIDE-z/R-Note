#include "markdown_editor_widget.h"
#include "markdown_preview_widget.h"   
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTimer>
#include <QDebug>

MarkdownEditorWidget::MarkdownEditorWidget(QWidget* parent)
    : QWidget(parent)
{
    // 顶部：一个切换按钮
    m_toggleButton = new QPushButton(tr("切换到预览"), this);

    // 中间：编辑器 + 预览，用 QStackedWidget 切换
    m_editor = new QPlainTextEdit(this);
    m_preview = new MarkdownPreviewWidget(this);
    m_stack = new QStackedWidget(this);

    m_stack->addWidget(m_editor);   // index 0: 编辑
    m_stack->addWidget(m_preview);  // index 1: 预览
    m_stack->setCurrentWidget(m_preview);

    // 自动保存计时器
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setSingleShot(true);

    // 布局
    auto* topLayout = new QHBoxLayout;
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(m_toggleButton);
    topLayout->addStretch();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_stack);

    // 信号连接
    connect(m_toggleButton, &QPushButton::clicked,
        this, &MarkdownEditorWidget::toggleMode);

    connect(m_editor, &QPlainTextEdit::textChanged,
        this, &MarkdownEditorWidget::onEditorTextChanged);

    connect(m_autoSaveTimer, &QTimer::timeout,
        this, &MarkdownEditorWidget::onAutoSaveTimeout);
}

void MarkdownEditorWidget::setFilePath(const QString& path)
{
    m_filePath = path;
}

QString MarkdownEditorWidget::markdownText() const
{
    return m_editor->toPlainText();
}

void MarkdownEditorWidget::setMarkdownText(const QString& text, bool updateEditor)
{
    if (updateEditor) {
        m_editor->setPlainText(text);
    }
    // 如果当前在预览模式，就顺便刷新一下
    if (m_inPreview) {
        updatePreview();
    }
}

void MarkdownEditorWidget::loadFromFile()
{
    if (m_filePath.isEmpty()) {
        qWarning() << "MarkdownEditorWidget::loadFromFile: filePath is empty";
        return;
    }

    QFile f(m_filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for read:" << m_filePath << f.errorString();
        return;
    }

    const QString content = QString::fromUtf8(f.readAll());
    f.close();

    // 这里设 updateEditor = true，同时根据当前模式决定是否更新预览
    setMarkdownText(content, true);
}

void MarkdownEditorWidget::saveToFile()
{
    if (m_filePath.isEmpty()) {
        // 没有路径就不自动保存，可以按需改成弹出保存对话框
        return;
    }

    QFile f(m_filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "Failed to open file for write:" << m_filePath << f.errorString();
        return;
    }

    const QByteArray data = markdownText().toUtf8();
    f.write(data);
    f.close();

    emit fileSaved(m_filePath);
}

void MarkdownEditorWidget::setPreviewMode(bool on)
{
    if (m_inPreview == on)
        return;

    m_inPreview = on;

    if (m_inPreview) {
        updatePreview();
        m_stack->setCurrentWidget(m_preview);
        m_toggleButton->setText(tr("切换到编辑"));
    }
    else {
        m_stack->setCurrentWidget(m_editor);
        m_toggleButton->setText(tr("切换到预览"));
    }

    emit modeChanged(m_inPreview);
}

void MarkdownEditorWidget::toggleMode()
{
    setPreviewMode(!m_inPreview);
}

void MarkdownEditorWidget::onEditorTextChanged()
{
    emit contentChanged();

    // 防抖自动保存：每次修改重置计时器
    m_autoSaveTimer->start(1000); // 1 秒不输入就保存
}

void MarkdownEditorWidget::onAutoSaveTimeout()
{
    saveToFile();
}

void MarkdownEditorWidget::updatePreview()
{
    // 这里假设 MarkdownPreviewWidget 有 setMarkdownText
    m_preview->setMarkdownText(markdownText());
}
