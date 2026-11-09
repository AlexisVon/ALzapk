// Copyright (c) 2026 AlexisVon

#include "FileTableModel.h"
#include "Global.h"

#include <QFileInfo>
#include <QSet>

#include <afile.h>

FileTableModel::FileTableModel(QObject* _parent)
    : QAbstractTableModel(_parent) {
}

int FileTableModel::rowCount(const QModelIndex& _parent) const {
    return _parent.isValid() ? 0 : m_entries.size();
}

int FileTableModel::columnCount(const QModelIndex& _parent) const {
    (void) _parent;
    return ColumnCount;
}

QVariant FileTableModel::data(const QModelIndex& _index, int _role) const {
    if (!_index.isValid() || _index.row() >= m_entries.size()) return QVariant();
    const Entry& entry = m_entries.at(_index.row());
    switch (_role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (_index.column()) {
        case ColumnPath: return entry.path;
        case ColumnName: return entry.name;
        case ColumnType: return entry.dir ? u8"目录" : u8"文件";
        case ColumnSize: return size2string(entry.size);
        default: return QVariant();
        }
    case Qt::ToolTipRole: return _index.column() == ColumnName ? QVariant(entry.name) : QVariant();
    default: return QVariant();
    }
}

QVariant FileTableModel::headerData(int _section, Qt::Orientation _orientation, int _role) const {
    if (Qt::Horizontal != _orientation || Qt::DisplayRole != _role) return QVariant();
    static const char* titles[]{"路径", "名称", "类型", "大小", "移除"};
    return _section >= 0 && _section < ColumnCount ? QVariant(QString::fromUtf8(titles[_section])) : QVariant();
}

Qt::ItemFlags FileTableModel::flags(const QModelIndex& _index) const {
    Qt::ItemFlags result = QAbstractTableModel::flags(_index);
    return _index.column() == ColumnName ? result | Qt::ItemIsEditable : result;
}

bool FileTableModel::setData(const QModelIndex& _index, const QVariant& _value, int _role) {
    if (Qt::EditRole != _role || !_index.isValid() || _index.column() != ColumnName) return false;

    const QString name = _value.toString();
    if (name.isEmpty() || name == "." || name.contains("..") || name.contains('/') || name.contains('\\')) {
        emit sig_message(u8"仅允许输入文件名，不支持路径分隔符");
        return false;
    }
    m_entries[_index.row()].name = name;
    emit dataChanged(_index, _index);
    return true;
}

bool FileTableModel::removeRows(int _row, int _count, const QModelIndex& _parent) {
    if (_parent.isValid() || _row < 0 || _count <= 0 || _row + _count > m_entries.size()) return false;
    beginRemoveRows(QModelIndex(), _row, _row + _count - 1);
    m_entries.remove(_row, _count);
    endRemoveRows();
    return true;
}

void FileTableModel::add_paths(const QStringList& _paths) {
    QVector<Entry> added;
    for (const QString& path : _paths) {
        QFileInfo info(path);
        if (info.isSymLink()) {
            emit sig_message(u8"不支持添加文件链接");
            continue;
        }
        Entry entry;
        entry.path = info.absoluteFilePath();
        entry.name = info.fileName();
        entry.dir = info.isDir();
        entry.size = info.isFile() ? info.size() : alx::file_info::calsize(alx::file_info(path.toLocal8Bit().toStdString()), true);
        added.append(entry);
    }
    if (added.isEmpty()) return;

    const int first = m_entries.size();
    beginInsertRows(QModelIndex(), first, first + added.size() - 1);
    m_entries.append(added);
    endInsertRows();
}

void FileTableModel::clear() {
    beginResetModel();
    m_entries.clear();
    endResetModel();
}

QList<QPair<QString, QString>> FileTableModel::entries() const {
    QList<QPair<QString, QString>> result;
    for (const Entry& entry : m_entries) result.append({entry.path, entry.name});
    return result;
}

quint64 FileTableModel::total_size() const {
    quint64 result{0};
    for (const Entry& entry : m_entries) result += entry.size;
    return result;
}

bool FileTableModel::has_duplicate_name(int& _row) const {
    QSet<QString> names;
    for (int i = 0; i < m_entries.size(); i++) {
        if (names.contains(m_entries.at(i).name)) {
            _row = i;
            return true;
        }
        names.insert(m_entries.at(i).name);
    }
    return false;
}
