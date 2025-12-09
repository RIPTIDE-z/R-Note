#include "markdown_renderer.h"

#include <QDebug>

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

        qDebug().noquote() << "md4c html:" << html;

        if (rc != 0)
        {
            return QStringLiteral("<p><b>md4c 解析失败</b></p><pre>%1</pre>")
                .arg(markdown.toHtmlEscaped());
        }

        QString fullHtml = QStringLiteral(
            "<html>"
            "<head>"
            "<meta charset=\"utf-8\">"
            "<style>"
            /* 整体背景 / 基础字体：颜色、配色不在这里控制，只管排版 */
            "body {"
            "  background: transparent;"     /* 背景交给 QTextBrowser 的 palette 管 */
            "  color: #E0E0E0;"              /* 默认文字颜色 */
            "  padding: 18px 20px;"          /* 四周留一点空隙 */
            "  margin: 0;"
            "}"

            /* 标题风格 */
            "h1 {"
            "  font-size: 22px;"
            "  font-weight: 700;"
            "  margin: 0.8em 0 0.4em;"
            "}"
            "h2 {"
            "  font-size: 18px;"
            "  font-weight: 600;"
            "  margin: 0.9em 0 0.4em;"
            "}"
            "h3 {"
            "  font-size: 16px;"
            "  font-weight: 600;"
            "  margin: 0.8em 0 0.4em;"
            "}"

            /* 强调 */
            "strong { color: #ffd479; }"
            "em { color: #ff9ac1; }"

            /* 链接 */
            "a {"
            "  color: #7b3ff2;"
            "  text-decoration: none;"
            "}"
            "a:hover {"
            "  text-decoration: underline;"
            "}"

            /* 水平线 */
            "hr {"
            "  border: none;"
            "  border-top: 1px solid #333333;"
            "  margin: 1.2em 0;"
            "}"

            /* 列表 */
            "ul, ol {"
            "  margin: 0.4em 0 0.6em 1.8em;"
            "}"
            "li {"
            "  margin: 0.1em 0;"
            "}"

            /* 行内代码：小块高亮 */
            "code {"
            "  font-family: 'JetBrains Mono','Fira Code','Consolas',monospace;"
            "  font-size: 18px;"             /* 行内略小一点，看起来更像标记 */
            "  background: #222222;"
            "  color: #ffd479;"
            "  padding: 1px 4px;"
            "  border-radius: 4px;"
            "}"

            /* 代码块（<pre><code>…</code></pre>） */
            "pre {"
            "  background: #31353a;"    
            "  border-radius: 0;"
            "  border-left: 3px solid #3b3b3b;" /* 左边竖线标识代码块 */
            "  padding: 0 0 0 10px;"         /* 左侧竖线和文字之间留点距离 */
            "  margin: 10px 0 14px;"
            "  font-family: 'JetBrains Mono','Fira Code','Consolas',monospace;"
            "  font-size: 18px;"             /* 代码块字号：略小于正文、略大于行内 */
            "  white-space: pre;"            /* 保留缩进 / 换行 */
            "}"
            "pre code {"
            "  background: transparent;"     /* 把行内 code 的底色清掉，避免重复高亮 */
            "  padding: 0;"
            "  color: #da627d;"              /* 用正文颜色，整体配色统一 */
            "}"

            /* 表格 */
            "table {"
            "  border-collapse: collapse;"
            "  margin: 0.8em 0;"
            "}"
            "th, td {"
            "  border: 1px solid #333333;"
            "  padding: 6px 10px;"
            "}"
            "th {"
            "  background: #181818;"
            "}"

            /* 引用块：整体高亮 */
            "blockquote {"
            "  background: #262626;"          /* 整块底色，高亮但别太亮 */
            "  border-left: 5px solid #7b3ff2;"/* 左侧竖线保留 */
            "  margin: 0.8em 0;"              /* 上下间距稍微大一点 */
            "  padding: 6px 12px;"            /* 内边距多一点，让文字别贴边 */
            "  color: #dddddd;"               /* 文本稍微亮一点，读起来舒服 */
            "}"
            "blockquote p {"
            "  margin: 0.2em 0;"              /* 防止 Qt 给段落再加很大的 margin */
            "}"

            "</style>"
            "</head>"
            "<body>%1</body>"
            "</html>"
        ).arg(html);
        return fullHtml;
    }
}
