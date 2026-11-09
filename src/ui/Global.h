// Copyright (c) 2026 AlexisVon

#pragma once

#include <QString>
#include <QWidget>

#include <core/format.h>

inline QString size2string(quint64 _size) {
    return QString::fromStdString(lza::size_string(_size));
}

inline QString time2string(quint64 _ms) {
    return QString::fromStdString(lza::time_string(_ms));
}

class LZAWidget : public QWidget {
    Q_OBJECT

public:
    explicit LZAWidget(QWidget* _parent = nullptr) : QWidget(_parent) {}
    ~LZAWidget() override = default;

    void set_path(const QString& _path) { m_fpath = _path; }
    virtual void activate() {}

signals:
    void sig_busy(bool _busy);
    void sig_message(const QString& _msg);
    void sig_speed(double _bytes_per_s);
    void sig_eta(quint64 _ms);
    void sig_sizes(quint64 _lza_bytes, quint64 _src_bytes);
    void sig_progress(int _percent);
    void sig_log(const QString& _line);
    void sig_done();

protected:
    QString m_fpath;
};
