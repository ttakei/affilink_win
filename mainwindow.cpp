#include "mainwindow.h"
#include <QtWidgets>
#include <QtNetwork/QNetworkAccessManager>
#include <QDir>
#include <QFile>
#include <QVector>
#include <QTextStream>

MainWindow::MainWindow() {}

bool MainWindow::load() {
    m_log = Log(this);
    m_ini = Ini(&m_log);
    m_wwl = Wwl();
    m_csv = Csv();
    m_search = new SearchEntryManager(&m_log, &m_ini, &m_wwl, &m_csv);
    m_layout = new Layout(this, &m_log, &m_ini, m_search, &m_wwl, &m_csv);
    m_here = QDir::current().canonicalPath();

    // 設定読み込み
    QString inifile_path = m_here + tr("\\conf\\default.ini");
    if (!m_ini.load(inifile_path) || !m_search->load()) {
        m_log.err("設定の読み込みに失敗しました");
        return false;
    }

    // レイアウト
    setWindowTitle("リンク作成ツール");
    QWidget *widget = new QWidget;
    widget->setLayout(m_layout->load());
    m_layout->setCallbackSubmit(this, SLOT(callbackSubmit()));
    setCentralWidget(widget);

    // リサイズ
    initSize();

    return true;
}

void MainWindow::initSize() {
    setMinimumSize(Layout::MIN_WIDTH, Layout::MIN_HEIGHT);
    resize(Layout::MIN_WIDTH, Layout::MIN_HEIGHT);
}

void MainWindow::callbackSubmit() {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // 入力読み込み
    if (!m_layout->loadInput()) {
        m_log.info("入力不正により処理終了");
        QApplication::restoreOverrideCursor();
        return;
    }

    // wwl読み込み
    if (!m_wwl.load(&m_log)) {
        m_log.info("WWL読み込みエラーにより処理終了");
        QApplication::restoreOverrideCursor();
        return;
    }

    // csv読み込み
    if (!m_csv.load(&m_log, &m_ini)) {
        m_log.info("csv読み込みエラーにより処理終了");
        QApplication::restoreOverrideCursor();
        return;
    }

    // 商品番号検索
    m_log.info("商品番号取得中");
    QString product_code;
    if (m_search->searchProductCode(product_code)) {
        m_search->setProductCode(product_code);
    }

    // 検索
    if (m_search->search(this, SLOT(callbackSearch()))) {
        m_log.info("検索実行中");
    }

    return;
}

void MainWindow::callbackSearch() {
    // 商品詳細情報
    if (m_search->parseProduct(this, SLOT(callbackParseProduct()))) {
        m_log.info("商品ページパース実行中");
    }
    return;
}

void MainWindow::callbackParseProduct() {
    if (outResult()) {
        m_log.info("処理が完了しました", "", true);
    } else {
        m_log.info("出力失敗により処理終了");
    }
    QApplication::restoreOverrideCursor();
    return;
}

bool MainWindow::outResult() {
    QString out;
    QTextStream outstream(&out);
    QString wwl;
    for (QMap<QString, SearchEntry*>::iterator it = m_search->m_map.begin(); it != m_search->m_map.end(); ++it) {
        SearchEntry *se = it.value();
        // これまでの結果を展開
        se->buildResult(&m_wwl);

        // 有効でない場合、あるいはリンクテンプレート設定なしの場合は出力しない
        if (!se->m_enable || se->m_links.size() < 1) {
            continue;
        }

        // tsv
        outstream
            << se->m_name << "\t"
            << se->m_jancode << "\t";
        int max_link_num = m_search->getMaxLinkNum();
        for (int i = 0; i < max_link_num; ++i) {
            if (i < se->m_links.size()) {
                outstream << se->m_links.at(i) << "\t";
            } else {
                outstream << "\t";
            }
        }
        outstream
            << se->m_search_url.toString() << "\t"
            << se->m_product_url << "\t"
            << se->m_product_img_url << "\t"
            << se->m_product_price << "\t";
        int max_parsed_product_word_num = m_search->getMaxParsedProductWordNum();
        for (int i = 0; i < max_parsed_product_word_num; ++i) {
            if (i < se->m_product_page_word.size()) {
                if (se->m_product_page_word.at(i)) {
                    outstream << "1\t";
                } else {
                    outstream << "0\t";
                }
            } else {
                outstream << "\t";
            }
        }
        outstream
            << se->m_product_page_desc << "\n";

        // wwl
        wwl = wwl + se->m_wwl;
    }

    // tsv出力
    QString file_tsv = m_layout->m_output_folder_le->text() + tr("\\") + m_layout->m_input_jancode_le->text() + tr(".tsv");
    if (!writeFile(file_tsv, out)) {
        m_log.err("tsvファイルの出力に失敗しました");
        QApplication::restoreOverrideCursor();
        return false;
    }
    m_log.info("tsvファイル出力完了");

    // wwl出力
    QString file_wwl = m_layout->m_output_folder_le->text() + tr("\\") + m_layout->m_input_jancode_le->text() + tr(".wwl");
    if (!writeFile(file_wwl, wwl)) {
        m_log.err("wwlファイルの出力に失敗しました");
        QApplication::restoreOverrideCursor();
        return false;
    }
    m_log.info("wwlファイル出力完了");

    // html出力
    for (QMap<QString, SearchEntry*>::iterator it = m_search->m_map.begin(); it != m_search->m_map.end(); ++it) {
        SearchEntry *se = it.value();
        if (!se->m_enable) {
            continue;
        }
        QString file_html = m_here + tr("\\html\\") + se->m_name + tr(".html");
        writeFile(file_html, se->m_html);
    }
    m_log.debug("htmlファイル出力完了");

    QApplication::restoreOverrideCursor();
    return true;
}

bool MainWindow::writeFile(const QString& filepath, const QString& str) {
    QFile file(filepath);
    QString dirpath = QFileInfo(filepath).absolutePath();

    // ディレクトリがない場合は作成
    QDir dir;
    if (!dir.exists(dirpath)){
        if (!dir.mkdir(dirpath)) {
            m_log.info(dirpath + tr("作成失敗"));
            return false;
        }
    }

    // ファイルオープン
    if (!file.open(QIODevice::WriteOnly)) {
        m_log.info(filepath + tr("オープン失敗"));
        m_log.info(file.errorString());
        return false;
    }

    // エンコード変換、出力
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextStream stream(&file);
    stream.setCodec(codec);
    stream << str;
    file.close();

    return true;
}
