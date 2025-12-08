#pragma once

#include <QString>

namespace MdRender
{
    QString MarkdowntoHtml(const QString& markdown); 

    // TODO: 要加主题 / CSS / 扩展选项，加个 options 结构体
    // QString toHtml(const QString& markdown, const RenderOptions& opt);
}
