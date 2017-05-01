#include "layout.h"
#include <QGroupBox>
#include <QFormLayout>
#include <QDir>
#include <QTextCodec>
#include <QLabel>
#include <QFileDialog>

Layout::Layout(QObject* parent) : QObject(parent){}

Layout::Layout(QWidget* main, Log* log, Ini* ini, SearchEntryManager* search, Wwl* wwl, Csv* csv) {
    m_main = main;
    m_log = log;
    m_ini = ini;
    m_search = search;
    m_wwl = wwl;
    m_csv = csv;
    m_here = QDir(QDir::current()).canonicalPath().replace("/", "\\");
}

QBoxLayout* Layout::load() {
    QVBoxLayout *layout = new QVBoxLayout;

    // 各列を束ねるレイアウト
    QHBoxLayout *cols_layout = new QHBoxLayout;

    // 1列目
    QWidget *widget_c1 = new QWidget();
    QVBoxLayout *layout_c1 = new QVBoxLayout;
    // - 検索サイトエリア
    layout_c1->addWidget(createSearchEntryWidget());
    // - ログメッセージエリア
    layout_c1->addWidget(createLogWidget());
    widget_c1->setLayout(layout_c1);
    widget_c1->setMaximumWidth(MAX_WIDTH_COL1);
    cols_layout->addWidget(widget_c1);

    // 2列目
    QWidget *widget_c2 = new QWidget();
    QVBoxLayout *layout_c2 = new QVBoxLayout;
    // - 検索入力
    layout_c2->addWidget(createSearchInputWidget());
    // - 出力オプション
    layout_c2->addWidget(createOutputInputWidget());
    // - 実行ボタン
    layout_c2->addLayout(createSubmitLayout());
    widget_c2->setLayout(layout_c2);
    cols_layout->addWidget(widget_c2);

    layout->addLayout(cols_layout);
    return layout;
}

bool Layout::loadInput() {
    if (!validate()) {
        return false;
    }

    // search
    for (QMap<QString, SearchEntry*>::iterator it = m_search->m_map.begin(); it != m_search->m_map.end(); it++) {
        SearchEntry* se = it.value();
        se->init(m_ini, m_csv);
        se->m_product_name = m_input_product_name_le->text();
        se->m_jancode = m_input_jancode_le->text();
        se->m_search_word = m_input_search_word_le->text();
        QTextCodec* codec = QTextCodec::codecForLocale();
        se->m_search_eword = codec->toUnicode(QUrl::toPercentEncoding(se->m_search_word));
        se->m_product_code = m_input_product_code_le->text();
        se->m_remarks1 = m_output_remarks_1_le->text();
        se->m_remarks2 = m_output_remarks_2_le->text();
        se->m_remarks3 = m_output_remarks_3_le->text();
        se->m_remarks4 = m_output_remarks_4_le->text();
        se->m_remarks5 = m_output_remarks_5_le->text();
        se->m_enable = m_search_entry_checkbox_map[se->m_id]->checkState() != Qt::Unchecked;

        // 設定更新
        if (se->m_enable) {
            m_ini->set(se->getIniGroup(), "enable", "true");
        } else {
            m_ini->set(se->getIniGroup(), "enable", "false");
        }
    }

    // wwl
    m_wwl->m_dir = m_output_wwl_folder_le->text();

    // csv
    m_csv->m_file = m_output_csv_file_le->text();

    // 設定更新
    m_ini->set("global", "output_folder", m_output_folder_le->text());
    m_ini->set("global", "wwl_folder", m_output_wwl_folder_le->text());
    m_ini->set("global", "csv_file", m_output_csv_file_le->text());

    return true;
}

bool Layout::validate() {
    // jancode
    if (m_input_jancode_le->text().isEmpty()) {
        m_log->err("JANコードが空です", "入力エラー");
        return false;
    }

    // 出力先フォルダ
    if (m_output_folder_le->text().isEmpty()) {
        m_log->err("出力先フォルダが空です", "入力エラー");
        return false;
    }
    QDir dir(m_output_folder_le->text());
    if (!dir.exists()) {
        m_log->err("出力先フォルダが存在しません", "入力エラー");
        return false;
    }

    // 出力テンプレートフォルダ
    if (m_output_wwl_folder_le->text().isEmpty()) {
        m_log->err("wwlテンプレートフォルダが空です", "入力エラー");
        return false;
    }
    QDir dir_wwl(m_output_wwl_folder_le->text());
    if (!dir_wwl.exists()) {
        m_log->err("wwlテンプレートフォルダが存在しません", "入力エラー");
        return false;
    }

    // CSVファイル
    if (m_output_csv_file_le->text().isEmpty()) {
        m_log->err("商品csvファイルの指定が空です", "入力エラー");
        return false;
    }
    QFile file_csv(m_output_wwl_folder_le->text());
    if (!file_csv.exists()) {
        m_log->err("csvファイルが存在しません", "入力エラー");
        return false;
    }

    return true;
}

QWidget* Layout::createSearchEntryWidget() {
    QGroupBox *input_group_box = new QGroupBox("検索対象");
    QFormLayout *input_layout = new QFormLayout();

    // 検索対象サイトチェックボックス
    for (QMap<QString, SearchEntry*>::iterator it = m_search->m_map.begin(); it != m_search->m_map.end(); it++) {
        SearchEntry *se = it.value();
        QCheckBox *cb = new QCheckBox(se->m_name);
        if (se->m_enable) {
            cb->setCheckState(Qt::Checked);
        } else {
            cb->setCheckState(Qt::Unchecked);
        }
        m_search_entry_checkbox_map.insert(it.key(), cb);
        input_layout->addWidget(cb);
    }
    input_group_box->setLayout(input_layout);
    return input_group_box;
}

QWidget* Layout::createSearchInputWidget() {
    QGroupBox *widget = new QGroupBox("検索条件");
    QHBoxLayout *input_layout = new QHBoxLayout();
    QVBoxLayout *input_layout_col1 = new QVBoxLayout();
    QVBoxLayout *input_layout_col2 = new QVBoxLayout();

    // JANコード
    m_input_jancode_le = new QLineEdit();
    input_layout_col1->addWidget(new QLabel("JANコード"));
    input_layout_col1->addWidget(m_input_jancode_le);

    // 商品名
    m_input_product_name_le = new QLineEdit();
    input_layout_col1->addWidget(new QLabel("商品名"));
    input_layout_col1->addWidget(m_input_product_name_le);

    // 商品番号
    m_input_product_code_le = new QLineEdit();
    input_layout_col1->addWidget(new QLabel("商品番号"));
    input_layout_col1->addWidget(m_input_product_code_le);

    // 検索ワード
    m_input_search_word_le = new QLineEdit();
    input_layout_col1->addWidget(new QLabel("検索ワード"));
    input_layout_col1->addWidget(m_input_search_word_le);

    // 備考1
    m_output_remarks_1_le = new QLineEdit();
    input_layout_col2->addWidget(new QLabel("備考1"));
    input_layout_col2->addWidget(m_output_remarks_1_le);

    // 備考2
    m_output_remarks_2_le = new QLineEdit();
    input_layout_col2->addWidget(new QLabel("備考2"));
    input_layout_col2->addWidget(m_output_remarks_2_le);

    // 備考3
    m_output_remarks_3_le = new QLineEdit();
    input_layout_col2->addWidget(new QLabel("備考3"));
    input_layout_col2->addWidget(m_output_remarks_3_le);

    // 備考4
    m_output_remarks_4_le = new QLineEdit();
    input_layout_col2->addWidget(new QLabel("備考4"));
    input_layout_col2->addWidget(m_output_remarks_4_le);

    // 備考5
    m_output_remarks_5_le = new QLineEdit();
    input_layout_col2->addWidget(new QLabel("備考5"));
    input_layout_col2->addWidget(m_output_remarks_5_le);

    input_layout_col1->addStretch();
    input_layout_col2->addStretch();
    input_layout->addLayout(input_layout_col1);
    input_layout->addLayout(input_layout_col2);
    widget->setLayout(input_layout);
    widget->setMinimumHeight(MIN_HEIGHT_SEARCH_INPUT);
    return widget;
}

QWidget* Layout::createOutputInputWidget() {
    QGroupBox *widget = new QGroupBox("出力オプション");
    QVBoxLayout *input_layout = new QVBoxLayout();

    // 出力先フォルダパステキストボックス
    m_output_folder_le = new QLineEdit();
    if (m_ini->empty("global/output_folder")) {
        m_output_folder_le->insert(m_here);
    } else {
        m_output_folder_le->insert(m_ini->get("global/output_folder").toString());
    }

    // 出力先フォルダ選択ボタン
    input_layout->addWidget(new QLabel("出力先フォルダ"));
    QHBoxLayout *output_folder_layout = new QHBoxLayout();
    m_output_folder_pb = new QPushButton("フォルダ選択");
    m_output_folder_pb->resize(30, 30);
    m_output_folder_pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(m_output_folder_pb, SIGNAL(clicked(bool)), this, SLOT(callbackSelectOutputFolder()));
    output_folder_layout->addWidget(m_output_folder_le);
    output_folder_layout->addWidget(m_output_folder_pb);
    input_layout->addLayout(output_folder_layout);

    // Wwlテンプレートフォルダテキストボックス
    m_output_wwl_folder_le = new QLineEdit();
    if (m_ini->empty("global/wwl_folder")) {
        m_output_wwl_folder_le->insert(m_here + "\\conf\\wwl");
    } else {
        m_output_wwl_folder_le->insert(m_ini->get("global/wwl_folder").toString());
    }

    // Wwlテンプレートフォルダ選択ボタン
    input_layout->addWidget(new QLabel("wwlテンプレートフォルダ"));
    QHBoxLayout *wwl_template_folder_layout = new QHBoxLayout();
    m_output_wwl_folder_pb = new QPushButton("フォルダ選択");
    m_output_wwl_folder_pb->resize(30, 30);
    m_output_wwl_folder_pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(m_output_wwl_folder_pb, SIGNAL(clicked(bool)), this, SLOT(callbackSelectWwlTemplateFolder()));
    wwl_template_folder_layout->addWidget(m_output_wwl_folder_le);
    wwl_template_folder_layout->addWidget(m_output_wwl_folder_pb);
    input_layout->addLayout(wwl_template_folder_layout);

    // 商品CSVテキストボックス
    m_output_csv_file_le = new QLineEdit();
    if (m_ini->empty("global/csv_file")) {
        m_output_csv_file_le->insert(m_here + "\\conf\\product.csv");
    } else {
        m_output_csv_file_le->insert(m_ini->get("global/csv_file").toString());
    }

    // 商品CSV選択ボタン
    input_layout->addWidget(new QLabel("商品csvファイル"));
    QHBoxLayout *csv_file_layout = new QHBoxLayout();
    m_output_csv_file_pb = new QPushButton("ファイル選択");
    m_output_csv_file_pb->resize(30, 30);
    m_output_csv_file_pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(m_output_csv_file_pb, SIGNAL(clicked(bool)), this, SLOT(callbackSelectCsvFile()));
    csv_file_layout->addWidget(m_output_csv_file_le);
    csv_file_layout->addWidget(m_output_csv_file_pb);
    input_layout->addLayout(csv_file_layout);

    widget->setLayout(input_layout);
    return widget;
}

QBoxLayout* Layout::createSubmitLayout() {
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(10);

    m_submit_pb = new QPushButton("実行");
    m_submit_pb->resize(30, 30);
    m_submit_pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addWidget(m_submit_pb);

    layout->setAlignment(Qt::AlignRight);
    return layout;
}

void Layout::setCallbackSubmit(QObject* obj, const char* slot) {
    QObject::connect(m_submit_pb, SIGNAL(clicked(bool)), obj, slot);
}

QWidget* Layout::createLogWidget() {
    QGroupBox *log_group_box = new QGroupBox("メッセージ");
    QVBoxLayout *log_layout = new QVBoxLayout();

    // ログ出力エリア
    m_log_te = new QTextEdit();
    m_log_te->setReadOnly(true);
    m_log_te->setUndoRedoEnabled(false);
    //m_log_te->setDisabled(true);
    // ログオブジェクトに紐付け
    m_log->setDisplayWidget(m_log_te);

    log_layout->addWidget(m_log_te);
    log_group_box->setLayout(log_layout);
    log_group_box->setMaximumHeight(MAX_HEIGHT_LOG);
    return log_group_box;
}

void Layout::callbackSelectOutputFolder() {
    QFileDialog fileDialog(m_main);
    fileDialog.setFileMode(QFileDialog::Directory);
    fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
    if(fileDialog.exec()){
        QString folder = fileDialog.selectedFiles().join("\\").replace(QString("/"), QString("\\"));
        m_output_folder_le->setText(folder);
    }
}

void Layout::callbackSelectWwlTemplateFolder() {
    QFileDialog fileDialog(m_main);
    fileDialog.setFileMode(QFileDialog::Directory);
    if(fileDialog.exec()){
        QString template_folder = fileDialog.selectedFiles().join("\\").replace(QString("/"), QString("\\"));
        m_output_wwl_folder_le->setText(template_folder);
    }
}

void Layout::callbackSelectCsvFile() {
    QFileDialog fileDialog(m_main);
    if(fileDialog.exec()){
        QString csv_file = fileDialog.selectedFiles().join("\\").replace(QString("/"), QString("\\"));
        m_output_csv_file_le->setText(csv_file);
    }
}
