#include "ini.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QStringList>
#include <QTextCodec>

Ini::Ini(){}
Ini::Ini(Log *log):
m_log(log) {}

bool Ini::load(const QString& inifile_path) {
    // ファイルが存在しない場合は作成
    QFile inifile(inifile_path);
    if (!inifile.exists()) {
        m_log->warn("設定ファイルが存在しません設定ファイルを復元します", "", true);
        if (!restoreFile(inifile_path)) {
            m_log->err("設定ファイルの復元に失敗しました");
            return false;
        }
    }

    // iniファイル
    m_settings = new QSettings(inifile_path, QSettings::IniFormat);
    int status = m_settings->status();
    if (m_settings->status() == QSettings::AccessError) {
        m_log->err("設定ファイルが読み込めません");
        return false;
    } else if (m_settings->status() == QSettings::FormatError) {
        m_log->err("設定ファイルの形式が不正です");
        return false;
    } else if (m_settings->status() != QSettings::NoError) {
        m_log->err("設定ファイル読み込み時に不明なエラーが発生しました");
        return false;
    }
    m_settings->setIniCodec("UTF-8");

    // ログレベル
    m_log->setLogLevel(get("global/loglevel").toString());
    return true;
}

QVariant Ini::get(const QString& key, const QVariant& value_def) {
    return m_settings->value(key, value_def);
}

void Ini::begingroup(const QString& group) {
    m_settings->beginGroup(group);
}

void Ini::endgroup() {
    m_settings->endGroup();
}

void Ini::set(const QString& key, const QString& val) {
    m_settings->setValue(key, val);
}

void Ini::set(const QString& group, const QString& key, const QString& val) {
    m_settings->beginGroup(group);
    m_settings->setValue(key, val);
    m_settings->endGroup();
}

bool Ini::contains(const QString &key) {
    return m_settings->contains(key);
}

bool Ini::empty(const QString &key) {
    return m_settings->value(key, "").toString().isEmpty();
}

QStringList Ini::explode(const QString &key, const QString &delimiter) {
    QString list_raw = get(key, "").toString();
    return list_raw.split(delimiter);
}

bool Ini::restoreFile(const QString &inifile_path) {
    QFile inifile(inifile_path);
    QDir inidir = QFileInfo(inifile).absoluteDir();
    QString dirpath = inidir.absolutePath();
    if (!inidir.exists()){
        if (!inidir.mkpath(dirpath)) {
            m_log->info(dirpath + QString("作成失敗"));
            return false;
        }
    }

    if (!inifile.open(QIODevice::WriteOnly | QIODevice::Text)){
        m_log->info(inifile_path + QString("設定ファイル作成失敗"));
        return false;
    }
    QTextStream out(&inifile);
    out.setCodec(QTextCodec::codecForName("UTF-8"));
    QString inibody = getDefault();
    out << inibody;
    inifile.close();

    return true;
}

QString Ini::getDefault() {
    QString ini = QString::fromUtf8(R"###EOF###([global]
output_folder=
search_entry_list="rakuten_books,seven_net,amiami_rakuten,bigcamera,furu1_online,tower_record,tsutaya,nojima_online,hapinet_online,toizarasu"
force_output=true

[_search_entry_rakuten_books]
name=楽天ブックス
search_url_template="http://search.books.rakuten.co.jp/bksearch/nm?b=1&g=000&sitem=:jancode"
regex_product_url="<div class=\"rbcomp__item-list__item__image\">[\\r\\n\\s]*<a href=\"([^\"]*)\""
regex_product_code="<div class=\"rbcomp__item-list__item__image\">[\\r\\n\\s]*<a href=\"http://books.rakuten.co.jp/rb/([^/]*)/\""
regex_product_img_url="<div class=\"rbcomp__item-list__item__image\">[\\r\\n\\s]*<a href=\"[^\"]*\"[^>]*><img src=\"([^\"]*)\""
regex_product_price="<span class=\"rbcomp__line-through\">([0-9,]*)"
regex_product_date="<p class=\"rbcomp__item-list__item__subtext\">[\\r\\n\\s]*([0-9]+年[0-9]+月[0-9]+日)"
regex_product_page_word="【発売前】"
regex_product_page_word2="送料無料"
regex_product_page_desc="<h1 itemprop=\"name\">([^<]*)<"
link_template="<a href=\"http://hb.afl.rakuten.co.jp/hgc/129cd0f9.bb4a99f6.129cd0fa.b7e93432/?pc=:product_eurl&scid=af_item_txt&link_type=text&ut=eyJwYWdlIjoiaXRlbSIsInR5cGUiOiJ0ZXh0Iiwic2l6ZSI6IjQwMHg0MDAiLCJuYW0iOjEsIm5hbXAiOiJkb3duIiwiY29tIjoxLCJjb21wIjoibGVmdCIsInByaWNlIjoxLCJib3IiOjEsImNvbCI6MH0%3D\" target=\"_blank\" style=\"word-wrap:break-word;\" >:product_name</a>"
link_template2="<a href=\"http://hb.afl.rakuten.co.jp/hgc/129cd0f9.bb4a99f6.129cd0fa.b7e93432/?pc=:product_eurl&scid=af_item_txt&link_type=text&ut=eyJwYWdlIjoiaXRlbSIsInR5cGUiOiJ0ZXh0Iiwic2l6ZSI6IjQwMHg0MDAiLCJuYW0iOjEsIm5hbXAiOiJkb3duIiwiY29tIjoxLCJjb21wIjoibGVmdCIsInByaWNlIjoxLCJib3IiOjEsImNvbCI6MH0%3D\" target=\"_blank\" style=\"word-wrap:break-word;\" >:product_name</a>"
enable=true

[_search_entry_seven_net]
name=セブンネット
search_url_template="http://7net.omni7.jp/search/?site=default_collection&client=facet_frontend&access=p&ctgySelect=all&fromKeywordSearch=true&oop=on&prvlg=off&narrowing=all&occupation=all&keyword=:jancode"
regex_product_url="<p class=\"productImg\"><a href=\"([^\"]*)\""
regex_product_img_url="<p class=\"productImg\">[^<]*<a[^>]*>[^<]*<img src=\"([^\"]*)\""
regex_product_price="<p class=\"productPrice\">[^<]*<span class=\"productPriceName\">[^<]*</span>[^<]*<b>[^0-9]*([0-9,]*)"
regex_product_date=
prefix_product_url=http:
prefix_product_img_url=http:
link_template="<a href=\"http://ck.jp.ap.valuecommerce.com/servlet/referral?sid=3128241&pid=882662476&vc_url=:product_eurl\" target=\"_blank\" ><img src=\"http://ad.jp.ap.valuecommerce.com/servlet/gifbanner?sid=3128241&pid=882662476\" height=\"1\" width=\"0\" border=\"0\">:product_name</a>"
enable=true

[_search_entry_amiami_rakuten]
name=あみあみ(楽天)
search_url_template="http://search.rakuten.co.jp/search/inshop-mall??f=1&v=2&sid=201619&uwd=1&s=1&p=1&st=A&sitem=:jancode"
regex_product_url="<!-- search result list -->[^<]*<div[^>]*>[^<]*<table[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*</tr[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<div[^>]*>[^<]*<a href=\"([^\"]*)"
regex_product_img_url="<!-- search result list -->[^<]*<div[^>]*>[^<]*<table[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*</tr[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<div[^>]*>[^<]*<a[^>]*>[^<]*<img alt=\"[^\"]*\" border=\"[^\"]*\" src=\"([^\"]*)\""
regex_product_no="<!-- search result list -->[^<]*<div[^>]*>[^<]*<table[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*</tr[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<div[^>]*>[^<]*<a href=\"http://item.rakuten.co.jp/amiami/([^\"/]*)"
regex_product_price="([0-9,]*) 円"
link_template="<a href=\"http://px.a8.net/svt/ejp?a8mat=2BNX0Q+79DBCI+NA2+BW8O2&a8ejpredirect=http%3A%2F%2Fwww.amiami.jp%2Ftop%2Fdetail%2Fdetail%3Fscode%3D:product_no%26page%3Dtop%26affid%3Dfc\" target=\"_blank\">:product_name</a><img border=\"0\" width=\"1\" height=\"1\" src=\"http://www12.a8.net/0.gif?a8mat=2BNX0Q+79DBCI+NA2+BW8O2\" alt=\"\"><br/><br/>各支店の商品ページはこちら<br/><a href=\"http://ck.jp.ap.valuecommerce.com/servlet/referral?sid=3128241&pid=882646042&vc_url=http%3A%2F%2Fstore.shopping.yahoo.co.jp%2Famiami%2F:product_no.html\" target=\"_blank\" ><img src=\"http://ad.jp.ap.valuecommerce.com/servlet/gifbanner?sid=3128241&pid=882646042\" height=\"1\" width=\"0\" border=\"0\">あみあみYahoo!ショッピング店</a>｜<a href=\"http://hb.afl.rakuten.co.jp/hgc/129b0470.3d7d766c.129b0471.e0f96c97/?pc=:product_eurl%3fscid%3daf_link_txt&amp;m=:product_eurl\" target=\"_blank\">あみあみ楽天市場</a><br/>"
enable=true

[_search_entry_bigcamera]
name=ビックカメラ
search_url_template="http://www.biccamera.com/bc/disp/CSfDispListPage_001.jsp?q=:jancode"
regex_product_url="<a class=\"cssopa\" href=\"([^\"]*)\">"
regex_product_img_url="<a class=\"cssopa\" href=\"[^\"]*\">[^<]*<img src=\"([^\"]*)\""
regex_product_price="税込：([0-9,]*)円"
link_template="<a href=\"http://click.linksynergy.com/fs-bin/click?id=LPuibgunRpQ&subid=&offerid=192116.1&type=10&tmpid=864&RD_PARM1=:product_eurl\" target=\"new\">:product_name</a><img alt=\"icon\" width=\"1\" height=\"1\" src=\"http://ad.linksynergy.com/fs-bin/show?id=LPuibgunRpQ&bids=192116.1&type=10\">"
link_template2="<a href=\"http://linksynergy.jrs5.com/fs-bin/click?id=LPuibgunRpQ&subid=&offerid=252693.1&type=10&tmpid=9823&RD_PARM1=:product_eurl\">:product_name</a><img alt=\"icon\" width=\"1\" height=\"1\" src=\"http://ad.linksynergy.com/fs-bin/show?id=LPuibgunRpQ&bids=252693.1&type=10\">"
enable=true

[_search_entry_furu1_online]
name=古市オンライン
search_url_template="http://www.furu1online.net/SearchItem?CID=&FREE_WORD=:jancode&PROP_ID_9200=2"
regex_product_url="<h3 class=\"geb_itemName\"><a href=\"([^\"]*)\""
regex_product_img_url="<p class=\"thumbnail\">[^<]*<a[^>]*><img src=\"([^\"]*)\""
regex_product_price="販売価格 <strong>([0-9,]*)</strong>"
prefix_product_img_url=http://www.furu1online.net
link_template="<a href=\"http://click.linksynergy.com/fs-bin/click?id=LPuibgunRpQ&subid=&offerid=192116.1&type=10&tmpid=864&RD_PARM1=:product_eurl\" target=\"new\">:product_name</a><img alt=\"icon\" width=\"1\" height=\"1\" src=\"http://ad.linksynergy.com/fs-bin/show?id=LPuibgunRpQ&bids=192116.1&type=10\">"
enable=true

[_search_entry_tower_record]
name=タワーレコード
search_url_template=http://tower.jp/search/item/:jancode
regex_product_url="<div class=\"discographyTableImg01\">[^<]*<p>[^<]*<!--[^>]*>[^<]*<a href=\"([^\"]*)"
regex_product_img_url="<div class=\"discographyTableImg01\">[^<]*<p>[^<]*<!--[^>]*>[^<]*<a[^>]*>[^<]*<!--[^>]*>[^<]*<img src=\"([^\"]*)"
regex_product_price="<dt>価格</dt><dd[^>]*>￥([0-9,]*)"
link_template="<a href=\"http://ck.jp.ap.valuecommerce.com/servlet/referral?sid=3128241&pid=882662482&vc_url=:product_eurl%3Dpafvc\" target=\"_blank\" ><img src=\"http://ad.jp.ap.valuecommerce.com/servlet/gifbanner?sid=3128241&pid=882662482\" height=\"1\" width=\"0\" border=\"0\">:product_name</a>"
enable=true

[_search_entry_tsutaya]
name=TSUTAYA
search_url_template="http://shop.tsutaya.co.jp/search_result?ecCategory=00&searchType=1&pSCS=%E3%81%99%E3%81%B9%E3%81%A6%E3%81%AE%E5%95%86%E5%93%81&searchKeyword=:jancode&x=62&y=5"
regex_product_url="<td class=\"tolImg\">[^<]*<a href=\"([^\"]*)"
regex_product_img_url="<td class=\"tolImg\">[^<]*<a href=\"[^\"]*\"><img title=\"[^\"]*\" style=\"[^\"]*\" alt=\"[^\"]*\" src=\"([^\"]*)"
regex_product_price="<span class=\"tolNote\"><em>[^0-9,]*([0-9,]*)円"
prefix_product_url=http://shop.tsutaya.co.jp/
link_template="<a href=\"http://ck.jp.ap.valuecommerce.com/servlet/referral?sid=3128241&pid=882672755&vc_url=:product_eurl\" target=\"_blank\" ><img src=\"http://ad.jp.ap.valuecommerce.com/servlet/gifbanner?sid=3128241&pid=882672755\" height=\"1\" width=\"0\" border=\"0\">:product_name</a>"
enable=true

[_search_entry_nojima_online]
name=NojimaOnline
search_url_template="http://online.nojima.co.jp/app/catalog/list/init?searchCategoryCode=0&searchMethod=0&searchWord=:jancode"
regex_product_url="<div class=\"cmdty_iteminfo\">[^<]*<div>[^<]*<a href=\"([^\"]*)"
regex_product_img_url="<div class=\"Cmdty_ThumbImg_Cell\">[^<]*<a[^>]*>[^<]*<div[^>]*>[^<]*<img src=\"([^\"]*)"
regex_product_price="<span id=\"praiceh\"></span><span class=\"price\">[^0-9,]*([0-9,]*)"
prefix_product_url=http://online.nojima.co.jp
prefix_product_img_url=http://online.nojima.co.jp
link_template="<a href=\"http://ck.jp.ap.valuecommerce.com/servlet/referral?sid=3128241&pid=882672760&vc_url=:product_eurl\" target=\"_blank\" ><img src=\"http://ad.jp.ap.valuecommerce.com/servlet/gifbanner?sid=3128241&pid=882672760\" height=\"1\" width=\"0\" border=\"0\">:product_name</a>"
enable=true

[_search_entry_hapinet_online]
name=ハピネットオンライン
search_url_template="http://www.happinetonline.com/NASApp/mnas/MxMProduct?Action=prd_detail&KIND=0&SHOP_ID=1&PRODUCT_ID=:jancode&DISPCATEGORY="
regex_product_url=
regex_product_img_url="<!--メイン画像ここから-->[^<]*<a[^>]*>[^<]*<img src=\"([^\"]*)"
regex_product_price="<p id=\"price_a02\">[^<]*<span id=\"ss01\">販売価格 :</span>[^<]*<span id=\"ss02\">([0-9,]*)円"
prefix_product_url="http://www.happinetonline.com/NASApp/mnas/MxMProduct?Action=prd_detail&KIND=0&SHOP_ID=1&PRODUCT_ID=:jancode&DISPCATEGORY="
prefix_product_img_url=http://www.happinetonline.com
link_template="<a href=\"http://click.linksynergy.com/fs-bin/click?id=LPuibgunRpQ&subid=&offerid=212933.1&type=10&tmpid=1969&RD_PARM1=:product_eurl\">:product_name</a><img alt=\"icon\" width=\"1\" height=\"1\" src=\"http://ad.linksynergy.com/fs-bin/show?id=LPuibgunRpQ&bids=212933.1&type=10\">"
enable=true

[_search_entry_toizarasu]
name=トイザらス
search_url_template="http://www.toysrus.co.jp/disp/CSfDispListPage_001.jsp?dispNo=&q=:jancode"
regex_product_url=
regex_product_img_url="<p id=\"detail-item-thumb-01\" class=\"detail-item-thumb\">[^<]*<a[^>]*>[^<]*<img onerror=\"[^\"]*\" src=\"([^\"]*)"
regex_product_price="<i class=\"inTax\">([0-9,]*)円"
prefix_product_url="http://www.toysrus.co.jp/disp/CSfDispListPage_001.jsp?dispNo=&q=:jancode"
prefix_product_img_url=http://www.toysrus.co.jp/disp/
link_template="<a href=\"http://click.linksynergy.com/fs-bin/click?id=LPuibgunRpQ&subid=&offerid=340343.1&type=10&tmpid=1148&RD_PARM1=http%253A%252F%252Fwww.toysrus.co.jp%252Fdisp%252FCSfDispListPage_001.jsp%253Fq%253D:jancode\">:product_name</a>"
enable=true
)###EOF###");
    return ini;
}
