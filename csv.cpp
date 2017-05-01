#include "csv.h"
#include <QStringList>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QDateTime>

Product::Product() {}

bool Product::load(const QString& csv) {
    QStringList list = csv.split(",");
    if (list.size() < 7) {
        return false;
    }

    int i = 0;
    m_code = list.at(i++);
    m_name = list.at(i++);
    m_store = list.at(i++);
    m_jancode = list.at(i++);
    m_min_price = list.at(i++);
    m_date = list.at(i++);
    m_soldout = list.at(i++);

    return true;
}

const QString& Product::code() const {
    return m_code;
}

const QString& Product::name() const {
    return m_name;
}

const QString& Product::store() const {
    return m_store;
}

const QString& Product::jancode() const {
    return m_jancode;
}

const QString& Product::min_price() const {
    return m_min_price;
}

const QString& Product::date() const {
    return m_date;
}

const QString& Product::soldout() const {
    return m_soldout;
}


Csv::Csv() {}

bool Csv::load(Log* log, Ini* ini) {
    // ヘッダ有無
    bool headless = true;
    QString headless_str = ini->get("global/csv_headless").toString();
    if (headless_str.isEmpty() || headless_str == "0") {
        headless = false;
    }

    // 文字コード
    QByteArray charcode = ini->get("global/csv_charcode", "Shift-JIS").toByteArray();

    // ファイル
    QFile file(m_file);
    QFileInfo fileinfo = QFileInfo(file);
    if (!m_file_last.filePath().isEmpty()) {
        QDateTime last_modify_before = m_file_last.lastModified();
        if (m_file_last.filePath() == fileinfo.filePath() && last_modify_before == fileinfo.lastModified()) {
            // 前回から変更ない場合はなにもしない
            return true;
        }
    }

    if (!file.open(QIODevice::ReadOnly)) {
        log->err(QString(m_file) + "が開けません");
        return false;
    }

    QTextCodec* codec = QTextCodec::codecForName(charcode);
    QTextStream in(&file);
    in.setCodec(codec);
    QStringList lines = in.readAll().split("\n");
    int i = 0;
    for (QStringList::iterator it = lines.begin(); it != lines.end(); ++it) {
        i++;

        // ヘッダはスルー
        if (!headless && i == 1) {
            continue;
        }

        // 空行はスルー
        QString line = it->trimmed();\
        if (line.isEmpty()) {
            continue;
        }

        Product product;
        if (!product.load(line)) {
            QString logout;
            QTextStream logouts(&logout);
            logouts.setCodec(codec);
            logouts << fileinfo.fileName() << i << QString("行目読み込み失敗");
            log->info(logout);
            continue;
        }
        m_product_map[product.jancode()] = product;
    }

    file.close();
    m_file_last = fileinfo;
    return true;
}

const QString& Csv::code(const QString& jancode) {
    const Product& product = get(jancode);
    return product.code();
}

const QString& Csv::name(const QString& jancode) {
    const Product& product = get(jancode);
    return product.name();
}

const QString& Csv::store(const QString& jancode) {
    const Product& product = get(jancode);
    return product.store();
}

const QString& Csv::jancode(const QString& jancode) {
    const Product& product = get(jancode);
    return product.jancode();
}

const QString& Csv::min_price(const QString& jancode) {
    const Product& product = get(jancode);
    return product.min_price();
}

const QString& Csv::date(const QString& jancode) {
    const Product& product = get(jancode);
    return product.date();
}

const QString& Csv::soldout(const QString& jancode) {
    const Product& product = get(jancode);
    return product.soldout();
}

const Product& Csv::get(const QString& jancode) {
    if (!m_product_map.contains(jancode)) {
        return m_empty_product;
    }
    return m_product_map[jancode];
}
