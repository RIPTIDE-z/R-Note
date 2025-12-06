#pragma once

#include <QDialog>

class QLineEdit;
class QLabel;

/**
 * 可复用的配置对话框：
 * - baseUrl：服务端根地址
 * - projectRoot：本地项目根目录（Note 根目录）
 *
 * 使用方式：
 *   ConfigDialog dlg(currentBaseUrl, currentProjectRoot, parent);
 *   connect(&dlg, &ConfigDialog::configAccepted, this, [&](const QString &baseUrl, const QString &projectRoot) {
 *       // 在这里调用 http->setBaseUrl(baseUrl)，保存 projectRoot 等
 *   });
 *   dlg.exec();
 */
class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigDialog(const QString& baseUrl,
        const QString& projectRoot,
        QWidget* parent = nullptr);

signals:
    void configAccepted(const QString& baseUrl,
        const QString& projectRoot);

private slots:
    void onAcceptClicked();
    void onBrowseProjectRoot();

private:
    QLineEdit* m_baseUrlEdit = nullptr;
    QLineEdit* m_projectRootEdit = nullptr;
    QLabel* m_errorLabel = nullptr;
};
