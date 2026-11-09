// Copyright (c) 2026 AlexisVon

#pragma once

#include <QWidget>

class QLabel;
class QProgressBar;
class QPushButton;

class TaskControl : public QWidget {
    Q_OBJECT

public:
    explicit TaskControl(const QString& _idle_text, QWidget* _parent = nullptr);

    void set_running(bool _running);
    void set_progress(int _percent);
    void reset();

signals:
    void sig_run();
    void sig_abort();

private:
    QProgressBar* ui_pb{nullptr};
    QLabel* ui_lb_percent{nullptr};
    QPushButton* ui_btn{nullptr};
    QString m_idle_text;
    bool m_running{false};
};
