#ifndef SEARCHENTRYCONTAINER_H
#define SEARCHENTRYCONTAINER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QNetworkReply>
#include "searchentry.h"
#include "log.h"
#include "ini.h"

class SearchEntryManager : public QObject
{
    Q_OBJECT

public:
    SearchEntryManager();
    SearchEntryManager(Log*, Ini*);
    bool load();
    void search(QObject*, void(*)());

    QMap<QString, SearchEntry*> m_map;

protected:
    Log *m_log;
    Ini *m_ini;
    QMap<QNetworkReply*, QString> m_search_entry_network_reply_map;

signals:
    void signalSearched();

protected slots:
    void callbackSearch(QNetworkReply*);
};

#endif // SEARCHENTRYCONTAINER_H
