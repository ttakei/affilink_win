#include "layoutold.h"
#include <QGroupBox>
#include <QFormLayout>
#include <QDir>
#include <QTextCodec>
#include <QLabel>
#include <QFileDialog>

/*
Layout::Layout(QObject *parent) :
    QObject(parent){}

Layout::~Layout(){}

void Layout::init(QWidget* main, Log* log, Ini* ini, SearchEntryManager* search, Wwl* wwl) {
    m_main = main;
    m_log = log;
    m_ini = ini;
    m_search = search;
    m_wwl = wwl;
    m_here = QDir(QDir::current()).canonicalPath().replace("/", "\\");
}

QBoxLayout* Layout::load() {
    QVBoxLayout *layout = new QVBoxLayout;

    // 操作エリア
    QHBoxLayout *input_layout = new QHBoxLayout;
    input_layout->addLayout(createSearchEntryLayout());
    input_layout->addLayout(createMainInputLayout());
    layout->addLayout(input_layout);

    // ログメッセージエリア
    layout->addLayout(createLogLayout());

    return layout;
}

bool Layout::loadInput() {
    if (!validate()) {
        return false;
    }

    // search
    for (QMap<QString, SearchEntry*>::iterator it = m_search->m_map.begin(); it != m_search->m_map.end(); it++) {
        SearchEntry* se = it.value();
        se->init(m_ini);
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
        // 設定更新
        m_ini->set(se->getIniGroup(), "enable", QString(se->m_enable));
    }

    // wwl
    m_wwl->m_dir = m_output_template_folder_le->text();

    return true;
}

bool Layout::validate() {
    // 出力先フォルダ
    if (m_output_folder_le->text().isEmpty()) {
        m_log->err("入力エラー", "出力先フォルダが空です");
        return false;
    }
    QDir dir(m_output_folder_le->text());
    if (!dir.exists()) {
        m_log->err("入力エラー", "出力先フォルダが存在しません");
        return false;
    }

    // 出力テンプレートフォルダ
    if (m_output_template_folder_le->text().isEmpty()) {
        m_log->err("入力エラー", "wwlテンプレートフォルダが空です");
        return false;
    }
    QDir dir_wwl(m_output_template_folder_le->text());
    if (!dir_wwl.exists()) {
        m_log->err("入力エラー", "wwlテンプレートフォルダが存在しません");
        return false;
    }

    return true;
}

QBoxLayout* Layout::createSearchEntryLayout() {
    QVBoxLayout *layout = new QVBoxLayout();
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

    layout->addWidget(input_group_box);
    return layout;
}

QBoxLayout* Layout::createMainInputLayout() {
    QVBoxLayout *layout = new QVBoxLayout;

    // 検索入力
    layout->addWidget(createSearchInputWidget());

    // 出力オプション
    layout->addWidget(createOutputInputWidget());

    // 実行ボタン
    layout->addLayout(createSubmitLayout());

    layout->addStretch();
    return layout;
}

QWidget* Layout::createSearchInputWidget() {
    QGroupBox *widget = new QGroupBox("検索条件");
    QVBoxLayout *input_layout = new QVBoxLayout();

    // 商品名
    m_input_product_name_le = new QLineEdit();
    input_layout->addWidget(new QLabel("商品名"));
    input_layout->addWidget(m_input_product_name_le);

    // JANコード
    m_input_jancode_le = new QLineEdit();
    input_layout->addWidget(new QLabel("JANコード"));
    input_layout->addWidget(m_input_jancode_le);

    // 検索ワード
    m_input_search_word_le = new QLineEdit();
    input_layout->addWidget(new QLabel("検索ワード"));
    input_layout->addWidget(m_input_search_word_le);

    // 品番コード
    m_input_product_code_le = new QLineEdit();
    input_layout->addWidget(new QLabel("品番コード"));
    input_layout->addWidget(m_input_product_code_le);

    // 備考1
    m_output_remarks_1_le = new QLineEdit();
    input_layout->addWidget(new QLabel("備考1"));
    input_layout->addWidget(m_output_remarks_1_le);

    // 備考2
    m_output_remarks_2_le = new QLineEdit();
    input_layout->addWidget(new QLabel("備考2"));
    input_layout->addWidget(m_output_remarks_2_le);

    // 備考3
    m_output_remarks_3_le = new QLineEdit();
    input_layout->addWidget(new QLabel("備考3"));
    input_layout->addWidget(m_output_remarks_3_le);

    // 備考4
    m_output_remarks_4_le = new QLineEdit();
    input_layout->addWidget(new QLabel("備考4"));
    input_layout->addWidget(m_output_remarks_4_le);

    // 備考5
    m_output_remarks_5_le = new QLineEdit();
    input_layout->addWidget(new QLabel("備考5"));
    input_layout->addWidget(m_output_remarks_5_le);

    widget->setLayout(input_layout);
    return widget;
}

QWidget* Layout::createOutputInputWidget() {
    QGroupBox *widget = new QGroupBox("出力オプション");
    QVBoxLayout *input_layout = new QVBoxLayout();

    // 出力先フォルダパステキストボックス
    m_output_folder_le = new QLineEdit();
    if (!m_ini->empty("global/output_folder")) {
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
    QObject::connect(m_output_folder_pb, SIGNAL(clicked(bool)), this, SLOT(selectOutputFolder()));
    output_folder_layout->addWidget(m_output_folder_le);
    output_folder_layout->addWidget(m_output_folder_pb);
    input_layout->addLayout(output_folder_layout);

    // Wllテンプレートフォルダテキストボックス
    m_output_template_folder_le = new QLineEdit();
    if (m_ini->empty("global/output_template_file")) {
        m_output_template_folder_le->insert(m_here + "\\conf\\wwl");
    } else {
        m_output_template_folder_le->insert(m_ini->get("output_template_file").toString());
    }

    // Wllテンプレートフォルダ選択ボタン
    input_layout->addWidget(new QLabel("wwlテンプレートフォルダ"));
    QHBoxLayout *wll_template_folder_layout = new QHBoxLayout();
    m_output_wll_folder_pb = new QPushButton("フォルダ選択");
    m_output_wll_folder_pb->resize(30, 30);
    m_output_wll_folder_pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(m_output_wll_folder_pb, SIGNAL(clicked(bool)), this, SLOT(selectWllTemplateFolder()));
    wll_template_folder_layout->addWidget(m_output_template_folder_le);
    wll_template_folder_layout->addWidget(m_output_wll_folder_pb);
    input_layout->addLayout(wll_template_folder_layout);

    widget->setLayout(input_layout);
    return widget;
}

QBoxLayout* Layout::createSubmitLayout() {
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(10);

    QPushButton *submit = new QPushButton("実行");
    submit->resize(30, 30);
    submit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(submit, SIGNAL(clicked(bool)), m_main, SLOT(run()));
    layout->addWidget(submit);

    layout->setAlignment(Qt::AlignRight);
    return layout;
}

QBoxLayout* Layout::createLogLayout() {
    QVBoxLayout *layout = new QVBoxLayout;

    // ログ出力エリア
    m_log_te = new QTextEdit();
    m_log_te->setReadOnly(true);

    // ログオブジェクトに紐付け
    m_log->setDisplayWidget(m_log_te);

    layout->addWidget(m_log_te);
    return layout;
}

void Layout::selectOutputFolder()
{
    QFileDialog fileDialog(m_main);
    fileDialog.setFileMode(QFileDialog::Directory);
    fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
    if(fileDialog.exec()){
        QString folder = fileDialog.selectedFiles().join("\\").replace(QString("/"), QString("\\"));
        m_output_folder_le->setText(folder);
        m_ini->set("global/output_folder", folder);
    }
}

void Layout::selectWllTemplateFolder()
{
    QFileDialog fileDialog(m_main);
    fileDialog.setFileMode(QFileDialog::Directory);
    if(fileDialog.exec()){
        QString template_folder = fileDialog.selectedFiles().join("\\").replace(QString("/"), QString("\\"));
        m_output_template_folder_le->setText(template_folder);
        m_ini->set("global/output_template_folder", template_folder);
    }
}
*/
