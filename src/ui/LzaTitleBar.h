// Copyright (c) 2026 AlexisVon

#pragma once

#include <QPushButton>
#include <QWidget>

class ChromeButton : public QPushButton {
    Q_OBJECT

public:
    enum Kind { Minimize,
                Maximize,
                Restore,
                Close };

    explicit ChromeButton(Kind _kind, QWidget* _parent = nullptr);

    void set_kind(Kind _kind);

protected:
    void paintEvent(QPaintEvent* _ev) override;

private:
    Kind m_kind;
};

class LzaTitleBar : public QWidget {
    Q_OBJECT

public:
    explicit LzaTitleBar(QWidget* _parent = nullptr);

    void set_leading(QWidget* _widget);
    void set_maximized(bool _maximized);

signals:
    void sig_minimize();
    void sig_maximize();
    void sig_close();

protected:
    void mousePressEvent(QMouseEvent* _ev) override;
    void mouseMoveEvent(QMouseEvent* _ev) override;
    void mouseReleaseEvent(QMouseEvent* _ev) override;
    void mouseDoubleClickEvent(QMouseEvent* _ev) override;

private:
    QWidget* ui_leading{nullptr};
    ChromeButton* ui_btn_min{nullptr};
    ChromeButton* ui_btn_max{nullptr};
    ChromeButton* ui_btn_close{nullptr};
    QPoint m_drag_offset;
    bool m_dragging{false};
};
