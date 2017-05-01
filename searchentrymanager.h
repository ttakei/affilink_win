#ifndef SEARCHENTRYCONTAINER_H
#define SEARCHENTRYCONTAINER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QNetworkReply>
#include "searchentry.h"
#include "log.h"
#include "ini.h"
#include "wwl.h"
#include "csv.h"

class SearchEntryManager : public QObject
{
    Q_OBJECT

public:
    explicit SearchEntryManager(QObject* = 0);
    explicit SearchEntryManager(Log*, Ini*, Wwl*, Csv*);
    ~SearchEntryManager();
    bool load();
    bool searchProductCode(QString&);
    void setProductCode(const QString&);
    bool search(QObject*, const char*);
    bool parseProduct(QObject*, const char*);
    int getMaxLinkNum();
    int getMaxParsedProductWordNum();

    QMap<QString, SearchEntry*> m_map;

protected:
    Log *m_log;
    Ini *m_ini;
    Wwl *m_wwl;
    Csv *m_csv;
    QMap<QNetworkReply*, SearchEntry*> m_search_entry_network_reply_map;
    int m_max_link_num;
    int m_max_parsed_product_num;
    QNetworkAccessManager *m_nam_search;
    QNetworkAccessManager *m_nam_product;

    bool containsStatus(SearchEntry::Status);
    bool requestHtmlSync(const QUrl&, QNetworkReply*, QString&);

signals:
    void signalSearched();
    void signalParsedProduct();

protected slots:
    void callbackSearch(QNetworkReply*);
    void callbackProduct(QNetworkReply*);
};

#endif // SEARCHENTRYCONTAINER_H
