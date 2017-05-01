#ifndef SEARCHENTRY_H
#define SEARCHENTRY_H

#include <QString>
#include <QPair>
#include <QVector>
#include <QUrl>
#include <QRegExp>
#include <QFile>
#include "ini.h"
#include "wwl.h"
#include "csv.h"

#define SEARCHENTRY_ENTRY_GROUP_NAME_PREFIX "_search_entry_"

class SearchEntry
{
public:
    enum Status {
        INIT,
        NOSEARCH,
        SEARCHING,
        SEARCHED,
        SEARCHERROR,
        SEARCHBUILT,
        PARSINGPRODUCT,
        PARSEDPRODUCT,
        RESULTBUILT
    };

    SearchEntry(const QString&);
    void init(Ini*, Csv*);
    QString getIniGroup();
    void buildSearchUrl();
    bool buildSearchResultProductCode();
    void buildSearchResult();
    void buildParseProductResult();
    void buildResult(Wwl*);
    void setStatusNosearch();
    void setStatusSearching();
    void setStatusSearched();
    void setStatusSearcherror();
    void setStatusSearchbuilt();
    void setStatusParsingproduct();
    void setStatusParsedproduct();
    void setStatusResultbuilt();
    bool isStatus(Status);
    bool isStatusSearching();
    bool isStatusSearched();
    bool isStatusNosearch();
    bool isStatusSearchbuilt();
    bool isStatusParsedproduct();
    bool isStatusResultbuilt();

    // 設定
    bool m_force_output;
    QString m_id;
    QString m_name;
    QString m_search_url_template;
    QRegExp m_regex_product_code;
    QRegExp m_regex_product_url;
    QRegExp m_regex_product_img_url;
    QRegExp m_regex_product_no;
    QRegExp m_regex_product_price;
    QRegExp m_regex_product_date;
    QVector<QRegExp> m_regex_product_page_word;
    QRegExp m_regex_product_page_desc;
    QString m_prefix_product_url;
    QString m_prefix_product_img_url;
    bool m_enable;
    bool m_enable_search_product_code;
    bool m_enable_parse_product;

    // 入力
    QString m_product_name;
    QString m_jancode;
    QString m_search_word;
    QString m_search_eword;
    QString m_product_code;
    QString m_remarks1;
    QString m_remarks2;
    QString m_remarks3;
    QString m_remarks4;
    QString m_remarks5;

    // 出力
    QUrl m_search_url;
    QString m_html;
    QString m_product_url;
    QString m_product_eurl;
    QString m_product_img_url;
    QString m_product_img_eurl;
    QString m_product_no;
    QString m_product_price;
    QString m_product_date;
    QString m_product_html;
    QVector<bool> m_product_page_word;
    QString m_product_page_desc;
    QString m_wwl;
    QVector<QString> m_links;

protected:
    void buildProductUrl();
    void buildProductImgUrl();
    void buildProductNO();
    void buildProductPrice();
    void buildProductDate();
    void buildProductPageWord();
    void buildProductPageDesc();
    bool buildWWL(Wwl*);
    void bind(QString&);

    int m_status;
    Csv* m_csv;
};

#endif // SEARCHENTRY_H
