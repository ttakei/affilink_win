#include "searchentry.h"
#include <QUrl>
#include <QtNetwork/QNetworkReply>
#include <QTextCodec>

SearchEntry::SearchEntry(const QString& id)
{
    m_id = id;
}

void SearchEntry::load(QSettings* settings)
{
    int max_template_num = settings->value("global/max_template_num", "1").toInt();
    m_force_output = settings->value("global/force_output", "false").toBool();

    QString group_name = getIniGroup();
    settings->beginGroup(group_name);
    m_name = settings->value("name").toString();
    m_search_url_template = settings->value("search_url_template").toString();
    m_regex_product_url = QRegExp(settings->value("regex_product_url").toString());
    m_regex_product_img_url = QRegExp(settings->value("regex_product_img_url").toString());
    m_regex_product_no = QRegExp(settings->value("regex_product_no").toString());
    m_regex_product_price = QRegExp(settings->value("regex_product_price").toString());
    m_regex_product_date = QRegExp(settings->value("regex_product_date").toString());
    m_prefix_product_url = settings->value("prefix_product_url").toString();
    m_prefix_product_img_url = settings->value("prefix_product_img_url").toString();
//    m_link_template = settings->value("link_template").toString();
//    m_link_template_default = settings->value("link_template_default").toString();
    for (int i = 1; i <= max_template_num; ++i) {
        QString key;
        QString default_key;
        if (i == 1) {
            key = QString("link_template");
            default_key = QString("link_template_default");
        } else {
            key = QString("link_template") + QString(QString::number(i));
            default_key = QString("link_template_default") + QString(QString::number(i));
        }

        QString link_template = settings->value(key).toString();
        QString link_template_default = settings->value(default_key).toString();
        if (i == 1) {
            if (link_template.isEmpty()) {
                key = QString("link_template1");
                link_template = settings->value(key).toString();
            }
            if (link_template_default.isEmpty()) {
                default_key = QString("link_template_default1");
                link_template_default = settings->value(default_key).toString();
            }
        }

        QPair<QString, QString> link_template_set;
        link_template_set.first = link_template;
        link_template_set.second = link_template_default;
        m_link_templates.push_back(link_template_set);
    }
    m_enable = settings->value("enable").toBool();
    settings->endGroup();
}

void SearchEntry::setInput(
    QString product_name,
    QString jancode,
    QString search_word,
    QString product_code,
    QString remarks1,
    QString remarks2,
    QString remarks3,
    QString output_template_folder
) {
    m_product_name = product_name;
    m_jancode = jancode;
    m_search_word = search_word;
    m_product_code = product_code;
    m_remarks1 = remarks1;
    m_remarks2 = remarks2;
    m_remarks3 = remarks3;
    m_wwl_template_file = output_template_folder + QString("\\") + m_id + QString(".tpl");
    m_wwl_template_file_default = output_template_folder + QString("\\default.tpl");
}

bool SearchEntry::buildSearchUrl()
{
    if (m_jancode.isEmpty()) {
        return false;
    }

    QString search_url_str = QString(m_search_url_template).replace(":jancode", m_jancode)
        .replace(":search_word", m_search_word)
        .replace(":product_code", m_product_code)
        .replace(":remarks_1", m_remarks1)
        .replace(":remarks_2", m_remarks2)
        .replace(":remarks_3", m_remarks3);
    m_search_url = QUrl(search_url_str);
    return true;
}


bool SearchEntry::fetch(QNetworkAccessManager* nam, QMap<QNetworkReply*, QString>& nm)
{
    if (!buildSearchUrl()) {
        return false;
    }

    nm.insert(nam->get(QNetworkRequest(m_search_url)), m_id);
    return true;
}


bool SearchEntry::extractProductUrl(const QString& search_result_html)
{
    m_product_url = m_prefix_product_url;
    m_regex_product_url.indexIn(search_result_html);
    QStringList match = m_regex_product_url.capturedTexts();
    if (match.size() > 1) {
        m_product_url += match.at(1);
    }
    m_product_url.replace(":product_name", m_product_name)
            .replace(":name", m_name)
            .replace(":product_no", m_product_no)
            .replace(":jancode", m_jancode)
            .replace(":search_word", m_search_word)
            .replace(":product_code", m_product_code)
            .replace(":remarks1", m_remarks1)
            .replace(":remarks2", m_remarks2)
            .replace(":remarks3", m_remarks3)
            .replace(":jancode", m_jancode);
    QTextCodec* codec = QTextCodec::codecForLocale();
    m_product_eurl = codec->toUnicode(QUrl::toPercentEncoding(m_product_url));
    return !m_product_url.isEmpty();
}


bool SearchEntry::extractProductImgUrl(const QString& search_result_html)
{
    m_product_img_url = m_prefix_product_img_url;
    m_regex_product_img_url.indexIn(search_result_html);
    QStringList match = m_regex_product_img_url.capturedTexts();
    if (match.size() > 1) {
        m_product_img_url += match.at(1);
    }
    m_product_img_url.replace(":jancode", m_jancode);
    return !m_product_img_url.isEmpty();
}


bool SearchEntry::extractProductNO(const QString& search_result_html)
{
    m_regex_product_no.indexIn(search_result_html);
    QStringList match = m_regex_product_no.capturedTexts();
    if (match.size() > 1) {
        m_product_no = match.at(1).toUpper();
    }
    return !m_product_no.isEmpty();
}


bool SearchEntry::extractProductPrice(const QString& search_result_html)
{
    m_regex_product_price.indexIn(search_result_html);
    QStringList match = m_regex_product_price.capturedTexts();
    if (match.size() > 1) {
        m_product_price = match.at(1);
    }
    return !m_product_price.isEmpty();
}


bool SearchEntry::extractProductDate(const QString& search_result_html)
{
    m_regex_product_date.indexIn(search_result_html);
    QStringList match = m_regex_product_date.capturedTexts();
    if (match.size() > 1) {
        m_product_date = match.at(1);
    }
    return !m_product_date.isEmpty();
}


bool SearchEntry::buildLink()
{
    if (m_product_url == m_prefix_product_url ||
        m_product_img_url == m_prefix_product_img_url ||
        m_product_price.isEmpty()) {
        m_ok = false;
    } else {
        m_ok = true;
    }

    m_links.clear();
    for (QVector<QPair<QString, QString> >::iterator it = m_link_templates.begin(); it != m_link_templates.end(); ++it) {
        QString link;
        QString link_template = it->first;
        QString link_template_default = it->second;

        if (!m_force_output && !m_ok) {
            if (link_template_default.isEmpty()) {
                link = "";
            }
            link_template = link_template_default;
        }
        link = link_template.replace(":product_name", m_product_name)
            .replace(":name", m_name)
            .replace(":product_url", m_product_url)
            .replace(":product_eurl", m_product_eurl)
            .replace(":product_img_url", m_product_img_url)
            .replace(":product_no", m_product_no)
            .replace(":product_price", m_product_price)
            .replace(":jancode", m_jancode)
            .replace(":search_word", m_search_word)
            .replace(":product_code", m_product_code)
            .replace(":remarks1", m_remarks1)
            .replace(":remarks2", m_remarks2)
            .replace(":remarks3", m_remarks3)
            .replace(":jancode", m_jancode);

        m_links.push_back(link);
    }

    return true;
}

bool SearchEntry::buildWWL()
{
    // テンプレートファイル読み込み
    QFile file(m_wwl_template_file);
    if (!file.open(QIODevice::ReadOnly)) {
        file.setFileName(m_wwl_template_file_default);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
    }

    QString wwl_template;
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextStream in(&file);
    in.setCodec( codec );
    wwl_template = in.readAll();
    file.close();

    m_wwl = wwl_template
            .replace(":id", m_id)
            .replace(":product_name", m_product_name)
            .replace(":name", m_name)
            .replace(":product_url", m_product_url)
            .replace(":product_eurl", m_product_eurl)
            .replace(":product_img_url", m_product_img_url)
            .replace(":product_no", m_product_no)
            .replace(":product_price", m_product_price)
            .replace(":jancode", m_jancode)
            .replace(":search_word", m_search_word)
            .replace(":product_code", m_product_code)
            .replace(":remarks1", m_remarks1)
            .replace(":remarks2", m_remarks2)
            .replace(":remarks3", m_remarks3)
            .replace(":jancode", m_jancode);

    QVector<QString>::iterator ite;
    int i = 1;
    for (ite = m_links.begin(); ite != m_links.end(); ++ite) {
        QString link = *ite;
        QString var_name = QString(":link_template") + QString(QString::number(i));

        m_wwl = m_wwl.replace(var_name, link);
        if (i == 1) {
            m_wwl = m_wwl.replace(":link_template", link);
        }
    }

    return true;
}

QString SearchEntry::getIniGroup()
{
    return  QString(SEARCHENTRY_ENTRY_GROUP_NAME_PREFIX) + m_id;
}
