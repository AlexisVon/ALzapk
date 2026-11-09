// Copyright (c) 2026 AlexisVon

#include "DecodeWidget.h"
#include "TaskControl.h"

#include <core/archive.h>

#include <QAction>
#include <QDragEnterEvent>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QPushButton>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTextEdit>
#include <QTreeView>

DecodeWidget::DecodeWidget(QWidget* _parent)
    : LZAWidget(_parent) {
    init_ui();
    init_signal();
}

DecodeWidget::~DecodeWidget() {
    m_abort = true;
    if (m_task.valid()) m_task.get();
}

void DecodeWidget::activate() {
    update_sizes();
}

void DecodeWidget::open_file() {
    if (m_running) return;
    if (m_fpath.isEmpty()) return emit sig_message(u8"输入文件路径为空");

    lza::open_result open_result;
    auto decoder = lza::archive::open(m_fpath.toLocal8Bit().toStdString(), ui_le_password->text().toStdString(), open_result);
    if (!decoder) {
        if (open_result == lza::open_result::need_password) emit sig_message(QString(u8"【加密文件】请输入密码再尝试打开"));
        else if (open_result == lza::open_result::bad_password) emit sig_message(QString(u8"【加密文件】输入密钥不正确"));
        else emit sig_message(QString(u8"无法打开文件"));
        return;
    }

    ui_model->setFSys(decoder->fsystem());
    ui_view->sortByColumn(0, Qt::SortOrder::AscendingOrder);
    m_decoder = std::move(decoder);
    emit sig_message(QString(u8"文件\"%1\"打开成功").arg(m_fpath));
    ui_log->clear();
    ui_log->setVisible(false);
    ui_task->reset();
    update_sizes();
    update_empty_state();
}

void DecodeWidget::dragEnterEvent(QDragEnterEvent* _ev) {
    if (!m_running && _ev->mimeData()->hasUrls()) _ev->acceptProposedAction();
}

void DecodeWidget::dropEvent(QDropEvent* _ev) {
    if (m_running || !_ev->mimeData()->hasUrls()) return;
    for (const QUrl& url : _ev->mimeData()->urls())
        if (url.isLocalFile()) {
            _ev->acceptProposedAction();
            return emit sig_path_dropped(url.toLocalFile());
        }
}

void DecodeWidget::init_ui() {
    setAcceptDrops(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 6, 0, 4);
    layout->setSpacing(6);

    auto* control = new QWidget(this);
    auto* control_layout = new QHBoxLayout(control);
    control_layout->setContentsMargins(0, 0, 0, 0);
    control_layout->setSpacing(6);

    ui_fm_password = new QFrame(control);
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

    ui_task = new TaskControl(u8"打开文件", control);

    control_layout->addWidget(ui_fm_password);
    control_layout->addWidget(ui_task, 1);
    layout->addWidget(control);

    ui_menu = new QMenu(this);
    ui_act_export = new QAction(u8"导出", ui_menu);
    ui_act_expand = new QAction(u8"展开", ui_menu);
    ui_act_collapse = new QAction(u8"折叠", ui_menu);
    ui_act_export_all = new QAction(u8"导出全部", ui_menu);
    ui_act_expand_all = new QAction(u8"展开全部", ui_menu);
    ui_act_collapse_all = new QAction(u8"折叠全部", ui_menu);
    ui_menu->addAction(ui_act_export);
    ui_menu->addAction(ui_act_export_all);
    ui_menu->addSeparator();
    ui_menu->addAction(ui_act_expand);
    ui_menu->addAction(ui_act_collapse);
    ui_menu->addAction(ui_act_expand_all);
    ui_menu->addAction(ui_act_collapse_all);

    ui_model = new FSysTreeModel(this);
    ui_view = new QTreeView(this);
    ui_view->setModel(ui_model);
    ui_view->setContextMenuPolicy(Qt::CustomContextMenu);
    ui_view->setSelectionMode(QAbstractItemView::SingleSelection);
    ui_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_view->setSortingEnabled(true);
    ui_view->header()->setStretchLastSection(false);
    ui_view->header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui_view->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui_view->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui_view->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui_view->header()->setMinimumSectionSize(40);

    auto* empty = new QLabel(u8"把 .lza 文件拖到这里，或点击「打开文件」", this);
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

void DecodeWidget::init_signal() {
    connect(ui_btn_showpwd, &QPushButton::toggled, this,
            [this](bool _checked) { ui_le_password->setEchoMode(_checked ? QLineEdit::Normal : QLineEdit::Password); });
    connect(ui_le_password, &QLineEdit::returnPressed, this, &DecodeWidget::open_file);
    connect(ui_task, &TaskControl::sig_run, this, &DecodeWidget::open_file);
    connect(ui_task, &TaskControl::sig_abort, this, &DecodeWidget::on_abort_clicked);
    connect(this, &DecodeWidget::sig_progress, ui_task, &TaskControl::set_progress);
    connect(this, &DecodeWidget::sig_log, this, [this](const QString& _line) {
        ui_log->setVisible(true);
        ui_log->append(_line);
    });
    connect(this, &DecodeWidget::sig_done, this, &DecodeWidget::on_finished);

    connect(ui_act_export, &QAction::triggered, this, &DecodeWidget::on_export);
    connect(ui_act_export_all, &QAction::triggered, this, [this]() {
        ui_selected = QModelIndex();
        on_export();
    });
    connect(ui_act_expand, &QAction::triggered, this, &DecodeWidget::on_expand);
    connect(ui_act_expand_all, &QAction::triggered, this, [this]() {
        ui_selected = QModelIndex();
        on_expand();
    });
    connect(ui_act_collapse, &QAction::triggered, this, &DecodeWidget::on_collapse);
    connect(ui_act_collapse_all, &QAction::triggered, this, [this]() {
        ui_selected = QModelIndex();
        on_collapse();
    });
    connect(ui_view, &QTreeView::customContextMenuRequested, this, &DecodeWidget::on_context_menu);
}

void DecodeWidget::update_sizes() {
    if (nullptr == m_decoder) {
        emit sig_sizes(0, 0);
        emit sig_speed(0.0);
        emit sig_eta(0);
    } else {
        emit sig_sizes(m_decoder->stream_bytes(), ui_model->root()->size());
        emit sig_speed(0.0);
        emit sig_eta(0);
    }
}

void DecodeWidget::update_empty_state() {
    ui_stack->setCurrentIndex(m_decoder ? 1 : 0);
}

void DecodeWidget::set_busy(bool _busy) {
    ui_fm_password->setEnabled(!_busy);
    emit sig_busy(_busy);
}

void DecodeWidget::on_abort_clicked() {
    m_abort = true;
    if (m_task.valid()) m_task.get();
}

void DecodeWidget::on_finished() {
    m_running = false;
    ui_task->set_running(false);
    set_busy(false);
    emit sig_speed(0.0);
    emit sig_eta(0);
    if (m_task.valid()) m_task.get();
}

void DecodeWidget::on_context_menu(const QPoint& _pos) {
    if (nullptr == m_decoder || m_running) return;

    const QModelIndex index = ui_view->indexAt(_pos);
    if (index.isValid()) {
        ui_selected = index;
        ui_act_export->setText(QString(u8"导出\"%1\"").arg(index.data(0).toString()));
        ui_act_export->setVisible(true);
        ui_act_export_all->setVisible(false);
        if (index.model()->hasChildren(index)) {
            const bool expanded = ui_view->isExpanded(index);
            ui_act_expand->setVisible(!expanded);
            ui_act_collapse->setVisible(expanded);
            ui_act_expand->setText(QString(u8"展开\"%1\"").arg(index.data(0).toString()));
            ui_act_collapse->setText(QString(u8"折叠\"%1\"").arg(index.data(0).toString()));
        } else {
            ui_act_expand->setVisible(false);
            ui_act_collapse->setVisible(false);
        }
        ui_act_expand_all->setVisible(false);
        ui_act_collapse_all->setVisible(false);
    } else {
        ui_selected = QModelIndex();
        ui_act_export->setVisible(false);
        ui_act_expand->setVisible(false);
        ui_act_collapse->setVisible(false);
        ui_act_export_all->setVisible(true);
        ui_act_expand_all->setVisible(true);
        ui_act_collapse_all->setVisible(true);
    }
    ui_menu->exec(ui_view->viewport()->mapToGlobal(_pos));
}

void DecodeWidget::on_export() {
    if (nullptr == m_decoder || m_running) return;

    static QString last_path;
    const QString output = QFileDialog::getExistingDirectory(
        this, u8"选择导出目录", last_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : last_path);
    if (output.isEmpty()) return;
    last_path = output;

    m_args.output = output;
    if (ui_selected.isValid()) {
        auto* item = (FSysTreeItem*) ui_selected.internalPointer();
        m_args.root = item->getRoot();
        m_args.path = item->getPath();
        m_args.size = item->size();
        m_args.url = FSysTreeItem::CalUrl(m_args.root, m_args.path);
    } else {
        m_args.root = "*";
        m_args.path.clear();
        m_args.size = ui_model->root()->size();
        m_args.url = "*";
    }

    ui_log->clear();
    ui_log->setVisible(false);
    ui_task->reset();
    ui_task->set_running(true);
    m_abort = false;
    m_running = true;
    set_busy(true);
    emit sig_message(QString(u8"数据\"%1\"正在导出到\"%2\"").arg(m_args.url).arg(output));

    m_task = std::async(std::bind(&DecodeWidget::export_impl, this));
}

void DecodeWidget::on_expand() {
    if (nullptr == m_decoder) return;
    if (ui_selected.isValid()) ui_view->expand(ui_selected);
    else ui_view->expandAll();
}

void DecodeWidget::on_collapse() {
    if (nullptr == m_decoder) return;
    if (ui_selected.isValid()) ui_view->collapse(ui_selected);
    else ui_view->collapseAll();
}

void DecodeWidget::export_impl() {
    QElapsedTimer total_cost;
    total_cost.start();

    const auto on_file = [this](const std::string& _file, uint64_t _total, uint64_t _current) {
        if (_total == uint64_t(-1)) {
            const QString line = QString(u8"\"%1\"提取失败，错误：%2")
                                     .arg(QString::fromLocal8Bit(_file.c_str(), _file.size()))
                                     .arg(lza::extract_error(uint8_t(_current)));
            emit sig_message(line);
            emit sig_log(line);
            return;
        }
        emit sig_message(QString("\"%1\" %2%").arg(QString::fromLocal8Bit(_file.c_str(), _file.size())).arg(_current * 100.0 / _total, 0, 'f', 2));
    };
    const auto on_progress = [this](const lza::extract_progress& _progress) {
        emit sig_speed(_progress.stat.speed_bps);
        emit sig_progress(_progress.stat.percent);
        emit sig_eta(_progress.stat.eta_ms);
    };

    const lza::extract_result result =
        m_decoder->extract(m_args.root, m_args.path, m_args.output.toLocal8Bit().toStdString(), &m_abort, on_file, on_progress);
    if (result.state == lza::extract_result::ok) {
        emit sig_progress(100);
        emit sig_message(QString(u8"数据\"%1\"导出到\"%2\"完成，耗时：%3").arg(m_args.url).arg(m_args.output).arg(time2string(total_cost.elapsed())));
    } else if (result.state == lza::extract_result::aborted) emit sig_message(u8"已中断");
    else emit sig_message(QString(u8"数据\"%1\"导出失败，错误位置：\"%2\"，错误：%3")
                              .arg(m_args.url)
                              .arg(QString::fromLocal8Bit(result.failed_file.c_str(), result.failed_file.size()))
                              .arg(lza::extract_error(result.error)));

    emit sig_done();
}
