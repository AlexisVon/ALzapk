// Copyright (c) 2026 AlexisVon

#include "ModeSelector.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>

ModeSelector::ModeSelector(QWidget* _parent)
    : QWidget(_parent) {
    setObjectName("mode_box");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(0);

    ui_btn_encode = new QPushButton(u8"编码", this);
    ui_btn_decode = new QPushButton(u8"解码", this);
    for (QPushButton* btn : {ui_btn_encode, ui_btn_decode}) {
        btn->setObjectName("mode_seg");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(24);
        layout->addWidget(btn);
    }
    ui_btn_encode->setChecked(true);

    auto* group = new QButtonGroup(this);
    group->addButton(ui_btn_encode, Encode);
    group->addButton(ui_btn_decode, Decode);
    connect(group, &QButtonGroup::idClicked, this, [this](int _id) { emit sig_mode_changed(Mode(_id)); });
}

ModeSelector::Mode ModeSelector::mode() const {
    return ui_btn_decode->isChecked() ? Decode : Encode;
}

void ModeSelector::set_mode(Mode _mode) {
    (_mode == Decode ? ui_btn_decode : ui_btn_encode)->setChecked(true);
}
