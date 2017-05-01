#include "wwl.h"
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>
#include <QDateTime>

Wwl::Wwl() {}

bool Wwl::load(Log* log) {
    QDir dir = QDir(m_dir);
    if (!dir.exists()) {
        log->err("wwlテンプレートフォルダが存在しません");
        return false;
    }

    QStringList filter;
    filter += "*.tpl";
    QStringList files = dir.entryList(filter);

    for (QStringList::iterator it = files.begin(); it != files.end(); ++it) {
        QString filepath = m_dir + QDir::separator() + *it;
        QString basename = QFileInfo(filepath).baseName();

        QFile file(filepath);
        QFileInfo fileinfo(file);
        if (m_templates.contains(basename)) {
            QDateTime last_modify_before = m_templates[basename].second.lastModified();
            if (m_templates[basename].second.filePath() == fileinfo.filePath() && last_modify_before == fileinfo.lastModified()) {
                // 前回から変更ない場合はなにもしない
                continue;
            }
        }

        if (!file.open(QIODevice::ReadOnly)) {
            log->warn(QString(filepath) + "が開けません");
        }

        QTextCodec* codec = QTextCodec::codecForName("UTF-8");
        QTextStream in(&file);
        in.setCodec(codec);
        m_templates[basename] = QPair<QString, QFileInfo>(in.readAll(), fileinfo);
        file.close();
    }

    return true;
}

QString Wwl::get(const QString& id) {
    if (m_templates.contains(id)) {
        return m_templates[id].first;
    } else {
        return m_templates[WWL_DEFAULT_KEY].first;
    }
}
