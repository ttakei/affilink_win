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
    m_link_template = settings->value("link_template").toString();
    m_link_template_default = settings->value("link_template_default").toString();
    m_enable = settings->value("enable").toBool();
    settings->endGroup();
}

void SearchEntry::setInput(QString product_name, QString jancode)
{
    m_product_name = product_name;
    m_jancode = jancode;
}

bool SearchEntry::buildSearchUrl()
{
    if (m_jancode.isEmpty()) {
        return false;
    }

    QString search_url_str = QString(m_search_url_template).replace(":jancode", m_jancode);
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
    m_product_url.replace(":jancode", m_jancode);
    //QString dummy_url("http://dummy.com?url=");
    //QUrl dummy(dummy_url + m_product_url);
    //m_product_eurl = dummy.toString().mid(dummy_url.length() - 1);
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

    if (!m_ok) {
        if (m_link_template_default.isEmpty()) {
            m_link = "";
            return false;
        }
        m_link_template = m_link_template_default;
    }
    m_link = m_link_template.replace(":product_name", m_product_name)
        .replace(":name", m_name)
        .replace(":product_url", m_product_url)
        .replace(":product_eurl", m_product_eurl)
        .replace(":product_img_url", m_product_img_url)
        .replace(":product_no", m_product_no)
        .replace(":product_price", m_product_price)
        .replace(":jancode", m_jancode);

    return true;
}


QString SearchEntry::getIniGroup()
{
    return  QString(SEARCHENTRY_ENTRY_GROUP_NAME_PREFIX) + m_id;
}
