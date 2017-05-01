#include "searchentrymanager.h"
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QTextCodec>
#include <QtGlobal>

SearchEntryManager::SearchEntryManager(QObject* parent) : QObject(parent){}
SearchEntryManager::SearchEntryManager(Log* log, Ini* ini, Wwl* wwl, Csv* csv):
m_log(log),
m_ini(ini),
m_wwl(wwl),
m_csv(csv),
m_max_link_num(-1),
m_max_parsed_product_num(-1)
{
    m_nam_search = new QNetworkAccessManager((QObject*)this);
    QObject::connect(m_nam_search, SIGNAL(finished(QNetworkReply*)), (QObject*)this, SLOT(callbackSearch(QNetworkReply*)));

    m_nam_product = new QNetworkAccessManager((QObject*)this);
    QObject::connect(m_nam_product, SIGNAL(finished(QNetworkReply*)), (QObject*)this, SLOT(callbackProduct(QNetworkReply*)));
}

SearchEntryManager::~SearchEntryManager() {
    for (QMap<QString, SearchEntry*>::iterator it = m_map.begin(); it != m_map.end(); ++it) {
        if (it.value() != NULL) {
            delete it.value();
        }
    }
    if (m_nam_search != NULL) {
        QObject::disconnect(m_nam_search, SIGNAL(finished(QNetworkReply*)), (QObject*)this, SLOT(callbackSearch(QNetworkReply*)));
        delete m_nam_search;
    }
    if (m_nam_product != NULL) {
        QObject::disconnect(m_nam_product, SIGNAL(finished(QNetworkReply*)), (QObject*)this, SLOT(callbackProduct(QNetworkReply*)));
        delete m_nam_product;
    }
}

bool SearchEntryManager::load() {
    QString inikey = "global/search_entry_list";

    if (m_ini->empty(inikey)) {
        m_log->err("[global]search_entry_listの値が空です");
        return false;
    }
    QStringList list = m_ini->explode(inikey, ",");

    for (QStringList::iterator it = list.begin(); it != list.end(); it++) {
        SearchEntry* se = new SearchEntry(*it);
        se->init(m_ini, m_csv);
        m_map[*it] = se;
    }

    return true;
}

int SearchEntryManager::getMaxLinkNum() {
    if (m_max_link_num >= 0) {
        return m_max_link_num;
    }

    for (QMap<QString, SearchEntry*>::iterator it = m_map.begin(); it != m_map.end(); ++it) {
        SearchEntry *se = it.value();
        if (m_max_link_num < se->m_links.size()) {
            m_max_link_num = se->m_links.size();
        }
    }
    return qMax(m_max_link_num, 0);
}

int SearchEntryManager::getMaxParsedProductWordNum() {
    if (m_max_parsed_product_num >= 0) {
        return m_max_parsed_product_num;
    }

    for (QMap<QString, SearchEntry*>::iterator it = m_map.begin(); it != m_map.end(); ++it) {
        SearchEntry *se = it.value();
        if (m_max_parsed_product_num < se->m_regex_product_page_word.size()) {
            m_max_parsed_product_num = se->m_regex_product_page_word.size();
        }
    }
    return qMax(m_max_parsed_product_num, 0);
}

bool SearchEntryManager::searchProductCode(QString& product_code) {
    for (QMap<QString, SearchEntry*>::iterator it = m_map.begin(); it != m_map.end(); ++it) {
        SearchEntry *se = it.value();
        if (!se->m_enable_search_product_code)  {
            continue;
        }

        // 検索URL組み立て
        se->buildSearchUrl();
        if (se->m_search_url.isEmpty()) {
            continue;
        }

        // 検索(同期通信)
        if (!requestHtmlSync(se->m_search_url, NULL, se->m_html)) {
            m_log->info(se->m_name + tr("商品番号検索失敗"));
            continue;
        }
        if (!se->buildSearchResultProductCode()) {
            m_log->info(se->m_name + tr("商品番号取得失敗"));
            continue;
        }
        // ついでに検索結果のビルドも行う
        se->buildSearchResult();

        m_log->info(tr("商品番号取得成功 ") + se->m_product_code);
        product_code = se->m_product_code;
        return true;
    }

    return false;
}

void SearchEntryManager::setProductCode(const QString& product_code) {
    for (QMap<QString, SearchEntry*>::iterator it = m_map.begin(); it != m_map.end(); ++it) {
        SearchEntry *se = it.value();
        se->m_product_code = product_code;
    }
}

bool SearchEntryManager::search(QObject* obj, const char* slot) {
    QObject::disconnect((QObject*)this, SIGNAL(signalSearched()), obj, slot);
    QObject::connect((QObject*)this, SIGNAL(signalSearched()), obj, slot);

    bool do_search = false;
    m_search_entry_network_reply_map.clear();
    for (QMap<QString, SearchEntry*>::iterator it = m_map.begin(); it != m_map.end(); ++it) {
        SearchEntry *se = it.value();

        // 検索済みならスルー
        if (se->isStatusSearchbuilt()) {
            continue;
        }

        // 無効サイトはスルー
        if (!se->m_enable || se->m_search_url_template.isEmpty()) {
            se->setStatusNosearch();
            continue;
        }

        // 検索URL組み立て
        se->buildSearchUrl();
        if (se->m_search_url.isEmpty()) {
            // URL空の場合検索はしない
            se->buildSearchResult();
            continue;
        }

        se->setStatusSearching();
        do_search = true;
    }

    if(!do_search) {
        // 1件も検索実行しない場合
        m_log->info("検索対象なし");
        emit signalSearched();
        return false;
    }

    for (QMap<QString, SearchEntry*>::iterator it = m_map.begin(); it != m_map.end(); ++it) {
        SearchEntry *se = it.value();
        if (se->isStatusNosearch()) {
            continue;
        }

        // 検索（非同期)
        m_search_entry_network_reply_map.insert(m_nam_search->get(QNetworkRequest(se->m_search_url)), se);
    }

    return true;
}

void SearchEntryManager::callbackSearch(QNetworkReply *reply) {
    try {
        SearchEntry *se = m_search_entry_network_reply_map[reply];

        // 結果判定とリダイレクト
        if (reply->error() == QNetworkReply::NoError && requestHtmlSync(QUrl(), reply, se->m_html)) {
            se->setStatusSearched();
            m_log->debug(se->m_name + tr("検索成功"));
        } else {
            se->setStatusSearcherror();
            m_log->info(se->m_name + tr("検索失敗"));
        }

        // 取得したhtmlからオブジェクトのメンバ変数に展開
        se->buildSearchResult();

        if (!containsStatus(SearchEntry::Status::SEARCHING)) {
            m_log->info("すべての検索完了");
            emit signalSearched();
        }
    } catch(...) {
        SearchEntry *se = m_search_entry_network_reply_map[reply];
        se->setStatusSearcherror();
        m_log->info(se->m_name + tr("検索中に異常が発生しました"));
        if (!containsStatus(SearchEntry::Status::SEARCHING)) {
            m_log->info("すべての検索完了");
            emit signalSearched();
        }
    }
    return;
}

bool SearchEntryManager::parseProduct(QObject* obj, const char* slot) {
    QObject::disconnect((QObject*)this, SIGNAL(signalParsedProduct()), obj, slot);
    QObject::connect((QObject*)this, SIGNAL(signalParsedProduct()), obj, slot);

    bool do_request_product = false;
    m_search_entry_network_reply_map.clear();
    for (QMap<QString, SearchEntry*>::iterator it = m_map.begin(); it != m_map.end(); ++it) {
        SearchEntry *se = it.value();

        // 商品詳細ページパース対象でないならスルー
        if (!se->m_enable_parse_product) {
            continue;
        }

        // 商品ページURLが空ならスルー
        if (se->m_product_url.isEmpty()) {
            continue;
        }

        // 商品ページリクエスト（非同期)
        m_search_entry_network_reply_map.insert(m_nam_product->get(QNetworkRequest(se->m_product_url)), se);

        se->setStatusParsingproduct();
        do_request_product = true;
    }

    if(!do_request_product) {
        // 1件も商品ページをパースしない場合
        m_log->info("商品ページパース対象なし");
        emit signalParsedProduct();
        return false;
    }

    return true;
}

void SearchEntryManager::callbackProduct(QNetworkReply *reply) {
    try {
        SearchEntry *se = m_search_entry_network_reply_map[reply];

        // 結果判定とリダイレクト
        if (reply->error() == QNetworkReply::NoError && requestHtmlSync(QUrl(), reply, se->m_product_html)) {
            m_log->debug(se->m_name + tr("商品ページパース成功"));
        } else {
            m_log->info(se->m_name + tr("商品ページパース失敗"));
        }

        // 取得したhtmlからオブジェクトのメンバ変数に展開
        se->buildParseProductResult();

        if (!containsStatus(SearchEntry::Status::PARSINGPRODUCT)) {
            m_log->info("すべての商品ページパース完了");
            emit signalParsedProduct();
        }
    } catch(...) {
        SearchEntry *se = m_search_entry_network_reply_map[reply];
        se->setStatusSearcherror();
        m_log->info(se->m_name + tr("商品ページパース中に異常が発生しました"));
        if (!containsStatus(SearchEntry::Status::PARSINGPRODUCT)) {
            // 接続解除(多重接続防止)
            m_log->info("すべての商品ページパース完了");
            emit signalParsedProduct();
        }
    }
    return;
}

bool SearchEntryManager::containsStatus(SearchEntry::Status status) {
    for (QMap<QString, SearchEntry*>::iterator it = m_map.begin(); it != m_map.end(); ++it) {
        SearchEntry *se = it.value();
        if (se->isStatus(status)) {
            return true;
        }
    }
    return false;
}

bool SearchEntryManager::requestHtmlSync(const QUrl& url_first, QNetworkReply* reply, QString& html) {
    int redirect_cnt = 0;
    int max_redirect = 5;
    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QEventLoop event_loop;
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)), &event_loop, SLOT(quit()));
    while (true) {
        QUrl url;
        if (reply != NULL) {
            int http_status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (http_status != 301 && http_status != 302 && http_status != 303) {
                // html取得し終了
                QByteArray html_ba = reply->readAll();
                QTextCodec* codec = QTextCodec::codecForHtml(html_ba);
                html = codec->toUnicode(html_ba);
                QObject::disconnect(nam, SIGNAL(finished(QNetworkReply*)), &event_loop, SLOT(quit()));
                delete nam;
                return true;
            }
            if (redirect_cnt >= max_redirect) {
                m_log->info("リダイレクト回数オーバー");
                QObject::disconnect(nam, SIGNAL(finished(QNetworkReply*)), &event_loop, SLOT(quit()));
                delete nam;
                return false;
            }
            url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            redirect_cnt++;
        } else {
            url = url_first;
        }

        reply = nam->get(QNetworkRequest(url));
        event_loop.exec();
    }
}
