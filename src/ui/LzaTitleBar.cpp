// Copyright (c) 2026 AlexisVon

#include "LzaTitleBar.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

ChromeButton::ChromeButton(Kind _kind, QWidget* _parent)
    : QPushButton(_parent), m_kind(_kind) {
    setObjectName(_kind == Close ? "chrome_close" : "chrome");
    setFixedSize(32, 26);
    setFocusPolicy(Qt::NoFocus);
}

void ChromeButton::set_kind(Kind _kind) {
    if (m_kind == _kind) return;
    m_kind = _kind;
    update();
}

void ChromeButton::paintEvent(QPaintEvent* _ev) {
    QPushButton::paintEvent(_ev);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(QColor("#ebdbb2"), 1));
    const QPoint center = rect().center();
    switch (m_kind) {
    case Minimize: painter.drawLine(center.x() - 5, center.y(), center.x() + 5, center.y()); break;
    case Maximize: painter.drawRect(QRect(center.x() - 5, center.y() - 5, 10, 10)); break;
    case Restore:
        painter.drawRect(QRect(center.x() - 5, center.y() - 2, 8, 8));
        painter.drawLine(center.x() - 2, center.y() - 5, center.x() + 4, center.y() - 5);
        painter.drawLine(center.x() + 4, center.y() - 5, center.x() + 4, center.y() + 1);
        break;
    case Close:
        painter.drawLine(center.x() - 5, center.y() - 5, center.x() + 5, center.y() + 5);
        painter.drawLine(center.x() - 5, center.y() + 5, center.x() + 5, center.y() - 5);
        break;
    }
}

LzaTitleBar::LzaTitleBar(QWidget* _parent)
    : QWidget(_parent) {
    setObjectName("titlebar");
    setFixedHeight(30);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 0, 0);
    layout->setSpacing(10);

    ui_leading = new QWidget(this);
    auto* leading_layout = new QHBoxLayout(ui_leading);
    leading_layout->setContentsMargins(0, 0, 0, 0);
    leading_layout->setSpacing(8);
    layout->addWidget(ui_leading);
    layout->addStretch(1);

    auto* chrome = new QWidget(this);
    auto* chrome_layout = new QHBoxLayout(chrome);
    chrome_layout->setContentsMargins(0, 0, 0, 0);
    chrome_layout->setSpacing(0);
    ui_btn_min = new ChromeButton(ChromeButton::Minimize, chrome);
    ui_btn_max = new ChromeButton(ChromeButton::Maximize, chrome);
    ui_btn_close = new ChromeButton(ChromeButton::Close, chrome);
    chrome_layout->addWidget(ui_btn_min);
    chrome_layout->addWidget(ui_btn_max);
    chrome_layout->addWidget(ui_btn_close);
    layout->addWidget(chrome);

    connect(ui_btn_min, &QPushButton::clicked, this, &LzaTitleBar::sig_minimize);
    connect(ui_btn_max, &QPushButton::clicked, this, &LzaTitleBar::sig_maximize);
    connect(ui_btn_close, &QPushButton::clicked, this, &LzaTitleBar::sig_close);
}

void LzaTitleBar::set_leading(QWidget* _widget) {
    ui_leading->layout()->addWidget(_widget);
}

void LzaTitleBar::set_maximized(bool _maximized) {
    ui_btn_max->set_kind(_maximized ? ChromeButton::Restore : ChromeButton::Maximize);
}

void LzaTitleBar::mousePressEvent(QMouseEvent* _ev) {
    if (_ev->button() != Qt::LeftButton) return QWidget::mousePressEvent(_ev);
    m_drag_offset = _ev->globalPosition().toPoint() - window()->pos();
    m_dragging = true;
}

void LzaTitleBar::mouseMoveEvent(QMouseEvent* _ev) {
    if (!m_dragging) return;

    const QPoint global = _ev->globalPosition().toPoint();
    if (window()->isMaximized()) {
        const QRect geo = window()->geometry();
        const qreal ratio = qreal(global.x() - geo.left()) / qreal(geo.width());
        window()->showNormal();
        window()->move(global.x() - int(window()->width() * ratio), global.y() - m_drag_offset.y());
        m_drag_offset = global - window()->pos();
    } else if (global.y() <= 0) {
        m_dragging = false;
        emit sig_maximize();
    } else window()->move(global - m_drag_offset);
}

void LzaTitleBar::mouseReleaseEvent(QMouseEvent* _ev) {
    m_dragging = false;
    QWidget::mouseReleaseEvent(_ev);
}

void LzaTitleBar::mouseDoubleClickEvent(QMouseEvent* _ev) {
    if (_ev->button() != Qt::LeftButton) return;
    m_dragging = false;
    emit sig_maximize();
}
