#include "markdown_renderer.h"

// md4c 头
extern "C" {
#include "md4c.h"
#include "md4c-html.h"
}

static void md4cOutputCallback(const MD_CHAR* text, MD_SIZE size, void* userdata)
{
    auto* out = static_cast<QString*>(userdata);
    out->append(QString::fromUtf8(text, static_cast<int>(size)));
}

namespace MdRender
{
    QString MarkdowntoHtml(const QString& markdown)
    {
        QByteArray utf8 = markdown.toUtf8();
        QString html;

        unsigned parserFlags = MD_FLAG_TABLES
            | MD_FLAG_TASKLISTS
            | MD_FLAG_STRIKETHROUGH;
        unsigned rendererFlags = 0;

        int rc = md_html(
            reinterpret_cast<const MD_CHAR*>(utf8.constData()),
            static_cast<MD_SIZE>(utf8.size()),
            md4cOutputCallback,
            &html,
            parserFlags,
            rendererFlags
        );

        if (rc != 0)
        {
            return QStringLiteral("<p><b>md4c 解析失败</b></p><pre>%1</pre>")
                .arg(markdown.toHtmlEscaped());
        }

        QString fullHtml = QStringLiteral(
            "<html><head><meta charset=\"utf-8\">"
            "<style>"
            "body { font-family: 'JetBrains Mono','Segoe UI',sans-serif;"
            "       background:#111; color:#ddd; padding:12px; }"
            // ... 这里放公共 CSS ...
            "</style>"
            "</head><body>%1</body></html>"
        ).arg(html);

        return fullHtml;
    }
}
