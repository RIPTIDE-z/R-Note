#include "config_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

#include <QWKWidgets/widgetwindowagent.h>
#include "title_bar.h"   // 你自己的自定义标题栏类

ConfigDialog::ConfigDialog(const QString& baseUrl,
    const QString& projectRoot,
    QWidget* parent)
    : QDialog(parent)
{
    // 去掉系统标题栏，用自定义 titlebar + QWindowKit
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    resize(400, 320);
    setMinimumSize(400, 260);

    // 顶层 layout：0 margin，把真正内容放进一个容器 widget，方便圆角/背景
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto* frame = new QWidget(this);
    frame->setObjectName("configFrame");
    frame->setAttribute(Qt::WA_StyledBackground, true);
    frame->setStyleSheet(
        "#configFrame { "
        "  background-color: #111111; "
        "}"
        "QLineEdit { background-color: #1e1e1e; border: 1px solid #444444; "
        "           border-radius: 4px; padding: 6px; }"
        "QPushButton { background-color: #7b3ff2; color: white; "
        "             border-radius: 6px; padding: 6px 10px; font-weight: 600; }"
        "QPushButton:disabled { background-color: #555555; }"
        "QLabel#errorLabel { color: #ff5c5c; }"
    );
    rootLayout->addWidget(frame);

    auto* frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->setSpacing(0);

    // 自定义标题栏，传入当前对话框作为 window
    auto* titleBar = new CustomTitleBar(this, frame);
    titleBar->setTitle(tr(""));

    // 只在这个对话框里隐藏 最小化 / 最大化 按钮
    if (auto btnMin = titleBar->minButton()) {
        btnMin->hide();
    }
    if (auto btnMax = titleBar->maxButton()) {
        btnMax->hide();
    }

    frameLayout->addWidget(titleBar);

    // 内容区
    auto* content = new QWidget(frame);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(16, 16, 16, 16);
    contentLayout->setSpacing(12);

    auto* titleLabel = new QLabel(tr("配置服务器与笔记根目录"), content);
    QFont f = titleLabel->font();
    f.setPointSize(12);
    f.setBold(true);
    titleLabel->setFont(f);
    contentLayout->addWidget(titleLabel);

    auto* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    formLayout->setHorizontalSpacing(8);
    formLayout->setVerticalSpacing(10);

    m_baseUrlEdit = new QLineEdit(content);
    m_baseUrlEdit->setPlaceholderText(tr("例如：http://127.0.0.1:8080"));
    m_baseUrlEdit->setText(baseUrl);
    formLayout->addRow(tr("Base URL："), m_baseUrlEdit);

    auto* rootRowLayout = new QHBoxLayout();
    rootRowLayout->setContentsMargins(0, 0, 0, 0);
    rootRowLayout->setSpacing(6);

    m_projectRootEdit = new QLineEdit(content);
    m_projectRootEdit->setPlaceholderText(tr("笔记根目录，例如：D:/Notes"));
    m_projectRootEdit->setText(projectRoot);

    auto* browseBtn = new QPushButton(tr("浏览…"), content);
    browseBtn->setFixedWidth(72);

    rootRowLayout->addWidget(m_projectRootEdit, 1);
    rootRowLayout->addWidget(browseBtn, 0);

    formLayout->addRow(tr("Project Root："), rootRowLayout);

    contentLayout->addLayout(formLayout);

    m_errorLabel = new QLabel(content);
    m_errorLabel->setObjectName("errorLabel");
    m_errorLabel->clear();
    contentLayout->addWidget(m_errorLabel);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    auto* cancelBtn = new QPushButton(tr("取消"), content);
    auto* okBtn = new QPushButton(tr("确定"), content);

    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(okBtn);

    contentLayout->addLayout(buttonLayout);

    frameLayout->addWidget(content);

    // QWindowKit：让这个 Dialog 也用自定义标题栏拖动/缩放
    auto* agent = new QWK::WidgetWindowAgent(this);
    agent->setup(this);
    agent->setTitleBar(titleBar);
    // 只标记 Close 按钮：
    agent->setSystemButton(QWK::WindowAgentBase::Close, titleBar->closeButton());

    // 标题栏上的按钮逻辑（这里只需要关闭）
    connect(titleBar->closeButton(), &QToolButton::clicked,
        this, &QDialog::reject);

    connect(cancelBtn, &QPushButton::clicked,
        this, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked,
        this, &ConfigDialog::onAcceptClicked);
    connect(browseBtn, &QPushButton::clicked,
        this, &ConfigDialog::onBrowseProjectRoot);
}

void ConfigDialog::onBrowseProjectRoot()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("选择项目根目录"),
        m_projectRootEdit->text().isEmpty() ? QString() : m_projectRootEdit->text()
    );
    if (!dir.isEmpty()) {
        m_projectRootEdit->setText(dir);
    }
}

void ConfigDialog::onAcceptClicked()
{
    // 获取到填写的配置
    const QString baseUrl = m_baseUrlEdit->text().trimmed();
    const QString projectDir = m_projectRootEdit->text().trimmed();

    // 判空
    if (baseUrl.isEmpty() || projectDir.isEmpty()) {
        m_errorLabel->setText(tr("Base URL 和 Project Root 都不能为空。"));
        return;
    }

    m_errorLabel->clear();
    // 发出接受配置的信号，由login/register窗口接受
    emit configAccepted(baseUrl, projectDir);
    accept();
}
