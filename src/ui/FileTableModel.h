// Copyright (c) 2026 AlexisVon

#pragma once

#include <QAbstractTableModel>
#include <QVector>

class FileTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column { ColumnPath = 0,
                  ColumnName,
                  ColumnType,
                  ColumnSize,
                  ColumnRemove,
                  ColumnCount };

    explicit FileTableModel(QObject* _parent = nullptr);

    int rowCount(const QModelIndex& _parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& _parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& _index, int _role) const override;
    QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const override;
    Qt::ItemFlags flags(const QModelIndex& _index) const override;
    bool setData(const QModelIndex& _index, const QVariant& _value, int _role) override;
    bool removeRows(int _row, int _count, const QModelIndex& _parent = QModelIndex()) override;

    void add_paths(const QStringList& _paths);
    void clear();
    QList<QPair<QString, QString>> entries() const;
    quint64 total_size() const;
    bool has_duplicate_name(int& _row) const;

signals:
    void sig_message(const QString& _msg);

private:
    struct Entry {
        QString path;
        QString name;
        quint64 size;
        bool dir;
    };

    QVector<Entry> m_entries;
};
