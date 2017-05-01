#ifndef CSV_H
#define CSV_H

#include <QString>
#include <QMap>
#include <QFileInfo>
#include "log.h"
#include "ini.h"

class Product
{
public:
    Product();
    bool load(const QString&);
    const QString& code() const;
    const QString& name() const;
    const QString& store() const;
    const QString& jancode() const;
    const QString& min_price() const;
    const QString& date() const;
    const QString& soldout() const;
protected:
    QString m_code;
    QString m_name;
    QString m_store;
    QString m_jancode;
    QString m_min_price;
    QString m_date;
    QString m_soldout;
};

class Csv
{
public:
    QString m_file;
    Csv();
    bool load(Log*, Ini*);
    const QString& code(const QString&);
    const QString& name(const QString&);
    const QString& store(const QString&);
    const QString& jancode(const QString&);
    const QString& min_price(const QString&);
    const QString& date(const QString&);
    const QString& soldout(const QString&);
protected:
    QFileInfo m_file_last;
    Product m_empty_product;
    QMap<QString, Product> m_product_map;
    const Product& get(const QString&);
};

#endif // CSV_H
