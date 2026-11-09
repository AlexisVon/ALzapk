// Copyright (c) 2026 AlexisVon

#include "LzaStatusBar.h"
#include "Global.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>

LzaStatusBar::LzaStatusBar(QWidget* _parent)
    : QWidget(_parent) {
    setObjectName("statusbar");
    setFixedHeight(24);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(10);

    ui_lb_message = new QLabel(this);
    ui_lb_message->setObjectName("status_message");
    layout->addWidget(ui_lb_message, 1);

    auto add_separator = [this, layout]() {
        auto* separator = new QFrame(this);
        separator->setObjectName("sep");
        separator->setFixedSize(1, 12);
        layout->addWidget(separator);
    };

    ui_lb_speed = new QLabel(this);
    ui_lb_eta = new QLabel(this);
    ui_lb_lza = new QLabel(this);
    ui_lb_src = new QLabel(this);
    layout->addWidget(ui_lb_speed);
    add_separator();
    layout->addWidget(ui_lb_eta);
    add_separator();
    layout->addWidget(ui_lb_lza);
    add_separator();
    layout->addWidget(ui_lb_src);
    reset();
}

void LzaStatusBar::show_message(const QString& _msg) {
    ui_lb_message->setText(_msg);
}

void LzaStatusBar::set_speed(double _bytes_per_s) {
    ui_lb_speed->setText(_bytes_per_s > 0.0 ? QString(u8"速度 %1/s").arg(size2string(quint64(_bytes_per_s))) : QString(u8"速度 —"));
}

void LzaStatusBar::set_eta(quint64 _ms) {
    ui_lb_eta->setText(_ms > 0 ? QString(u8"剩余 %1").arg(time2string(_ms)) : QString(u8"剩余 —"));
}

void LzaStatusBar::set_sizes(quint64 _lza_bytes, quint64 _src_bytes) {
    ui_lb_lza->setText(_lza_bytes > 0 ? QString(u8"压缩包 %1").arg(size2string(_lza_bytes)) : QString(u8"压缩包 —"));
    ui_lb_src->setText(_src_bytes > 0 ? QString(u8"原始 %1").arg(size2string(_src_bytes)) : QString(u8"原始 —"));
}

void LzaStatusBar::reset() {
    ui_lb_message->clear();
    set_speed(0.0);
    set_eta(0);
    set_sizes(0, 0);
}
