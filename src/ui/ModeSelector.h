// Copyright (c) 2026 AlexisVon

#pragma once

#include <QWidget>

class QPushButton;

class ModeSelector : public QWidget {
    Q_OBJECT

public:
    enum Mode { Encode = 0,
                Decode = 1 };
    Q_ENUM(Mode)

    explicit ModeSelector(QWidget* _parent = nullptr);

    Mode mode() const;
    void set_mode(Mode _mode);

signals:
    void sig_mode_changed(Mode _mode);

private:
    QPushButton* ui_btn_encode{nullptr};
    QPushButton* ui_btn_decode{nullptr};
};
