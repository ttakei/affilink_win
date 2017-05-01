#ifndef LAYOUT_H
#define LAYOUT_H

#include <QObject>
#include <QWidget>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QString>
#include <QMap>
#include "log.h"
#include "ini.h"
#include "searchentrymanager.h"
#include "wwl.h"
#include "csv.h"

class Layout : public QObject
{
    Q_OBJECT
public:
    static const int MIN_WIDTH = 600;
    static const int MIN_HEIGHT = 520;

    QLineEdit *m_input_product_name_le;
    QLineEdit *m_input_jancode_le;
    QLineEdit *m_input_search_word_le;
    QLineEdit *m_input_product_code_le;
    QLineEdit *m_output_folder_le;
    QLineEdit *m_output_wwl_folder_le;
    QLineEdit *m_output_csv_file_le;
    QLineEdit *m_output_remarks_1_le;
    QLineEdit *m_output_remarks_2_le;
    QLineEdit *m_output_remarks_3_le;
    QLineEdit *m_output_remarks_4_le;
    QLineEdit *m_output_remarks_5_le;
    QTextEdit *m_log_te;
    QMap<QString, QCheckBox*> m_search_entry_checkbox_map;

    explicit Layout(QObject* = 0);
    explicit Layout(QWidget*, Log*, Ini*, SearchEntryManager*, Wwl*, Csv*);

    QBoxLayout* load();
    void setCallbackSubmit(QObject*, const char*);
    bool loadInput();

protected:
    const int MAX_WIDTH_COL1 = 240;
    const int MAX_HEIGHT_LOG = 240;
    const int MIN_HEIGHT_SEARCH_INPUT = 250;

    QWidget* m_main;
    Ini* m_ini;
    Log* m_log;
    SearchEntryManager* m_search;
    Wwl* m_wwl;
    Csv* m_csv;
    QString m_here;
    QPushButton *m_output_folder_pb;
    QPushButton *m_output_wwl_folder_pb;
    QPushButton *m_output_csv_file_pb;
    QPushButton *m_submit_pb;

    bool validate();
    QWidget* createSearchEntryWidget();
    QWidget* createSearchInputWidget();
    QWidget* createOutputInputWidget();
    QBoxLayout* createSubmitLayout();
    QWidget* createLogWidget();

private slots:
    void callbackSelectOutputFolder();
    void callbackSelectWwlTemplateFolder();
    void callbackSelectCsvFile();
};

#endif // LAYOUT_H
