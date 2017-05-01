#include <QObject>
#include "searchentrymanagerbak.h"

SearchEntryManager::SearchEntryManager(){}
SearchEntryManager::SearchEntryManager(Log* log, Ini* ini):
m_log(log),
m_ini(ini) {}

bool SearchEntryManager::load() {
    QString inikey = "global/search_entry_list";

    if (m_ini->empty(inikey)) {
        m_log->err("[global]search_entry_listの値が空です");
        return false;
    }
    QStringList list = m_ini->explode(inikey, ",");

    for (QStringList::iterator it = list.begin(); it != list.end(); it++) {
        SearchEntry* se = new SearchEntry(*it);
        se->init(m_ini);
        m_map[*it] = se;
    }

    return true;
}

void SearchEntryManager::search(QObject* obj, void(*callback)()) {
    QObject::connect((QObject*)this, SIGNAL(signalSearched()), obj, SLOT(callback()));

    bool searched = false;
    for (QMap<QString, SearchEntry*>::iterator ite = m_map.begin(); ite != m_map.end(); ++ite) {
        QString key = ite.key();
        SearchEntry *se = ite.value();

        // 無効サイトはスルー
        if (!se->m_enable || se->m_search_url_template.isEmpty()) {
            continue;
        }

        // 検索URL組み立て
        se->buildSearchUrl();
        if (se->m_search_url.isEmpty()) {
            // URL空の場合検索はしない
            se->buildSearchResult();
            continue;
        }

        // 検索
        QNetworkAccessManager *nam = new QNetworkAccessManager((QObject*)this);
        QObject::connect(nam, SIGNAL(finished(QNetworkReply*)), (QObject*)this, SLOT(callbackSearch(QNetworkReply*)));
        m_search_entry_network_reply_map.insert(nam->get(QNetworkRequest(se->m_search_url)), se->m_id);
        searched = true;
    }

    if(!searched) {
        // 1件も検索実行していない場合
        emit signalSearched();
    }
}

void SearchEntryManager::callbackSearch(QNetworkReply *reply)
{
    try {
        emit signalSearched();
        return;
        /*
        SearchEntry *se = m_search_entry_map[m_search_entry_nam_state[reply]];
        if(reply->error() == QNetworkReply::NoError) {
            // リダイレクトチェック
            int redirct_cnt = 0;
            int max_redirct = 5;
            while (true) {
                int http_status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                if (http_status != 301 && http_status != 302 && http_status != 303) {
                    break;
                }
                if (redirect_cnt >= max_redirct) {
                    m_log->warn(QString(se->m_id) + tr("　リダイレクト回数オーバー"));
                    // 結果は作成する
                    se->buildSearchResult();
                    return;
                }
                // 同期通信で処理する
                QEventLoop event_loop;
                QUrl redirect_url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
                QNetworkAccessManager *redirect_nam = new QNetworkAccessManager();
                connect(redirect_nam, SIGNAL(finished(QNetworkReply*)), &redirect_nam, SLOT(quit()));
                reply = redirect_nam->get(QNetworkRequest(redirect_url));
                event_loop.exec();
                redirect_cnt++;
            }

            // 取得したhtmlからオブジェクトのメンバ変数に展開
            QByteArray html_ba = reply->readAll();
            QTextCodec* codec = QTextCodec::codecForHtml(html_ba);
            se->m_html = codec->toUnicode(html_ba);
            se->buildSearchResult();

            // 確認用
            QString htmlfile_path = m_here + tr("\\html\\") + se->m_name + tr(".html");
            QString htmldir_path = QFileInfo(htmlfile_path).absolutePath();
            QDir htmldir;
            if (!htmldir.exists(htmldir_path)){
                htmldir.mkdir(htmldir_path);
            }
            QFile htmlfile(htmlfile_path);
            htmlfile.open(QIODevice::WriteOnly);
            QTextCodec* codecd = QTextCodec::codecForName("UTF-8");
            QTextStream stream(&htmlfile);
            stream.setCodec(codecd);
            stream << html;
            htmlfile.close();
        }

        m_run_wait_count--;
        if (m_run_wait_count < 1) {
            m_log->info("すべての検索に失敗しました")
            outResult();
        }
        */
    } catch(...) {
        m_log->err("検索中に異常が発生しました");
        emit signalSearched();
    }
}
