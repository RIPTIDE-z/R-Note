#pragma once
#include <QWidget>

class QPlainTextEdit;
class QPushButton;
class QStackedWidget;
class QTimer;
class MarkdownPreviewWidget;

class MarkdownEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MarkdownEditorWidget(QWidget* parent = nullptr);

    void setFilePath(const QString& path);
    QString filePath() const { return m_filePath; }

    QString markdownText() const;
    void setMarkdownText(const QString& text, bool updateEditor = true);

signals:
    void contentChanged();
    void fileSaved(const QString& path);
    void modeChanged(bool previewMode); // true=预览

public slots:
    void loadFromFile();
    void saveToFile();
    void setPreviewMode(bool on);
    void toggleMode();

private slots:
    void onEditorTextChanged();
    void onAutoSaveTimeout();

private:
    void updatePreview(); // 调用 MdRender::toHtml

private:
    QPlainTextEdit* m_editor;
    MarkdownPreviewWidget* m_preview;
    QStackedWidget* m_stack;
    QPushButton* m_toggleButton;

    // md文件路径
    QString   m_filePath;
    QTimer* m_autoSaveTimer;

    // 标识 编辑/预览
    bool      m_inPreview = false;
};
