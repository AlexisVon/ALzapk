// Copyright (c) 2026 AlexisVon

#pragma once

#include "Global.h"
#include "FSysTree.h"

#include <future>
#include <memory>

namespace lza {
    class archive;
}

class QAction;
class QFrame;
class QLineEdit;
class QMenu;
class QPushButton;
class QStackedWidget;
class QTextEdit;
class QTreeView;
class TaskControl;

class DecodeWidget : public LZAWidget {
    Q_OBJECT

public:
    explicit DecodeWidget(QWidget* _parent = nullptr);
    ~DecodeWidget() override;

    void activate() override;
    void open_file();

protected:
    void dragEnterEvent(QDragEnterEvent* _ev) override;
    void dropEvent(QDropEvent* _ev) override;

signals:
    void sig_path_dropped(const QString& _path);

private:
    void init_ui();
    void init_signal();

    void update_sizes();
    void update_empty_state();
    void set_busy(bool _busy);

    void on_abort_clicked();
    void on_finished();
    void on_context_menu(const QPoint& _pos);
    void on_export();
    void on_expand();
    void on_collapse();

    void export_impl();

private:
    struct ExportTask {
        QString output;
        std::string root;
        std::list<std::string> path;
        QString url;
        quint64 size{0};
    };

    QMenu* ui_menu{nullptr};
    QAction* ui_act_export{nullptr};
    QAction* ui_act_expand{nullptr};
    QAction* ui_act_collapse{nullptr};
    QAction* ui_act_export_all{nullptr};
    QAction* ui_act_expand_all{nullptr};
    QAction* ui_act_collapse_all{nullptr};
    QFrame* ui_fm_password{nullptr};
    QLineEdit* ui_le_password{nullptr};
    QPushButton* ui_btn_showpwd{nullptr};
    TaskControl* ui_task{nullptr};
    QTreeView* ui_view{nullptr};
    QStackedWidget* ui_stack{nullptr};
    QTextEdit* ui_log{nullptr};
    FSysTreeModel* ui_model{nullptr};

    QModelIndex ui_selected;
    std::unique_ptr<lza::archive> m_decoder;
    ExportTask m_args;
    std::future<void> m_task;
    bool m_abort{false};
    bool m_running{false};
};
