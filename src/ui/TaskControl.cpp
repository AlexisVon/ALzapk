// Copyright (c) 2026 AlexisVon

#include "TaskControl.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QStyle>

TaskControl::TaskControl(const QString& _idle_text, QWidget* _parent)
    : QWidget(_parent), m_idle_text(_idle_text) {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    ui_pb = new QProgressBar(this);
    ui_pb->setRange(0, 100);
    ui_pb->setTextVisible(false);
    ui_pb->setMinimumWidth(120);
    ui_pb->setMinimumHeight(18);
    ui_pb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    ui_lb_percent = new QLabel(this);
    ui_lb_percent->setObjectName("task_percent");
    ui_lb_percent->setFixedWidth(44);
    ui_lb_percent->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    ui_btn = new QPushButton(m_idle_text, this);
    ui_btn->setMinimumSize(88, 24);
    ui_btn->setProperty("primary", true);

    layout->addWidget(ui_pb);
    layout->addWidget(ui_lb_percent);
    layout->addWidget(ui_btn);

    connect(ui_btn, &QPushButton::clicked, this, [this]() { m_running ? emit sig_abort() : emit sig_run(); });
}

void TaskControl::set_running(bool _running) {
    m_running = _running;
    ui_btn->setText(_running ? u8"中断执行" : m_idle_text);
    ui_btn->setProperty("primary", !_running);
    ui_btn->style()->unpolish(ui_btn);
    ui_btn->style()->polish(ui_btn);
}

void TaskControl::set_progress(int _percent) {
    ui_pb->setValue(_percent);
    ui_lb_percent->setText(_percent > 0 ? QString("%1%").arg(_percent) : QString());
}

void TaskControl::reset() {
    set_running(false);
    ui_pb->setValue(0);
    ui_lb_percent->clear();
}
