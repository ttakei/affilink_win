#ifndef SEARCHENTRY_H
#define SEARCHENTRY_H

#include <QSettings>
#include <QUrl>
#include <QRegExp>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#define SEARCHENTRY_ENTRY_GROUP_NAME_PREFIX "_search_entry_"

class SearchEntry
{
public:
    SearchEntry(const QString&);
    void load(QSettings*);
    void setInput(QString, QString);
    bool buildSearchUrl();
    bool fetch(QNetworkAccessManager*, QMap<QNetworkReply*, QString>&);
    bool extractProductUrl(const QString&);
    bool extractProductImgUrl(const QString&);
    bool extractProductNO(const QString&);
    bool extractProductPrice(const QString&);
    bool extractProductDate(const QString&);
    bool buildLink();
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
    QString m_link_template;
    QString m_link_template_default;
    QString m_product_name;
    QString m_jancode;
    QUrl m_search_url;
    QString m_product_url;
    QString m_product_eurl;
    QString m_product_img_url;
    QString m_product_no;
    QString m_product_price;
    QString m_product_date;
    QString m_link;
    bool m_enable;
    bool m_ok;

// private:
};

#endif // SEARCHENTRY_H
