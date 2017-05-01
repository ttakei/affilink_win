#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QNetworkReply>
#include "log.h"
#include "ini.h"
#include "wwl.h"
#include "csv.h"
#include "searchentrymanager.h"
#include "layout.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    bool load();

    Log m_log;
    Ini m_ini;
    Wwl m_wwl;
    Csv m_csv;
    SearchEntryManager* m_search;
    Layout* m_layout;
    QString m_here;

protected:
    bool loadSearchEntry();
    void initSize();
    bool outResult();
    bool writeFile(const QString&, const QString&);

public slots:
    void callbackSubmit();
    void callbackSearch();
    void callbackParseProduct();
};

#endif
