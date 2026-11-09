// Copyright (c) 2026 AlexisVon

#pragma once

#include <QRect>
#include <QWidget>

class DecodeWidget;
class EncodeWidget;
class LzaStatusBar;
class LzaTitleBar;
class ModeSelector;
class QLineEdit;
class QPushButton;
class QStackedWidget;

class AlexisLza : public QWidget {
    Q_OBJECT

public:
    explicit AlexisLza(QWidget* _parent = nullptr);
    ~AlexisLza() override;

protected:
    void mouseMoveEvent(QMouseEvent* _ev) override;
    void mousePressEvent(QMouseEvent* _ev) override;
    void mouseReleaseEvent(QMouseEvent* _ev) override;
    void changeEvent(QEvent* _ev) override;

private:
    void init_ui();
    void init_signal();
    void init_argv();
    void update_mode();
    void set_busy(bool _busy);
    void on_browse();
    void on_maximize();
    void on_path_dropped(const QString& _path);
    int edge_at(const QPoint& _pos) const;
    void apply_edge_cursor(int _edge);
    void do_resize(const QPoint& _global_pos);

private:
    LzaTitleBar* ui_title{nullptr};
    ModeSelector* ui_mode{nullptr};
    QPushButton* ui_btn_path{nullptr};
    QLineEdit* ui_le_path{nullptr};
    QStackedWidget* ui_sw{nullptr};
    EncodeWidget* ui_encode{nullptr};
    DecodeWidget* ui_decode{nullptr};
    LzaStatusBar* ui_status{nullptr};

    int m_resize_edge{0};
    QRect m_resize_rect;
    QPoint m_resize_pos;
};
