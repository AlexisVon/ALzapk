// Copyright (c) 2026 AlexisVon

#include "EncodeWidget.h"
#include "FileTableModel.h"
#include "TaskControl.h"

#include <functional>

#include <core/pack.h>

#include <QCheckBox>
#include <QDragEnterEvent>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTextEdit>
#include <QHeaderView>

namespace {
    class RemoveDelegate : public QStyledItemDelegate {
    public:
        explicit RemoveDelegate(std::function<bool()> _can_remove, QObject* _parent = nullptr)
            : QStyledItemDelegate(_parent), m_can_remove(std::move(_can_remove)) {}

        void paint(QPainter* _painter, const QStyleOptionViewItem& _option, const QModelIndex& _index) const override {
            QStyleOptionViewItem option = _option;
            initStyleOption(&option, _index);
            option.text.clear();
            QStyledItemDelegate::paint(_painter, option, _index);

            _painter->save();
            _painter->setRenderHint(QPainter::Antialiasing, true);
            _painter->setPen(QPen(_option.state & QStyle::State_MouseOver ? QColor("#d75f5f") : QColor("#b4b4b4"), 1.4));
            const QPoint center = _option.rect.center();
            _painter->drawLine(center.x() - 4, center.y() - 4, center.x() + 4, center.y() + 4);
            _painter->drawLine(center.x() - 4, center.y() + 4, center.x() + 4, center.y() - 4);
            _painter->restore();
        }

        bool editorEvent(QEvent* _ev, QAbstractItemModel* _model, const QStyleOptionViewItem& _option, const QModelIndex& _index) override {
            if (_ev->type() == QEvent::MouseButtonRelease) {
                if (m_can_remove()) _model->removeRow(_index.row());
                return true;
            }
            return QStyledItemDelegate::editorEvent(_ev, _model, _option, _index);
        }

    private:
        std::function<bool()> m_can_remove;
    };
}

EncodeWidget::EncodeWidget(QWidget* _parent)
    : LZAWidget(_parent) {
    init_ui();
    init_signal();
}

EncodeWidget::~EncodeWidget() {
    m_abort = true;
    if (m_task.valid()) m_task.get();
}

void EncodeWidget::activate() {
    update_sizes();
}

void EncodeWidget::dragEnterEvent(QDragEnterEvent* _ev) {
    if (!m_running && _ev->mimeData()->hasUrls()) _ev->acceptProposedAction();
}

void EncodeWidget::dropEvent(QDropEvent* _ev) {
    if (m_running || !_ev->mimeData()->hasUrls()) return;

    QStringList paths;
    for (const QUrl& url : _ev->mimeData()->urls())
        if (url.isLocalFile()) paths.append(url.toLocalFile());
    ui_model->add_paths(paths);
    _ev->acceptProposedAction();
}

void EncodeWidget::init_ui() {
    setAcceptDrops(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 6, 0, 4);
    layout->setSpacing(6);

    auto* options = new QWidget(this);
    auto* options_layout = new QHBoxLayout(options);
    options_layout->setContentsMargins(0, 0, 0, 0);
    options_layout->setSpacing(10);

    ui_cb_compress = new QCheckBox(u8"启用压缩", options);
    ui_cb_encrypt = new QCheckBox(u8"启用加密", options);

    ui_fm_password = new QFrame(options);
    ui_fm_password->setObjectName("password_box");
    auto* password_layout = new QHBoxLayout(ui_fm_password);
    password_layout->setContentsMargins(6, 2, 4, 2);
    password_layout->setSpacing(6);
    ui_le_password = new QLineEdit(ui_fm_password);
    ui_le_password->setEchoMode(QLineEdit::Password);
    ui_le_password->setContextMenuPolicy(Qt::NoContextMenu);
    ui_le_password->setFixedWidth(200);
    ui_btn_showpwd = new QPushButton(u8"显示", ui_fm_password);
    ui_btn_showpwd->setObjectName("pwd_toggle");
    ui_btn_showpwd->setCheckable(true);
    password_layout->addWidget(ui_le_password);
    password_layout->addWidget(ui_btn_showpwd);
    ui_fm_password->setEnabled(false);

    ui_cb_igerr = new QCheckBox(u8"错误继续", options);

    options_layout->addWidget(ui_cb_compress);
    options_layout->addWidget(ui_cb_encrypt);
    options_layout->addWidget(ui_fm_password);
    options_layout->addStretch(1);
    options_layout->addWidget(ui_cb_igerr);
    layout->addWidget(options);

    auto* control = new QWidget(this);
    auto* control_layout = new QHBoxLayout(control);
    control_layout->setContentsMargins(0, 0, 0, 0);
    control_layout->setSpacing(6);
    ui_btn_addfile = new QPushButton(u8"添加文件", control);
    ui_btn_addfolder = new QPushButton(u8"添加目录", control);
    ui_btn_clear = new QPushButton(u8"清空所有", control);
    ui_task = new TaskControl(u8"开始执行", control);
    for (QPushButton* btn : {ui_btn_addfile, ui_btn_addfolder, ui_btn_clear}) btn->setMinimumSize(76, 24);
    control_layout->addWidget(ui_btn_addfile);
    control_layout->addWidget(ui_btn_addfolder);
    control_layout->addWidget(ui_btn_clear);
    control_layout->addWidget(ui_task, 1);
    layout->addWidget(control);

    ui_model = new FileTableModel(this);
    ui_view = new QTableView(this);
    ui_view->setModel(ui_model);
    ui_view->setShowGrid(false);
    ui_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui_view->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui_view->verticalHeader()->setVisible(false);
    ui_view->verticalHeader()->setDefaultSectionSize(22);
    ui_view->horizontalHeader()->setStretchLastSection(false);
    ui_view->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui_view->horizontalHeader()->setSectionResizeMode(FileTableModel::ColumnPath, QHeaderView::Stretch);
    ui_view->horizontalHeader()->setSectionResizeMode(FileTableModel::ColumnName, QHeaderView::ResizeToContents);
    ui_view->horizontalHeader()->setSectionResizeMode(FileTableModel::ColumnType, QHeaderView::ResizeToContents);
    ui_view->horizontalHeader()->setSectionResizeMode(FileTableModel::ColumnSize, QHeaderView::ResizeToContents);
    ui_view->horizontalHeader()->setSectionResizeMode(FileTableModel::ColumnRemove, QHeaderView::Fixed);
    ui_view->setColumnWidth(FileTableModel::ColumnRemove, 52);
    ui_view->setItemDelegateForColumn(FileTableModel::ColumnRemove, new RemoveDelegate([this]() { return !m_running; }, this));

    auto* empty = new QLabel(u8"把文件或文件夹拖到这里，或点击「添加文件」", this);
    empty->setObjectName("empty_hint");
    empty->setAlignment(Qt::AlignCenter);

    ui_stack = new QStackedWidget(this);
    ui_stack->addWidget(empty);
    ui_stack->addWidget(ui_view);
    layout->addWidget(ui_stack, 1);

    ui_log = new QTextEdit(this);
    ui_log->setObjectName("log");
    ui_log->setReadOnly(true);
    ui_log->setFixedHeight(80);
    ui_log->setVisible(false);
    layout->addWidget(ui_log);
}

void EncodeWidget::init_signal() {
    connect(ui_cb_encrypt, &QCheckBox::toggled, this, [this](bool _checked) { ui_fm_password->setEnabled(_checked); });
    connect(ui_btn_showpwd, &QPushButton::toggled, this,
            [this](bool _checked) { ui_le_password->setEchoMode(_checked ? QLineEdit::Normal : QLineEdit::Password); });

    connect(ui_btn_addfile, &QPushButton::clicked, this, &EncodeWidget::on_add_files);
    connect(ui_btn_addfolder, &QPushButton::clicked, this, &EncodeWidget::on_add_folder);
    connect(ui_btn_clear, &QPushButton::clicked, this, &EncodeWidget::on_clear);
    connect(ui_task, &TaskControl::sig_run, this, &EncodeWidget::on_run_clicked);
    connect(ui_task, &TaskControl::sig_abort, this, &EncodeWidget::on_abort_clicked);
    connect(this, &EncodeWidget::sig_progress, ui_task, &TaskControl::set_progress);
    connect(this, &EncodeWidget::sig_log, this, [this](const QString& _line) {
        ui_log->setVisible(true);
        ui_log->append(_line);
    });
    connect(this, &EncodeWidget::sig_done, this, &EncodeWidget::on_run_finished);

    auto* remove_shortcut = new QShortcut(QKeySequence::Delete, ui_view);
    remove_shortcut->setContext(Qt::WidgetShortcut);
    connect(remove_shortcut, &QShortcut::activated, this, &EncodeWidget::on_remove_selected);

    connect(ui_model, &FileTableModel::sig_message, this, &EncodeWidget::sig_message);

    auto refresh = [this]() {
        update_sizes();
        update_empty_state();
    };
    connect(ui_model, &QAbstractItemModel::rowsInserted, this, refresh);
    connect(ui_model, &QAbstractItemModel::rowsRemoved, this, refresh);
    connect(ui_model, &QAbstractItemModel::modelReset, this, refresh);
}

void EncodeWidget::update_sizes() {
    emit sig_sizes(0, ui_model->total_size());
}

void EncodeWidget::update_empty_state() {
    ui_stack->setCurrentIndex(ui_model->rowCount() > 0 ? 1 : 0);
}

void EncodeWidget::set_busy(bool _busy) {
    ui_cb_compress->setEnabled(!_busy);
    ui_cb_encrypt->setEnabled(!_busy);
    ui_fm_password->setEnabled(!_busy && ui_cb_encrypt->isChecked());
    ui_cb_igerr->setEnabled(!_busy);
    ui_btn_addfile->setEnabled(!_busy);
    ui_btn_addfolder->setEnabled(!_busy);
    ui_btn_clear->setEnabled(!_busy);
    ui_view->setEditTriggers(_busy ? QAbstractItemView::NoEditTriggers : QAbstractItemView::DoubleClicked);
    emit sig_busy(_busy);
}

void EncodeWidget::on_add_files() {
    static QString last_path;
    const QStringList paths = QFileDialog::getOpenFileNames(
        this, u8"选择要添加的文件(可多选)", last_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : last_path);
    if (paths.isEmpty()) return;
    last_path = paths.last();
    ui_model->add_paths(paths);
}

void EncodeWidget::on_add_folder() {
    static QString last_path;
    const QString path = QFileDialog::getExistingDirectory(
        this, u8"选择要添加的文件夹", last_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : last_path);
    if (path.isEmpty()) return;
    last_path = path;
    ui_model->add_paths({path});
}

void EncodeWidget::on_clear() {
    ui_model->clear();
    ui_log->clear();
    ui_log->setVisible(false);
    ui_task->reset();
}

void EncodeWidget::on_remove_selected() {
    if (m_running) return;
    const QModelIndexList selected = ui_view->selectionModel()->selectedRows();
    for (int i = selected.size() - 1; i >= 0; i--) ui_model->removeRows(selected.at(i).row(), 1);
}

void EncodeWidget::on_run_clicked() {
    if (m_fpath.isEmpty()) return emit sig_message(u8"输出文件路径为空");

    int duplicate_row{-1};
    if (ui_model->has_duplicate_name(duplicate_row)) return emit sig_message(QString(u8"第%1行出现重复名称").arg(duplicate_row + 1));

    m_args.fpath = m_fpath;
    m_args.paths = ui_model->entries();
    m_args.total = ui_model->total_size();
    m_args.compress = ui_cb_compress->isChecked();
    m_args.igerr = ui_cb_igerr->isChecked();
    m_args.password = ui_cb_encrypt->isChecked() ? ui_le_password->text().toStdString() : std::string();

    ui_log->clear();
    ui_log->setVisible(false);
    ui_task->reset();
    ui_task->set_running(true);
    m_abort = false;
    m_running = true;
    set_busy(true);
    m_task = std::async(std::bind(&EncodeWidget::exec_impl, this));
}

void EncodeWidget::on_abort_clicked() {
    m_abort = true;
    if (m_task.valid()) m_task.get();
}

void EncodeWidget::on_run_finished() {
    m_running = false;
    ui_task->set_running(false);
    set_busy(false);
    emit sig_speed(0.0);
    emit sig_eta(0);
    if (m_task.valid()) m_task.get();
}

void EncodeWidget::exec_impl() {
    QElapsedTimer total_cost;
    total_cost.start();

    std::vector<lza::pack_item> items;
    items.reserve(m_args.paths.size());
    for (const auto& path : m_args.paths)
        items.push_back({path.first.toLocal8Bit().toStdString(), path.second.toLocal8Bit().toStdString()});

    lza::pack_options options;
    options.compress = m_args.compress;
    options.ignore_errors = m_args.igerr;
    options.password = m_args.password;

    const auto on_file = [this](const std::string& _file, uint64_t _total, uint64_t _current) {
        if (_total == uint64_t(-1)) {
            const QString line = QString(u8"\"%1\"编码失败，错误：%2")
                                     .arg(QString::fromLocal8Bit(_file.c_str(), _file.size()))
                                     .arg(lza::pack_error(uint8_t(_current)));
            emit sig_message(line);
            emit sig_log(line);
            return;
        }
        emit sig_message(QString("\"%1\" %2%").arg(QString::fromLocal8Bit(_file.c_str(), _file.size())).arg(_current * 100.0 / _total, 0, 'f', 2));
    };
    const auto on_progress = [this](const lza::pack_progress& _progress) {
        emit sig_speed(_progress.stat.speed_bps);
        emit sig_progress(_progress.stat.percent);
        emit sig_eta(_progress.stat.eta_ms);
        emit sig_sizes(_progress.archive_bytes, _progress.source_bytes);
    };

    const QString archive = m_args.fpath;
    const lza::pack_result result = lza::pack(archive.toLocal8Bit().toStdString(), items, options, &m_abort, on_file, on_progress);
    switch (result.state) {
    case lza::pack_result::ok:
        emit sig_progress(100);
        emit sig_sizes(QFileInfo(archive).size(), m_args.total);
        emit sig_message(QString(u8"文件\"%1\"写入完成，耗时：%2").arg(archive).arg(time2string(total_cost.elapsed())));
        break;
    case lza::pack_result::aborted: emit sig_message(u8"已中断"); break;
    case lza::pack_result::open_failed: emit sig_message(QString(u8"文件\"%1\"无法写入").arg(archive)); break;
    case lza::pack_result::append_failed:
        emit sig_message(QString(u8"文件\"%1\"写入失败，错误位置：\"%2\"，错误：%3")
                             .arg(archive)
                             .arg(QString::fromLocal8Bit(result.failed_file.c_str(), result.failed_file.size()))
                             .arg(lza::pack_error(result.error)));
        break;
    case lza::pack_result::finish_failed: emit sig_message(QString(u8"文件\"%1\"写入失败，文件尾编码失败").arg(archive)); break;
    }

    emit sig_done();
}
