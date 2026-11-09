// Copyright (c) 2026 AlexisVon

#include "Alexis_Lza.h"
#include "DecodeWidget.h"
#include "EncodeWidget.h"
#include "LzaStatusBar.h"
#include "LzaTitleBar.h"
#include "ModeSelector.h"

#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QVBoxLayout>

AlexisLza::AlexisLza(QWidget* _parent)
    : QWidget(_parent) {
    init_ui();
    init_signal();
    init_argv();
}

AlexisLza::~AlexisLza() {
}

void AlexisLza::mouseMoveEvent(QMouseEvent* _ev) {
    if (m_resize_edge) return do_resize(_ev->globalPosition().toPoint());
    apply_edge_cursor(edge_at(_ev->position().toPoint()));
}

void AlexisLza::mousePressEvent(QMouseEvent* _ev) {
    if (_ev->button() != Qt::LeftButton) return QWidget::mousePressEvent(_ev);
    m_resize_edge = edge_at(_ev->position().toPoint());
    if (m_resize_edge) {
        m_resize_rect = geometry();
        m_resize_pos = _ev->globalPosition().toPoint();
    }
}

void AlexisLza::mouseReleaseEvent(QMouseEvent* _ev) {
    if (!m_resize_edge) return QWidget::mouseReleaseEvent(_ev);
    m_resize_edge = 0;
    apply_edge_cursor(edge_at(_ev->position().toPoint()));
}

void AlexisLza::changeEvent(QEvent* _ev) {
    if (QEvent::WindowStateChange == _ev->type() && ui_title) ui_title->set_maximized(isMaximized());
    QWidget::changeEvent(_ev);
}

void AlexisLza::init_ui() {
    setObjectName("root");
    setWindowFlags(Qt::FramelessWindowHint);
    setMouseTracking(true);
    setMinimumSize(640, 460);
    resize(760, 520);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(0);

    ui_title = new LzaTitleBar(this);
    auto* leading = new QWidget(ui_title);
    auto* leading_layout = new QHBoxLayout(leading);
    leading_layout->setContentsMargins(0, 0, 0, 0);
    leading_layout->setSpacing(8);
    auto* mark = new QLabel(leading);
    mark->setPixmap(QIcon(":/icon.ico").pixmap(16, 16));
    ui_mode = new ModeSelector(leading);
    leading_layout->addWidget(mark);
    leading_layout->addWidget(ui_mode);
    ui_title->set_leading(leading);
    layout->addWidget(ui_title);

    auto* path_row = new QWidget(this);
    path_row->setObjectName("pathrow");
    auto* path_layout = new QHBoxLayout(path_row);
    path_layout->setContentsMargins(0, 8, 0, 4);
    path_layout->setSpacing(8);
    ui_btn_path = new QPushButton(path_row);
    ui_btn_path->setMinimumWidth(76);
    ui_le_path = new QLineEdit(path_row);
    path_layout->addWidget(ui_btn_path);
    path_layout->addWidget(ui_le_path, 1);
    layout->addWidget(path_row);

    ui_encode = new EncodeWidget(this);
    ui_decode = new DecodeWidget(this);
    ui_sw = new QStackedWidget(this);
    ui_sw->addWidget(ui_encode);
    ui_sw->addWidget(ui_decode);
    layout->addWidget(ui_sw, 1);

    ui_status = new LzaStatusBar(this);
    layout->addWidget(ui_status);

    // the resize cursor lives on the root; each direct child claims its own so it cannot leak inside
    for (QWidget* child : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) child->setCursor(Qt::ArrowCursor);

    update_mode();
}

void AlexisLza::init_signal() {
    connect(ui_title, &LzaTitleBar::sig_minimize, this, &QWidget::showMinimized);
    connect(ui_title, &LzaTitleBar::sig_maximize, this, &AlexisLza::on_maximize);
    connect(ui_title, &LzaTitleBar::sig_close, this, &QWidget::close);

    connect(ui_mode, &ModeSelector::sig_mode_changed, this, [this](ModeSelector::Mode) { update_mode(); });
    connect(ui_btn_path, &QPushButton::clicked, this, &AlexisLza::on_browse);
    connect(ui_le_path, &QLineEdit::textChanged, this, [this](const QString& _path) {
        ui_encode->set_path(_path);
        ui_decode->set_path(_path);
    });

    auto wire = [this](LZAWidget* _page) {
        connect(_page, &LZAWidget::sig_message, ui_status, &LzaStatusBar::show_message);
        connect(_page, &LZAWidget::sig_speed, ui_status, &LzaStatusBar::set_speed);
        connect(_page, &LZAWidget::sig_eta, ui_status, &LzaStatusBar::set_eta);
        connect(_page, &LZAWidget::sig_sizes, ui_status, &LzaStatusBar::set_sizes);
        connect(_page, &LZAWidget::sig_busy, this, &AlexisLza::set_busy);
    };
    wire(ui_encode);
    wire(ui_decode);

    connect(ui_decode, &DecodeWidget::sig_path_dropped, this, &AlexisLza::on_path_dropped);
}

void AlexisLza::init_argv() {
    const QStringList args = QApplication::arguments();
    if (args.size() <= 1) return;
    ui_mode->set_mode(ModeSelector::Decode);
    update_mode();
    ui_le_path->setText(args.mid(1).join(' '));
}

void AlexisLza::update_mode() {
    const bool encode = ui_mode->mode() == ModeSelector::Encode;
    ui_sw->setCurrentWidget(encode ? static_cast<QWidget*>(ui_encode) : static_cast<QWidget*>(ui_decode));
    ui_btn_path->setText(encode ? u8"输出到" : u8"打开");
    ui_le_path->setPlaceholderText(encode ? u8"选择要生成的 .lza 文件" : u8"选择或拖入 .lza 文件");
    ui_status->reset();
    (encode ? static_cast<LZAWidget*>(ui_encode) : static_cast<LZAWidget*>(ui_decode))->activate();
}

void AlexisLza::set_busy(bool _busy) {
    ui_btn_path->setEnabled(!_busy);
    ui_le_path->setEnabled(!_busy);
    ui_mode->setEnabled(!_busy);
}

void AlexisLza::on_browse() {
    const bool encode = ui_mode->mode() == ModeSelector::Encode;
    const QString base =
        ui_le_path->text().isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : ui_le_path->text();
    const QString path = encode ? QFileDialog::getSaveFileName(this, u8"选择LZA文件输出路径，编辑名称", base, "LZA(*.lza)")
                                : QFileDialog::getOpenFileName(this, u8"选择要打开的LZA文件", base, "LZA(*.lza)");
    if (!path.isEmpty()) ui_le_path->setText(path);
}

void AlexisLza::on_maximize() {
    isMaximized() ? showNormal() : showMaximized();
}

void AlexisLza::on_path_dropped(const QString& _path) {
    ui_mode->set_mode(ModeSelector::Decode);
    update_mode();
    ui_le_path->setText(_path);
    ui_decode->open_file();
}

int AlexisLza::edge_at(const QPoint& _pos) const {
    if (isMaximized() || isFullScreen()) return 0;

    const int margin{4};
    int edge{0};
    if (_pos.x() <= margin) edge |= 1;
    else if (_pos.x() >= width() - margin - 1) edge |= 2;
    if (_pos.y() <= margin) edge |= 4;
    else if (_pos.y() >= height() - margin - 1) edge |= 8;
    return edge;
}

void AlexisLza::apply_edge_cursor(int _edge) {
    switch (_edge) {
    case 1:
    case 2: setCursor(Qt::SizeHorCursor); break;
    case 4:
    case 8: setCursor(Qt::SizeVerCursor); break;
    case 1 | 4:
    case 2 | 8: setCursor(Qt::SizeFDiagCursor); break;
    case 1 | 8:
    case 2 | 4: setCursor(Qt::SizeBDiagCursor); break;
    default: unsetCursor(); break;
    }
}

void AlexisLza::do_resize(const QPoint& _global_pos) {
    const QPoint delta = _global_pos - m_resize_pos;
    QRect rect = m_resize_rect;
    if (m_resize_edge & 1) rect.setLeft(rect.left() + delta.x());
    if (m_resize_edge & 2) rect.setRight(rect.right() + delta.x());
    if (m_resize_edge & 4) rect.setTop(rect.top() + delta.y());
    if (m_resize_edge & 8) rect.setBottom(rect.bottom() + delta.y());
    if (rect.width() < minimumWidth() || rect.height() < minimumHeight()) return;
    setGeometry(rect);
}
