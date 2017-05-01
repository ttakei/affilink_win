#ifndef WWL_H
#define WWL_H

#include <QString>
#include <QMap>
#include <QPair>
#include <QFileInfo>
#include "log.h"

#define WWL_DEFAULT_KEY "default"

class Wwl
{
public:
    explicit Wwl();
    bool load(Log*);
    QString get(const QString&);
    QString m_dir;
protected:
    QMap<QString, QPair<QString, QFileInfo>> m_templates;
};

#endif // WWL_H
