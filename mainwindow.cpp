/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include <QtWidgets>
#include <QtNetwork/QNetworkAccessManager>
#include <QMessageBox>
#include <QFile>
#include "searchentry.h"

MainWindow::MainWindow()
{
    m_here = QDir(QDir::current()).canonicalPath().replace("/", "\\");
    QMessageBox msg(this);

    // 設定読み込み
    QString inifile_path = m_here + tr("\\conf\\default.ini");
    if (!loadIni(inifile_path)) {
        msg.setText("設定ファイルの読み込みに失敗しました");
        msg.exec();
        return;
    }

    // レイアウト
    QWidget *widget = new QWidget;
    setCentralWidget(widget);
    QHBoxLayout *layout = new QHBoxLayout;
    // - 検索オプション
    addSearchOptionLayout(layout);
    // - 右側エリア
    addRightLayout(layout);
    // - セット
    widget->setLayout(layout);
    setWindowTitle(tr("リンク作成ツール"));
    setMinimumSize(480, 320);
    resize(480, 346);
}

bool MainWindow::loadIni(const QString& inifile_path) {
    // 設定用インスタンス
    m_settings = new QSettings(inifile_path, QSettings::IniFormat, this);
    m_settings->setIniCodec("UTF-8");
    QString search_entry_list_raw = m_settings->value(tr("global/search_entry_list")).toString();

    // 読み込みに失敗した場合は復元を試みる
    if (search_entry_list_raw.isEmpty()) {
        QMessageBox msg(this);
        QFile inifile(inifile_path);
        if (!inifile.exists()) {
            msg.setText("設定ファイルが存在しません。設定ファイルを復元します");
            msg.exec();

            QString inidir_path = QFileInfo(inifile_path).absolutePath();
            QDir inidir;
            if (!inidir.exists(inidir_path)){
                inidir.mkdir(inidir_path);
            }

            if (!inifile.open(QIODevice::WriteOnly | QIODevice::Text)){
                msg.setText("設定ファイルの復元に失敗しました");
                msg.exec();
                return false;
            }
            QTextStream out(&inifile);
            out.setCodec(QTextCodec::codecForName("UTF-8"));
            QString inibody("");
            inibody += QString::fromUtf8("[global]\n");
            inibody += QString::fromUtf8("output_folder=\n");
            inibody += QString::fromUtf8("search_entry_list=\"rakuten_books,seven_net,amiami_rakuten,bigcamera,furu1_online,tower_record,tsutaya,nojima_online,hapinet_online,toizarasu\"\n");
            inibody += QString::fromUtf8("\n");
            inibody += QString::fromUtf8("[_search_entry_rakuten_books]\n");
            inibody += QString::fromUtf8("name=楽天ブックス\n");
            inibody += QString::fromUtf8("search_url_template=\"http://search.books.rakuten.co.jp/bksearch/nm?b=1&g=000&sitem=:jancode\"\n");
            inibody += QString::fromUtf8("regex_product_url=\"<div class=\\\"rbcomp__item-list__item__image\\\">[\\\\r\\\\n\\\\s]*<a href=\\\"([^\\\"]*)\\\"\"\n");
            inibody += QString::fromUtf8("regex_product_img_url=\"<div class=\\\"rbcomp__item-list__item__image\\\">[\\\\r\\\\n\\\\s]*<a href=\\\"[^\\\"]*\\\"[^>]*><img src=\\\"([^\\\"]*)\\\"\"\n");
            inibody += QString::fromUtf8("regex_product_price=\"<span class=\\\"rbcomp__line-through\\\">([0-9,]*)\"\n");
            inibody += QString::fromUtf8("regex_product_date=\"<p class=\\\"rbcomp__item-list__item__subtext\\\">[\\\\r\\\\n\\\\s]*([0-9]+年[0-9]+月[0-9]+日)\"\n");
            inibody += QString::fromUtf8("link_template=\"<a href=\\\"http://hb.afl.rakuten.co.jp/hgc/129cd0f9.bb4a99f6.129cd0fa.b7e93432/?pc=:product_eurl&scid=af_item_txt&link_type=text&ut=eyJwYWdlIjoiaXRlbSIsInR5cGUiOiJ0ZXh0Iiwic2l6ZSI6IjQwMHg0MDAiLCJuYW0iOjEsIm5hbXAiOiJkb3duIiwiY29tIjoxLCJjb21wIjoibGVmdCIsInByaWNlIjoxLCJib3IiOjEsImNvbCI6MH0%3D\\\" target=\\\"_blank\\\" style=\\\"word-wrap:break-word;\\\" >:product_name</a>\"\n");
            inibody += QString::fromUtf8("enable=true\n");
            inibody += QString::fromUtf8("\n");
            inibody += QString::fromUtf8("[_search_entry_seven_net]\n");
            inibody += QString::fromUtf8("name=セブンネット\n");
            inibody += QString::fromUtf8("search_url_template=\"http://7net.omni7.jp/search/?site=default_collection&client=facet_frontend&access=p&ctgySelect=all&fromKeywordSearch=true&oop=on&prvlg=off&narrowing=all&occupation=all&keyword=:jancode\"\n");
            inibody += QString::fromUtf8("regex_product_url=\"<p class=\\\"productImg\\\"><a href=\\\"([^\\\"]*)\\\"\"\n");
            inibody += QString::fromUtf8("regex_product_img_url=\"<p class=\\\"productImg\\\">[^<]*<a[^>]*>[^<]*<img src=\\\"([^\\\"]*)\\\"\"\n");
            inibody += QString::fromUtf8("regex_product_price=\"<p class=\\\"productPrice\\\">[^<]*<span class=\\\"productPriceName\\\">[^<]*</span>[^<]*<b>[^0-9]*([0-9,]*)\"\n");
            inibody += QString::fromUtf8("regex_product_date=\n");
            inibody += QString::fromUtf8("prefix_product_url=http:\n");
            inibody += QString::fromUtf8("prefix_product_img_url=http:\n");
            inibody += QString::fromUtf8("link_template=\"<a href=\\\"http://ck.jp.ap.valuecommerce.com/servlet/referral?sid=3128241&pid=882662476&vc_url=:product_eurl\\\" target=\\\"_blank\\\" ><img src=\\\"http://ad.jp.ap.valuecommerce.com/servlet/gifbanner?sid=3128241&pid=882662476\\\" height=\\\"1\\\" width=\\\"0\\\" border=\\\"0\\\">:product_name</a>\"\n");
            inibody += QString::fromUtf8("enable=true\n");
            inibody += QString::fromUtf8("\n");
            inibody += QString::fromUtf8("[_search_entry_amiami_rakuten]\n");
            inibody += QString::fromUtf8("name=あみあみ(楽天)\n");
            inibody += QString::fromUtf8("search_url_template=\"http://search.rakuten.co.jp/search/inshop-mall??f=1&v=2&sid=201619&uwd=1&s=1&p=1&st=A&sitem=:jancode\"\n");
            inibody += QString::fromUtf8("regex_product_url=\"<!-- search result list -->[^<]*<div[^>]*>[^<]*<table[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*</tr[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<div[^>]*>[^<]*<a href=\\\"([^\\\"]*)\"\n");
            inibody += QString::fromUtf8("regex_product_img_url=\"<!-- search result list -->[^<]*<div[^>]*>[^<]*<table[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*</tr[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<div[^>]*>[^<]*<a[^>]*>[^<]*<img alt=\\\"[^\\\"]*\\\" border=\\\"[^\\\"]*\\\" src=\\\"([^\\\"]*)\\\"\"\n");
            inibody += QString::fromUtf8("regex_product_no=\"<!-- search result list -->[^<]*<div[^>]*>[^<]*<table[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*<td[^>]*>[^<]*<font[^>]*>[^<]*</font[^>]*>[^<]*</td[^>]*>[^<]*</tr[^>]*>[^<]*<tr[^>]*>[^<]*<td[^>]*>[^<]*<div[^>]*>[^<]*<a href=\\\"http://item.rakuten.co.jp/amiami/([^\\\"/]*)\"\n");
            inibody += QString::fromUtf8("regex_product_price=\"([0-9,]*) 円\"\n");
            inibody += QString::fromUtf8("link_template=\"<a href=\\\"http://px.a8.net/svt/ejp?a8mat=2BNX0Q+79DBCI+NA2+BW8O2&a8ejpredirect=http%3A%2F%2Fwww.amiami.jp%2Ftop%2Fdetail%2Fdetail%3Fscode%3D:product_no%26page%3Dtop%26affid%3Dfc\\\" target=\\\"_blank\\\">:product_name</a><img border=\\\"0\\\" width=\\\"1\\\" height=\\\"1\\\" src=\\\"http://www12.a8.net/0.gif?a8mat=2BNX0Q+79DBCI+NA2+BW8O2\\\" alt=\\\"\\\"><br/><br/>各支店の商品ページはこちら<br/><a href=\\\"http://ck.jp.ap.valuecommerce.com/servlet/referral?sid=3128241&pid=882646042&vc_url=http%3A%2F%2Fstore.shopping.yahoo.co.jp%2Famiami%2F:product_no.html\\\" target=\\\"_blank\\\" ><img src=\\\"http://ad.jp.ap.valuecommerce.com/servlet/gifbanner?sid=3128241&pid=882646042\\\" height=\\\"1\\\" width=\\\"0\\\" border=\\\"0\\\">あみあみYahoo!ショッピング店</a>｜<a href=\\\"http://hb.afl.rakuten.co.jp/hgc/129b0470.3d7d766c.129b0471.e0f96c97/?pc=:product_eurl%3fscid%3daf_link_txt&amp;m=:product_eurl\\\" target=\\\"_blank\\\">あみあみ楽天市場</a><br/>\"\n");
            inibody += QString::fromUtf8("enable=true\n");
            inibody += QString::fromUtf8("\n");
            inibody += QString::fromUtf8("[_search_entry_bigcamera]\n");
            inibody += QString::fromUtf8("name=ビックカメラ\n");
            inibody += QString::fromUtf8("search_url_template=\"http://www.biccamera.com/bc/disp/CSfDispListPage_001.jsp?q=:jancode\"\n");
            inibody += QString::fromUtf8("regex_product_url=\"<a class=\\\"cssopa\\\" href=\\\"([^\\\"]*)\\\">\"\n");
            inibody += QString::fromUtf8("regex_product_img_url=\"<a class=\\\"cssopa\\\" href=\\\"[^\\\"]*\\\">[^<]*<img src=\\\"([^\\\"]*)\\\"\"\n");
            inibody += QString::fromUtf8("regex_product_price=\"税込：([0-9,]*)円\"\n");
            inibody += QString::fromUtf8("link_template=\"<a href=\\\"http://click.linksynergy.com/fs-bin/click?id=LPuibgunRpQ&subid=&offerid=192116.1&type=10&tmpid=864&RD_PARM1=:product_eurl\\\" target=\\\"new\\\">:product_name</a><img alt=\\\"icon\\\" width=\\\"1\\\" height=\\\"1\\\" src=\\\"http://ad.linksynergy.com/fs-bin/show?id=LPuibgunRpQ&bids=192116.1&type=10\\\">\"\n");
            inibody += QString::fromUtf8("link_template=\"<a href=\\\"http://linksynergy.jrs5.com/fs-bin/click?id=LPuibgunRpQ&subid=&offerid=252693.1&type=10&tmpid=9823&RD_PARM1=:product_eurl\\\">:product_name</a><img alt=\\\"icon\\\" width=\\\"1\\\" height=\\\"1\\\" src=\\\"http://ad.linksynergy.com/fs-bin/show?id=LPuibgunRpQ&bids=252693.1&type=10\\\">\"\n");
            inibody += QString::fromUtf8("enable=true\n");
            inibody += QString::fromUtf8("\n");
            inibody += QString::fromUtf8("[_search_entry_furu1_online]\n");
            inibody += QString::fromUtf8("name=古市オンライン\n");
            inibody += QString::fromUtf8("search_url_template=\"http://www.furu1online.net/SearchItem?CID=&FREE_WORD=:jancode&PROP_ID_9200=2\"\n");
            inibody += QString::fromUtf8("regex_product_url=\"<h3 class=\\\"geb_itemName\\\"><a href=\\\"([^\\\"]*)\\\"\"\n");
            inibody += QString::fromUtf8("regex_product_img_url=\"<p class=\\\"thumbnail\\\">[^<]*<a[^>]*><img src=\\\"([^\\\"]*)\\\"\"\n");
            inibody += QString::fromUtf8("regex_product_price=\"販売価格 <strong>([0-9,]*)</strong>\"\n");
            inibody += QString::fromUtf8("prefix_product_img_url=http://www.furu1online.net\n");
            inibody += QString::fromUtf8("link_template=\"<a href=\\\"http://click.linksynergy.com/fs-bin/click?id=LPuibgunRpQ&subid=&offerid=192116.1&type=10&tmpid=864&RD_PARM1=:product_eurl\\\" target=\\\"new\\\">:product_name</a><img alt=\\\"icon\\\" width=\\\"1\\\" height=\\\"1\\\" src=\\\"http://ad.linksynergy.com/fs-bin/show?id=LPuibgunRpQ&bids=192116.1&type=10\\\">\"\n");
            inibody += QString::fromUtf8("enable=true\n");
            inibody += QString::fromUtf8("\n");
            inibody += QString::fromUtf8("[_search_entry_tower_record]\n");
            inibody += QString::fromUtf8("name=タワーレコード\n");
            inibody += QString::fromUtf8("search_url_template=http://tower.jp/search/item/:jancode\n");
            inibody += QString::fromUtf8("regex_product_url=\"<div class=\\\"discographyTableImg01\\\">[^<]*<p>[^<]*<!--[^>]*>[^<]*<a href=\\\"([^\\\"]*)\"\n");
            inibody += QString::fromUtf8("regex_product_img_url=\"<div class=\\\"discographyTableImg01\\\">[^<]*<p>[^<]*<!--[^>]*>[^<]*<a[^>]*>[^<]*<!--[^>]*>[^<]*<img src=\\\"([^\\\"]*)\"\n");
            inibody += QString::fromUtf8("regex_product_price=\"<dt>価格</dt><dd[^>]*>￥([0-9,]*)\"\n");
            inibody += QString::fromUtf8("link_template=\"<a href=\\\"http://ck.jp.ap.valuecommerce.com/servlet/referral?sid=3128241&pid=882662482&vc_url=:product_eurl%3Dpafvc\\\" target=\\\"_blank\\\" ><img src=\\\"http://ad.jp.ap.valuecommerce.com/servlet/gifbanner?sid=3128241&pid=882662482\\\" height=\\\"1\\\" width=\\\"0\\\" border=\\\"0\\\">:product_name</a>\"\n");
            inibody += QString::fromUtf8("enable=true\n");
            inibody += QString::fromUtf8("\n");
            inibody += QString::fromUtf8("[_search_entry_tsutaya]\n");
            inibody += QString::fromUtf8("name=TSUTAYA\n");
            inibody += QString::fromUtf8("search_url_template=\"http://shop.tsutaya.co.jp/search_result?ecCategory=00&searchType=1&pSCS=%E3%81%99%E3%81%B9%E3%81%A6%E3%81%AE%E5%95%86%E5%93%81&searchKeyword=:jancode&x=62&y=5\"\n");
            inibody += QString::fromUtf8("regex_product_url=\"<td class=\\\"tolImg\\\">[^<]*<a href=\\\"([^\\\"]*)\"\n");
            inibody += QString::fromUtf8("regex_product_img_url=\"<td class=\\\"tolImg\\\">[^<]*<a href=\\\"[^\\\"]*\\\"><img title=\\\"[^\\\"]*\\\" style=\\\"[^\\\"]*\\\" alt=\\\"[^\\\"]*\\\" src=\\\"([^\\\"]*)\"\n");
            inibody += QString::fromUtf8("regex_product_price=\"<span class=\\\"tolNote\\\"><em>[^0-9,]*([0-9,]*)円\"\n");
            inibody += QString::fromUtf8("prefix_product_url=http://shop.tsutaya.co.jp/\n");
            inibody += QString::fromUtf8("link_template=\"<a href=\\\"http://ck.jp.ap.valuecommerce.com/servlet/referral?sid=3128241&pid=882672755&vc_url=:product_eurl\\\" target=\\\"_blank\\\" ><img src=\\\"http://ad.jp.ap.valuecommerce.com/servlet/gifbanner?sid=3128241&pid=882672755\\\" height=\\\"1\\\" width=\\\"0\\\" border=\\\"0\\\">:product_name</a>\"\n");
            inibody += QString::fromUtf8("enable=true\n");
            inibody += QString::fromUtf8("\n");
            inibody += QString::fromUtf8("[_search_entry_nojima_online]\n");
            inibody += QString::fromUtf8("name=NojimaOnline\n");
            inibody += QString::fromUtf8("search_url_template=\"http://online.nojima.co.jp/app/catalog/list/init?searchCategoryCode=0&searchMethod=0&searchWord=:jancode\"\n");
            inibody += QString::fromUtf8("regex_product_url=\"<div class=\\\"cmdty_iteminfo\\\">[^<]*<div>[^<]*<a href=\\\"([^\\\"]*)\"\n");
            inibody += QString::fromUtf8("regex_product_img_url=\"<div class=\\\"Cmdty_ThumbImg_Cell\\\">[^<]*<a[^>]*>[^<]*<div[^>]*>[^<]*<img src=\\\"([^\\\"]*)\"\n");
            inibody += QString::fromUtf8("regex_product_price=\"<span id=\\\"praiceh\\\"></span><span class=\\\"price\\\">[^0-9,]*([0-9,]*)\"\n");
            inibody += QString::fromUtf8("prefix_product_url=http://online.nojima.co.jp\n");
            inibody += QString::fromUtf8("prefix_product_img_url=http://online.nojima.co.jp\n");
            inibody += QString::fromUtf8("link_template=\"<a href=\\\"http://ck.jp.ap.valuecommerce.com/servlet/referral?sid=3128241&pid=882672760&vc_url=:product_eurl\\\" target=\\\"_blank\\\" ><img src=\\\"http://ad.jp.ap.valuecommerce.com/servlet/gifbanner?sid=3128241&pid=882672760\\\" height=\\\"1\\\" width=\\\"0\\\" border=\\\"0\\\">:product_name</a>\"\n");
            inibody += QString::fromUtf8("enable=true\n");
            inibody += QString::fromUtf8("\n");
            inibody += QString::fromUtf8("[_search_entry_hapinet_online]\n");
            inibody += QString::fromUtf8("name=ハピネットオンライン\n");
            inibody += QString::fromUtf8("search_url_template=\"http://www.happinetonline.com/NASApp/mnas/MxMProduct?Action=prd_detail&KIND=0&SHOP_ID=1&PRODUCT_ID=:jancode&DISPCATEGORY=\"\n");
            inibody += QString::fromUtf8("regex_product_url=\n");
            inibody += QString::fromUtf8("regex_product_img_url=\"<!--メイン画像ここから-->[^<]*<a[^>]*>[^<]*<img src=\\\"([^\\\"]*)\"\n");
            inibody += QString::fromUtf8("regex_product_price=\"<p id=\\\"price_a02\\\">[^<]*<span id=\\\"ss01\\\">販売価格 :</span>[^<]*<span id=\\\"ss02\\\">([0-9,]*)円\"\n");
            inibody += QString::fromUtf8("prefix_product_url=\"http://www.happinetonline.com/NASApp/mnas/MxMProduct?Action=prd_detail&KIND=0&SHOP_ID=1&PRODUCT_ID=:jancode&DISPCATEGORY=\"\n");
            inibody += QString::fromUtf8("prefix_product_img_url=http://www.happinetonline.com\n");
            inibody += QString::fromUtf8("link_template=\"<a href=\\\"http://click.linksynergy.com/fs-bin/click?id=LPuibgunRpQ&subid=&offerid=212933.1&type=10&tmpid=1969&RD_PARM1=:product_eurl\\\">:product_name</a><img alt=\\\"icon\\\" width=\\\"1\\\" height=\\\"1\\\" src=\\\"http://ad.linksynergy.com/fs-bin/show?id=LPuibgunRpQ&bids=212933.1&type=10\\\">\"\n");
            inibody += QString::fromUtf8("enable=true\n");
            inibody += QString::fromUtf8("\n");
            inibody += QString::fromUtf8("[_search_entry_toizarasu]\n");
            inibody += QString::fromUtf8("name=トイザらス\n");
            inibody += QString::fromUtf8("search_url_template=\"http://www.toysrus.co.jp/disp/CSfDispListPage_001.jsp?dispNo=&q=:jancode\"\n");
            inibody += QString::fromUtf8("regex_product_url=\n");
            inibody += QString::fromUtf8("regex_product_img_url=\"<p id=\\\"detail-item-thumb-01\\\" class=\\\"detail-item-thumb\\\">[^<]*<a[^>]*>[^<]*<img onerror=\\\"[^\\\"]*\\\" src=\\\"([^\\\"]*)\"\n");
            inibody += QString::fromUtf8("regex_product_price=\"<i class=\\\"inTax\\\">([0-9,]*)円\"\n");
            inibody += QString::fromUtf8("prefix_product_url=\"http://www.toysrus.co.jp/disp/CSfDispListPage_001.jsp?dispNo=&q=:jancode\"\n");
            inibody += QString::fromUtf8("prefix_product_img_url=http://www.toysrus.co.jp/disp/\n");
            inibody += QString::fromUtf8("link_template=\"<a href=\\\"http://click.linksynergy.com/fs-bin/click?id=LPuibgunRpQ&subid=&offerid=340343.1&type=10&tmpid=1148&RD_PARM1=http%253A%252F%252Fwww.toysrus.co.jp%252Fdisp%252FCSfDispListPage_001.jsp%253Fq%253D:jancode\\\">:product_name</a>\"\n");
            inibody += QString::fromUtf8("enable=true\n");
            out << inibody;
            inifile.close();

            delete m_settings;
            m_settings = NULL;
            m_settings = new QSettings(inifile_path, QSettings::IniFormat, this);
            m_settings->setIniCodec("UTF-8");
            search_entry_list_raw = m_settings->value(tr("global/search_entry_list")).toString();
            if (search_entry_list_raw.isEmpty()) {
                // 設定ファイル復元して読み込み失敗
                msg.setText("設定ファイルの復元に失敗しました");
                msg.exec();
                return false;            }
        } else {
            // 設定ファイルが存在して読み込み失敗
            msg.setText("設定ファイルが不正です");
            msg.exec();
            return false;
        }
    }

    // 検索ページリスト読み込み
    QStringList search_entry_str_list = search_entry_list_raw.split(",");
    for (int i = 0; i < search_entry_str_list.size(); ++i) {
        QString search_entry_id = search_entry_str_list.at(i);
        SearchEntry *search_entry = new SearchEntry(search_entry_id);
        search_entry->load(m_settings);
        m_search_entry_map.insert(search_entry_id, search_entry);
    }

    return true;
}

void MainWindow::addSearchOptionLayout(QHBoxLayout* p) {
    QVBoxLayout *layout = new QVBoxLayout();
    QGroupBox *searchTargetGB = new QGroupBox(tr("検索対象"));
    QFormLayout *stFormLayout = new QFormLayout();

    // 検索対象サイトチェックボックス
    QMap<QString, SearchEntry*>::iterator ite;
    for (ite = m_search_entry_map.begin(); ite != m_search_entry_map.end(); ite++) {
        SearchEntry *se = ite.value();
        QCheckBox *cb = new QCheckBox(se->m_name);
        if (se->m_enable) {
            cb->setCheckState(Qt::Checked);
        } else {
            cb->setCheckState(Qt::Unchecked);
        }
        m_search_entry_checkbox_map.insert(ite.key(), cb);
        stFormLayout->addWidget(cb);
    }
    searchTargetGB->setLayout(stFormLayout);

    layout->addWidget(searchTargetGB);
    p->addLayout(layout);
    return;
}

void MainWindow::addRightLayout(QHBoxLayout* p) {
    QVBoxLayout *layout = new QVBoxLayout;

    // 検索入力
    addSearchInputWidget(layout);

    // 出力オプション
    addOutputOptionWidget(layout);

    //// 実行ボタン
    addSubmitLayout(layout);

    layout->addStretch();
    p->addLayout(layout);
}

void MainWindow::addSearchInputWidget(QVBoxLayout* p) {
    QGroupBox *searchConditionGB = new QGroupBox(tr("検索条件"));
    QVBoxLayout *scLayout = new QVBoxLayout();

    // 商品名
    m_input_product_name_le = new QLineEdit();
    scLayout->addWidget(new QLabel(tr("商品名")));
    scLayout->addWidget(m_input_product_name_le);

    // JANコード
    m_input_jancode_le = new QLineEdit();

    scLayout->addWidget(new QLabel(tr("JANコード")));
    scLayout->addWidget(m_input_jancode_le);
    searchConditionGB->setLayout(scLayout);
    p->addWidget(searchConditionGB);
}

void MainWindow::addOutputOptionWidget(QVBoxLayout* p) {
    QGroupBox *outOptGB = new QGroupBox(tr("出力オプション"));
    QVBoxLayout *outOptLayout = new QVBoxLayout();

    // フォルダパステキストボックス
    m_output_folder_le = new QLineEdit();
    m_settings->beginGroup("global");
    if (m_settings->value("output_folder").toString().isEmpty()) {
        m_output_folder_le->insert(m_here);
    } else {
        m_output_folder_le->insert(m_settings->value("output_folder").toString());
    }
    m_settings->endGroup();

    // フォルダ選択ボタン
    outOptSelectFolderSubmit = new QPushButton(tr("フォルダ選択"));
    outOptSelectFolderSubmit->resize(30, 30);
    outOptSelectFolderSubmit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(outOptSelectFolderSubmit, SIGNAL(clicked(bool)), this, SLOT(outOptSelectFolder()));
    QHBoxLayout *outOptSelectFolderLayout = new QHBoxLayout();
    outOptSelectFolderLayout->addWidget(m_output_folder_le);
    outOptSelectFolderLayout->addWidget(outOptSelectFolderSubmit);
    outOptLayout->addLayout(outOptSelectFolderLayout);
    outOptGB->setLayout(outOptLayout);

    p->addWidget(outOptGB);
}

void MainWindow::addSubmitLayout(QVBoxLayout* p) {
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(10);
    QPushButton *submit = new QPushButton(tr("実行"));
    submit->resize(30, 30);
    submit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(submit, SIGNAL(clicked(bool)), this, SLOT(run()));
    layout->addWidget(submit);
    layout->setAlignment(Qt::AlignRight);
    p->addLayout(layout);
}

void MainWindow::outOptSelectFolder()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::Directory);
    fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
    if(fileDialog.exec()){
        QString folder = fileDialog.selectedFiles().join("\\").replace(QString("/"), QString("\\"));
        m_output_folder_le->setText(folder);
        m_settings->beginGroup("global");
        m_settings->setValue("output_folder", folder);
        m_settings->endGroup();
    }
}

void MainWindow::run()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_run_wait_count = 0;
    QMap<QString, SearchEntry*>::iterator ite;
    for (ite = m_search_entry_map.begin(); ite != m_search_entry_map.end(); ++ite) {
        QString key = ite.key();
        SearchEntry *se = ite.value();
        if (m_search_entry_checkbox_map[key]->checkState() == Qt::Checked) {
            se->m_enable= true;
            m_run_wait_count++;
        } else {
            se->m_enable= false;
        }
        // 設定も更新しておく
        m_settings->beginGroup(se->getIniGroup());
        m_settings->setValue("enable", se->m_enable);
        m_settings->endGroup();
    }
    for (ite = m_search_entry_map.begin(); ite != m_search_entry_map.end(); ++ite) {
        QString key = ite.key();
        SearchEntry *se = ite.value();
        if (se->m_enable) {
            se->setInput(m_input_product_name_le->text(), m_input_jancode_le->text());

            QNetworkAccessManager *nam = new QNetworkAccessManager(this);
            connect(nam, SIGNAL(finished(QNetworkReply*)),
                    this, SLOT(replyFinished(QNetworkReply*)));
            se->fetch(nam, m_search_entry_network_reply_map);
        }
    }
    if(m_run_wait_count == 0) {
        QApplication::restoreOverrideCursor();
    }
}

void MainWindow::replyFinished(QNetworkReply *reply)
{
    try {
        SearchEntry *se = m_search_entry_map[m_search_entry_network_reply_map[reply]];
        if(reply->error() == QNetworkReply::NoError) {
            // リダイレクトチェック
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if(statusCode == 301 || statusCode == 302 || statusCode == 303) {
                QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
                QNetworkAccessManager *manager = new QNetworkAccessManager();
                QEventLoop eventLoop;
                connect(manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
                reply = manager->get(QNetworkRequest(redirectUrl));
                eventLoop.exec();
            }

            QByteArray htmlRaw = reply->readAll();
            QTextCodec* codec = QTextCodec::codecForHtml(htmlRaw);
            QString html = codec->toUnicode(htmlRaw);
            se->extractProductUrl(html);
            se->extractProductImgUrl(html);
            se->extractProductNO(html);
            se->extractProductPrice(html);
            se->extractProductDate(html);
            se->buildLink();

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
        } else {
            /*
            QString str;
            str = se->m_name;
            str += tr("の検索に失敗しました。");
            str += reply->errorString();
            QMessageBox msg_box;
            msg_box.setText(str);
            msg_box.exec();
            */
        }

        m_run_wait_count--;
        if (m_run_wait_count < 1) {
            QString str;
            QMap<QString, SearchEntry*>::iterator ite;
            for (ite = m_search_entry_map.begin(); ite != m_search_entry_map.end(); ++ite) {
                SearchEntry *se = ite.value();
                if (!se->m_enable) {
                    continue;
                }
                str += se->m_name;
                str += "\t";
                str += se->m_jancode;
                str += "\t";
                str += se->m_link;
                str += "\t";
                str += se->m_search_url.toString();
                str += "\t";
                str += se->m_product_url;
                str += "\t";
                str += se->m_product_img_url;
                str += "\t";
                str += se->m_product_price;
                str += "\t";
                if (se->m_ok) {
                    str += tr("成功");
                    str += "\n";
                } else {
                    str += tr("抽出商品情報不完全");
                    str += "\n";
                }
            }

            QString filename = m_output_folder_le->text() + tr("\\") + m_input_jancode_le->text() + tr(".tsv");
            QFile file(filename);
            if (!file.open(QIODevice::WriteOnly))
            {
                QApplication::restoreOverrideCursor();
                QMessageBox::information(this, tr("Unable to open file"),
                    file.errorString());
                return;
            }
            QTextCodec* codec = QTextCodec::codecForName("UTF-8");
            QTextStream stream( &file );
            stream.setCodec( codec );
            stream << str;
            file.close();
            QApplication::restoreOverrideCursor();
            QMessageBox msg_box;
            msg_box.setText("ファイルへの出力が完了しました");
            msg_box.exec();
        }
    } catch(...) {
        QApplication::restoreOverrideCursor();
    }
}

#ifndef QT_NO_CONTEXTMENU
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(cutAct);
    menu.addAction(copyAct);
    menu.addAction(pasteAct);
    menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU

void MainWindow::newFile()
{
    infoLabel->setText(tr("Invoked <b>File|New</b>"));
}

void MainWindow::open()
{
    infoLabel->setText(tr("Invoked <b>File|Open</b>"));
}

void MainWindow::save()
{
    infoLabel->setText(tr("Invoked <b>File|Save</b>"));
}

void MainWindow::print()
{
    infoLabel->setText(tr("Invoked <b>File|Print</b>"));
}

void MainWindow::undo()
{
    infoLabel->setText(tr("Invoked <b>Edit|Undo</b>"));
}

void MainWindow::redo()
{
    infoLabel->setText(tr("Invoked <b>Edit|Redo</b>"));
}

void MainWindow::cut()
{
    infoLabel->setText(tr("Invoked <b>Edit|Cut</b>"));
}

void MainWindow::copy()
{
    infoLabel->setText(tr("Invoked <b>Edit|Copy</b>"));
}

void MainWindow::paste()
{
    infoLabel->setText(tr("Invoked <b>Edit|Paste</b>"));
}

void MainWindow::bold()
{
    infoLabel->setText(tr("Invoked <b>Edit|Format|Bold</b>"));
}

void MainWindow::italic()
{
    infoLabel->setText(tr("Invoked <b>Edit|Format|Italic</b>"));
}

void MainWindow::leftAlign()
{
    infoLabel->setText(tr("Invoked <b>Edit|Format|Left Align</b>"));
}

void MainWindow::rightAlign()
{
    infoLabel->setText(tr("Invoked <b>Edit|Format|Right Align</b>"));
}

void MainWindow::justify()
{
    infoLabel->setText(tr("Invoked <b>Edit|Format|Justify</b>"));
}

void MainWindow::center()
{
    infoLabel->setText(tr("Invoked <b>Edit|Format|Center</b>"));
}

void MainWindow::setLineSpacing()
{
    infoLabel->setText(tr("Invoked <b>Edit|Format|Set Line Spacing</b>"));
}

void MainWindow::setParagraphSpacing()
{
    infoLabel->setText(tr("Invoked <b>Edit|Format|Set Paragraph Spacing</b>"));
}

void MainWindow::about()
{
    infoLabel->setText(tr("Invoked <b>Help|About</b>"));
    QMessageBox::about(this, tr("About Menu"),
            tr("The <b>Menu</b> example shows how to create "
               "menu-bar menus and context menus."));
}

void MainWindow::aboutQt()
{
    infoLabel->setText(tr("Invoked <b>Help|About Qt</b>"));
}

void MainWindow::createActions()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip(tr("Print the document"));
    connect(printAct, &QAction::triggered, this, &MainWindow::print);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    undoAct = new QAction(tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last operation"));
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);

    redoAct = new QAction(tr("&Redo"), this);
    redoAct->setShortcuts(QKeySequence::Redo);
    redoAct->setStatusTip(tr("Redo the last operation"));
    connect(redoAct, &QAction::triggered, this, &MainWindow::redo);

    cutAct = new QAction(tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, &QAction::triggered, this, &MainWindow::cut);

    copyAct = new QAction(tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, &QAction::triggered, this, &MainWindow::copy);

    pasteAct = new QAction(tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, &QAction::triggered, this, &MainWindow::paste);

    boldAct = new QAction(tr("&Bold"), this);
    boldAct->setCheckable(true);
    boldAct->setShortcut(QKeySequence::Bold);
    boldAct->setStatusTip(tr("Make the text bold"));
    connect(boldAct, &QAction::triggered, this, &MainWindow::bold);

    QFont boldFont = boldAct->font();
    boldFont.setBold(true);
    boldAct->setFont(boldFont);

    italicAct = new QAction(tr("&Italic"), this);
    italicAct->setCheckable(true);
    italicAct->setShortcut(QKeySequence::Italic);
    italicAct->setStatusTip(tr("Make the text italic"));
    connect(italicAct, &QAction::triggered, this, &MainWindow::italic);

    QFont italicFont = italicAct->font();
    italicFont.setItalic(true);
    italicAct->setFont(italicFont);

    setLineSpacingAct = new QAction(tr("Set &Line Spacing..."), this);
    setLineSpacingAct->setStatusTip(tr("Change the gap between the lines of a "
                                       "paragraph"));
    connect(setLineSpacingAct, &QAction::triggered, this, &MainWindow::setLineSpacing);

    setParagraphSpacingAct = new QAction(tr("Set &Paragraph Spacing..."), this);
    setParagraphSpacingAct->setStatusTip(tr("Change the gap between paragraphs"));
    connect(setParagraphSpacingAct, &QAction::triggered,
            this, &MainWindow::setParagraphSpacing);

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(aboutQtAct, &QAction::triggered, this, &MainWindow::aboutQt);

    leftAlignAct = new QAction(tr("&Left Align"), this);
    leftAlignAct->setCheckable(true);
    leftAlignAct->setShortcut(tr("Ctrl+L"));
    leftAlignAct->setStatusTip(tr("Left align the selected text"));
    connect(leftAlignAct, &QAction::triggered, this, &MainWindow::leftAlign);

    rightAlignAct = new QAction(tr("&Right Align"), this);
    rightAlignAct->setCheckable(true);
    rightAlignAct->setShortcut(tr("Ctrl+R"));
    rightAlignAct->setStatusTip(tr("Right align the selected text"));
    connect(rightAlignAct, &QAction::triggered, this, &MainWindow::rightAlign);

    justifyAct = new QAction(tr("&Justify"), this);
    justifyAct->setCheckable(true);
    justifyAct->setShortcut(tr("Ctrl+J"));
    justifyAct->setStatusTip(tr("Justify the selected text"));
    connect(justifyAct, &QAction::triggered, this, &MainWindow::justify);

    centerAct = new QAction(tr("&Center"), this);
    centerAct->setCheckable(true);
    centerAct->setShortcut(tr("Ctrl+E"));
    centerAct->setStatusTip(tr("Center the selected text"));
    connect(centerAct, &QAction::triggered, this, &MainWindow::center);

    alignmentGroup = new QActionGroup(this);
    alignmentGroup->addAction(leftAlignAct);
    alignmentGroup->addAction(rightAlignAct);
    alignmentGroup->addAction(justifyAct);
    alignmentGroup->addAction(centerAct);
    leftAlignAct->setChecked(true);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    formatMenu = editMenu->addMenu(tr("&Format"));
    formatMenu->addAction(boldAct);
    formatMenu->addAction(italicAct);
    formatMenu->addSeparator()->setText(tr("Alignment"));
    formatMenu->addAction(leftAlignAct);
    formatMenu->addAction(rightAlignAct);
    formatMenu->addAction(justifyAct);
    formatMenu->addAction(centerAct);
    formatMenu->addSeparator();
    formatMenu->addAction(setLineSpacingAct);
    formatMenu->addAction(setParagraphSpacingAct);
}
