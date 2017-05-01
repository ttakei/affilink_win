#ifndef LOG_H
#define LOG_H

#include <QWidget>
#include <QTextEdit>
#include <QString>

class Log
{
public:
    enum Level {
        ERR,
        WARN,
        INFO,
        DEBUG
    };

    Log();
    Log(QWidget*, QTextEdit* = NULL, Level = Level::INFO);
    void setDisplayWidget(QTextEdit*);
    void setLogLevel(const QString&);
    void debug(const QString&);
    void info(const QString&, const QString& = "", bool = false);
    void warn(const QString&, const QString& = "", bool = false);
    void err(const QString&, const QString& = "");
    void msginfo(const QString&, const QString& = "");
    void msgwarn(const QString&, const QString& = "");

    QTextEdit* m_display_widget;
    QWidget* m_msg_widget;
    Level m_loglevel;

private:
    void display(Level, const QString&);
};
#endif // LOG_H
