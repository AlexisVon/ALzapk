// Copyright (c) 2026 AlexisVon

#pragma once

#include <QWidget>

class QLabel;

class LzaStatusBar : public QWidget {
    Q_OBJECT

public:
    explicit LzaStatusBar(QWidget* _parent = nullptr);

    void show_message(const QString& _msg);
    void set_speed(double _bytes_per_s);
    void set_eta(quint64 _ms);
    void set_sizes(quint64 _lza_bytes, quint64 _src_bytes);
    void reset();

private:
    QLabel* ui_lb_message{nullptr};
    QLabel* ui_lb_speed{nullptr};
    QLabel* ui_lb_eta{nullptr};
    QLabel* ui_lb_lza{nullptr};
    QLabel* ui_lb_src{nullptr};
};
