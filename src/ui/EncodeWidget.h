// Copyright (c) 2026 AlexisVon

#pragma once

#include "Global.h"

#include <future>

class FileTableModel;
class QCheckBox;
class QComboBox;
class QFrame;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QTableView;
class QTextEdit;
class TaskControl;

class EncodeWidget : public LZAWidget {
    Q_OBJECT

public:
    explicit EncodeWidget(QWidget* _parent = nullptr);
    ~EncodeWidget() override;

    void activate() override;

protected:
    void dragEnterEvent(QDragEnterEvent* _ev) override;
    void dropEvent(QDropEvent* _ev) override;

private:
    void init_ui();
    void init_signal();

    void update_sizes();
    void update_empty_state();
    void set_busy(bool _busy);

    void on_add_files();
    void on_add_folder();
    void on_clear();
    void on_remove_selected();
    void on_run_clicked();
    void on_abort_clicked();
    void on_run_finished();

    void exec_impl();

private:
    struct EncodeTask {
        QString fpath;
        QList<QPair<QString, QString>> paths;
        quint64 total{0};
        bool compress{false};
        bool igerr{false};
        bool legacy_v1{false};
        std::string password;
    };

    FileTableModel* ui_model{nullptr};
    QTableView* ui_view{nullptr};
    QStackedWidget* ui_stack{nullptr};
    QCheckBox* ui_cb_compress{nullptr};
    QCheckBox* ui_cb_encrypt{nullptr};
    QCheckBox* ui_cb_igerr{nullptr};
    QComboBox* ui_cb_profile{nullptr};
    QFrame* ui_fm_password{nullptr};
    QLineEdit* ui_le_password{nullptr};
    QPushButton* ui_btn_showpwd{nullptr};
    QPushButton* ui_btn_addfile{nullptr};
    QPushButton* ui_btn_addfolder{nullptr};
    QPushButton* ui_btn_clear{nullptr};
    TaskControl* ui_task{nullptr};
    QTextEdit* ui_log{nullptr};

    EncodeTask m_args;
    std::future<void> m_task;
    bool m_abort{false};
    bool m_running{false};
};
