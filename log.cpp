#include "log.h"
#include <QString>
#include <QTime>
#include <QTextStream>
#include <QMessageBox>
#include <QTextCursor>
#include <QApplication>

Log::Log(){}
Log::Log(QWidget* main, QTextEdit* display, Level loglevel):
m_msg_widget(main),
m_display_widget(display),
m_loglevel(loglevel)
{}

void Log::setDisplayWidget(QTextEdit* display) {
    m_display_widget = display;
}

void Log::setLogLevel(const QString& levelstr) {
    if (levelstr == "err") {
        m_loglevel = Level::ERR;
    } else if (levelstr == "warn") {
        m_loglevel = Level::WARN;
    } else if (levelstr == "info") {
        m_loglevel = Level::INFO;
    } else if (levelstr == "debug") {
        m_loglevel = Level::DEBUG;
    } else {
        m_loglevel = Level::INFO;
    }
}

void Log::display(Level level, const QString &str) {
    if (m_display_widget == NULL) {
        return;
    }

    if (level > m_loglevel) {
        return;
    }

    m_display_widget->append(str);
    m_display_widget->textCursor().movePosition(QTextCursor::EndOfBlock);
}

void Log::debug(const QString &body) {
    display(Level::DEBUG, body);
}

void Log::info(const QString &body, const QString &title, bool msg) {
    display(Level::INFO, body);
    if (msg) {
        msginfo(body, title);
    }
}

void Log::warn(const QString &body, const QString &title, bool msg) {
    display(Level::WARN, body);
    if (msg) {
        msgwarn(body, title);
    }
}

void Log::err(const QString &body, const QString &title) {
    display(Level::ERR, body);
    msgwarn(body, title);
}

void Log::msginfo(const QString &body, const QString &title)
{
    QApplication::restoreOverrideCursor();
    QMessageBox::information(m_msg_widget, title, body);
}

void Log::msgwarn(const QString &body, const QString &title)
{
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(m_msg_widget, title, body);
}
