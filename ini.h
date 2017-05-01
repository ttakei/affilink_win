#ifndef INI_H
#define INI_H

#include <QString>
#include <QStringList>
#include <QSettings>
#include "log.h"

class Ini
{
public:
    Ini();
    Ini(Log*);
    bool load(const QString&);
    bool contains(const QString&);
    bool empty(const QString&);
    QVariant get(const QString&, const QVariant& = "");
    void set(const QString&, const QString&);
    void set(const QString&, const QString&, const QString&);
    void begingroup(const QString&);
    void endgroup();
    QStringList explode(const QString&, const QString&);

protected:
    QSettings* m_settings;
    Log* m_log;

    bool restoreFile(const QString&);
    QString getDefault();
};

#endif // INI_H
