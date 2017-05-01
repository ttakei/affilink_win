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

    // ディレクトリが存在しない場合は作成
    if (!dir.exists() && !dir.mkpath(dir.absolutePath())){
        log->info(m_dir + QString("作成失敗"));
        return false;
    }

    // デフォルトのtplが存在しない場合は作成
    if (!m_templates.contains(WWL_DEFAULT_KEY)) {
        if (!restoreDefaultFile(log)) {
            log->info(QString(WWL_DEFAULT_KEY".tpl作成失敗"));
            return false;
        }
    }

    // ディレクトリ直下のテンプレートファイル
    QStringList filter;
    filter += "*.tpl";
    QStringList files = dir.entryList(filter);

    // テンプレートファイル読み込み
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

bool Wwl::restoreDefaultFile(Log* log) {
    QString filepath = m_dir + QString("\\"WWL_DEFAULT_KEY".tpl");
    QFile file(filepath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        log->info(filepath + QString("作成失敗"));
        return false;
    }
    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("UTF-8"));
    QString body = getDefault();
    out << body;
    file.close();

    return true;
}

QString Wwl::getDefault() {
    QString wwl = QString::fromUtf8(R"###EOF###(	//{{TOP-W
    //{{TOP
Name: :id
Targ:
Refe:
Exec: :nameの『:jancode』のリンク一覧
Keyw:
Cate:
Blog: :link_template
    END-W}}//
)###EOF###");
    return wwl;
}
