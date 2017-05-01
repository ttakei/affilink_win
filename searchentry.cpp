#include "searchentry.h"
#include <QUrl>
#include <QtNetwork/QNetworkReply>
#include <QTextCodec>

SearchEntry::SearchEntry(const QString& id) {
    m_id = id;
    m_status = Status::INIT;
    m_enable_search_product_code = false;
}

void SearchEntry::init(Ini* ini, Csv* csv) {
    // 状態
    m_status = Status::INIT;

    // 設定で値を決めるメンバ変数
    m_force_output = ini->get("global/force_output", "false").toBool();
    ini->begingroup(getIniGroup());
    m_name = ini->get("name").toString();
    m_search_url_template = ini->get("search_url_template").toString();
    m_regex_product_url = QRegExp(ini->get("regex_product_url").toString());
    m_regex_product_img_url = QRegExp(ini->get("regex_product_img_url").toString());
    m_regex_product_no = QRegExp(ini->get("regex_product_no").toString());
    m_regex_product_price = QRegExp(ini->get("regex_product_price").toString());
    m_regex_product_date = QRegExp(ini->get("regex_product_date").toString());
    if (ini->contains("regex_product_code")) {
        m_regex_product_code = QRegExp(ini->get("regex_product_code").toString());
        m_enable_search_product_code = true;
    } else {
        m_enable_search_product_code = false;
    }
    m_prefix_product_url = ini->get("prefix_product_url").toString();
    m_prefix_product_img_url = ini->get("prefix_product_img_url").toString();
    m_enable = ini->get("enable", "false").toBool();
    // リンクテンプレート
    m_links.clear();
    int i = 0;
    while (true) {
        i++;
        QString key = QString("link_template") + QString(QString::number(i));
        if (ini->contains(key)) {
            QString link_template = ini->get(key).toString();
            m_links.push_back(link_template);
            continue;
        }
        // 番号1に限り番号はなくても良い
        if (i == 1) {
            key = QString("link_template");
            if (ini->contains(key)) {
                QString link_template = ini->get(key).toString();
                m_links.push_back(link_template);
                continue;
            }
        }
        break;
    }
    // 商品ページパース設定
    i = 0;
    m_regex_product_page_word.clear();
    while (true) {
        i++;
        QString key = QString("regex_product_page_word") + QString(QString::number(i));
        if (ini->contains(key)) {
            QString regex_product_page_word = ini->get(key).toString();
            m_regex_product_page_word.push_back(QRegExp(regex_product_page_word));
            continue;
        }

        // 番号1に限り番号はなくても良い
        if (i == 1) {
            key = QString("regex_product_page_word");
            if (ini->contains(key)) {
                QString regex_product_page_word = ini->get(key).toString();
                m_regex_product_page_word.push_back(QRegExp(regex_product_page_word));
                continue;
            }
        }
        break;
    }
    m_regex_product_page_desc = QRegExp(ini->get("regex_product_page_desc").toString());
    if (m_regex_product_page_word.size() > 0 || !m_regex_product_page_desc.pattern().isEmpty()) {
        m_enable_parse_product = true;
    } else {
        m_enable_parse_product = false;
    }
    ini->endgroup();

    // 他メンバ変数
    m_product_name.clear();
    m_jancode.clear();
    m_search_word.clear();
    m_search_eword.clear();
    m_product_code.clear();
    m_remarks1.clear();
    m_remarks2.clear();
    m_remarks3.clear();
    m_remarks4.clear();
    m_remarks5.clear();

    m_search_url.clear();
    m_html.clear();
    m_product_url.clear();
    m_product_eurl.clear();
    m_product_img_url.clear();
    m_product_no.clear();
    m_product_price.clear();
    m_product_date.clear();
    m_wwl.clear();
    m_csv = csv;
}

void SearchEntry::setStatusNosearch() {
    m_status = Status::NOSEARCH;
}

void SearchEntry::setStatusSearching() {
    m_status = Status::SEARCHING;
}

void SearchEntry::setStatusSearched() {
    m_status = Status::SEARCHED;
}

void SearchEntry::setStatusSearcherror() {
    m_status = Status::SEARCHERROR;
}

void SearchEntry::setStatusSearchbuilt() {
    m_status = Status::SEARCHBUILT;
}

void SearchEntry::setStatusParsingproduct() {
    m_status = Status::PARSINGPRODUCT;
}

void SearchEntry::setStatusParsedproduct() {
    m_status = Status::PARSEDPRODUCT;
}

void SearchEntry::setStatusResultbuilt() {
    m_status = Status::RESULTBUILT;
}

bool SearchEntry::isStatus(SearchEntry::Status status) {
    return m_status == status;
}

bool SearchEntry::isStatusSearching() {
    return isStatus(SearchEntry::Status::SEARCHING);
}

bool SearchEntry::isStatusSearched() {
    return isStatus(SearchEntry::Status::SEARCHED);
}

bool SearchEntry::isStatusNosearch() {
    return isStatus(SearchEntry::Status::NOSEARCH);
}

bool SearchEntry::isStatusSearchbuilt() {
    return isStatus(SearchEntry::Status::SEARCHBUILT);
}

bool SearchEntry::isStatusParsedproduct() {
    return isStatus(SearchEntry::Status::PARSEDPRODUCT);
}

bool SearchEntry::isStatusResultbuilt() {
    return isStatus(SearchEntry::Status::RESULTBUILT);
}

void SearchEntry::buildSearchUrl() {
    QString search_url_str = m_search_url_template;
    bind(search_url_str);
    m_search_url = QUrl(search_url_str);
    return;
}

bool SearchEntry::buildSearchResultProductCode() {
    m_regex_product_code.indexIn(m_html);
    QStringList match = m_regex_product_code.capturedTexts();
    if (match.size() > 1) {
        QString product_code = match.at(1).toUpper();
        if (product_code.isEmpty()) {
            return false;
        }
        m_product_code = product_code;
        return true;
    }
    return false;
}

void SearchEntry::buildSearchResult() {
    buildProductUrl();    
    buildProductImgUrl();
    buildProductNO();
    buildProductPrice();
    buildProductDate();
    setStatusSearchbuilt();
    return;
}

void SearchEntry::buildParseProductResult() {
    buildProductPageWord();
    buildProductPageDesc();
    setStatusParsedproduct();
    return;
}

void SearchEntry::buildResult(Wwl* wwl) {
    for (QVector<QString>::iterator it = m_links.begin(); it != m_links.end(); ++it) {
        bind(*it);
    }
    buildWWL(wwl);
    setStatusResultbuilt();
    return;
}

void SearchEntry::buildProductUrl() {
    m_product_url = m_prefix_product_url;
    m_regex_product_url.indexIn(m_html);
    QStringList match = m_regex_product_url.capturedTexts();
    if (match.size() > 1) {
        m_product_url += match.at(1);
    }
    // prefix設定値との結合のためbind必要
    // bind後にurlエンコードする必要がある
    bind(m_product_url);

    QTextCodec* codec = QTextCodec::codecForLocale();
    m_product_eurl = codec->toUnicode(QUrl::toPercentEncoding(m_product_url));
}

void SearchEntry::buildProductImgUrl() {
    m_product_img_url = m_prefix_product_img_url;
    m_regex_product_img_url.indexIn(m_html);
    QStringList match = m_regex_product_img_url.capturedTexts();
    if (match.size() > 1) {
        m_product_img_url += match.at(1);
    }
    // prefix設定値との結合のためbind必要
    // bind後にurlエンコードする必要がある
    bind(m_product_img_url);

    QTextCodec* codec = QTextCodec::codecForLocale();
    m_product_img_eurl = codec->toUnicode(QUrl::toPercentEncoding(m_product_img_url));
}

void SearchEntry::buildProductNO() {
    m_regex_product_no.indexIn(m_html);
    QStringList match = m_regex_product_no.capturedTexts();
    if (match.size() > 1) {
        m_product_no = match.at(1).toUpper();
    }
    return;
}

void SearchEntry::buildProductPrice() {
    m_regex_product_price.indexIn(m_html);
    QStringList match = m_regex_product_price.capturedTexts();
    if (match.size() > 1) {
        m_product_price = match.at(1);
    }
    return;
}

void SearchEntry::buildProductDate() {
    m_regex_product_date.indexIn(m_html);
    QStringList match = m_regex_product_date.capturedTexts();
    if (match.size() > 1) {
        m_product_date = match.at(1);
    }
    return;
}

void SearchEntry::buildProductPageWord() {
    for (int i = 0; i < m_regex_product_page_word.size(); ++i) {
        QRegExp regex = m_regex_product_page_word.at(i);
        if (regex.indexIn(m_product_html) >= 0) {
            m_product_page_word.push_back(true);
        } else {
            m_product_page_word.push_back(false);
        }
    }
    return;
}

void SearchEntry::buildProductPageDesc() {
    m_regex_product_page_desc.indexIn(m_product_html);
    QStringList match = m_regex_product_page_desc.capturedTexts();
    if (match.size() > 1) {
        m_product_page_desc = match.at(1);
    }
    return;
}

bool SearchEntry::buildWWL(Wwl* wwl) {
    m_wwl = wwl->get(m_id);
    if (m_wwl.isEmpty()) {
        return false;
    }

    int i = 1;
    for (QVector<QString>::iterator it = m_links.begin(); it != m_links.end(); ++it) {
        QString link = *it;
        QString var_name = QString(":link_template") + QString(QString::number(i));

        m_wwl.replace(var_name, link);
        if (i == 1) {
            // 変数名の最後が1でない場合も
            m_wwl.replace(":link_template", link);
        }
        i++;
    }
    bind(m_wwl);

    return true;
}

QString SearchEntry::getIniGroup() {
    return QString(SEARCHENTRY_ENTRY_GROUP_NAME_PREFIX) + m_id;
}

void SearchEntry::bind(QString& str) {
    str
            .replace(":id", m_id)
            .replace(":name", m_name)
            .replace(":product_name", m_product_name)
            .replace(":product_url", m_product_url)
            .replace(":product_eurl", m_product_eurl)
            .replace(":product_img_url", m_product_img_url)
            .replace(":product_img_eurl", m_product_img_eurl)
            .replace(":product_no", m_product_no)
            .replace(":product_price", m_product_price)
            .replace(":jancode", m_jancode)
            .replace(":search_word", m_search_word)
            .replace(":search_eword", m_search_eword)
            .replace(":product_code", m_product_code)
            .replace(":remarks_1", m_remarks1)
            .replace(":remarks_2", m_remarks2)
            .replace(":remarks_3", m_remarks3)
            .replace(":remarks_4", m_remarks4)
            .replace(":remarks_5", m_remarks5)
            .replace(":data_product_code", m_csv->code(m_jancode))
            .replace(":data_product_name", m_csv->name(m_jancode))
            .replace(":data_product_store", m_csv->store(m_jancode))
            .replace(":data_product_jancode", m_csv->jancode(m_jancode))
            .replace(":data_product_minprice", m_csv->min_price(m_jancode))
            .replace(":data_product_date", m_csv->date(m_jancode))
            .replace(":data_product_soldout", m_csv->soldout(m_jancode))
            .replace(":product_page_desc", m_csv->soldout(m_product_page_desc));
}
