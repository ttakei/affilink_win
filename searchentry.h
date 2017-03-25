#ifndef SEARCHENTRY_H
#define SEARCHENTRY_H

#include <QString>
#include <QPair>
#include <QVector>
#include <QSettings>
#include <QUrl>
#include <QRegExp>
#include <QFile>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#define SEARCHENTRY_ENTRY_GROUP_NAME_PREFIX "_search_entry_"

class SearchEntry
{
public:
    SearchEntry(const QString&);
    void load(QSettings*);
    void setInput(QString, QString, QString, QString, QString, QString, QString, QString);
    bool buildSearchUrl();
    bool fetch(QNetworkAccessManager*, QMap<QNetworkReply*, QString>&);
    bool extractProductUrl(const QString&);
    bool extractProductImgUrl(const QString&);
    bool extractProductNO(const QString&);
    bool extractProductPrice(const QString&);
    bool extractProductDate(const QString&);
    bool buildLink();
    bool buildWWL();
    QString getIniGroup();

    QString m_id;
    QString m_name;
    QString m_search_url_template;
    QRegExp m_regex_product_url;
    QRegExp m_regex_product_img_url;
    QRegExp m_regex_product_no;
    QRegExp m_regex_product_price;
    QRegExp m_regex_product_date;
    QString m_prefix_product_url;
    QString m_prefix_product_img_url;
    QVector<QPair<QString, QString>> m_link_templates;
    QString m_product_name;
    QString m_jancode;
    QString m_search_word;
    QString m_esearch_word;
    QString m_product_code;
    QString m_remarks1;
    QString m_remarks2;
    QString m_remarks3;
    QUrl m_search_url;
    QString m_product_url;
    QString m_product_eurl;
    QString m_product_img_url;
    QString m_product_no;
    QString m_product_price;
    QString m_product_date;
    QString m_wwl_template_file;
    QString m_wwl_template_file_default;
    QString m_wwl;
    QVector<QString> m_links;
    bool m_enable;
    bool m_ok;
    bool m_force_output;

// private:
};

#endif // SEARCHENTRY_H
