/* @file Реализация файлового потока данных. */

#include "FileDataStream.h"

#include <QtCore/QFile>

FileDataStream::FileDataStream(const QString &aPath) : DataStream(nullptr) {
    m_stream = QSharedPointer<QIODevice>(new QFile(aPath));
    m_stream->open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Unbuffered);
}

//------------------------------------------------------------------------
bool FileDataStream::clear() {
    auto *file = dynamic_cast<QFile *>(m_stream.data());

    return file->resize(0) && file->seek(0);
}

//------------------------------------------------------------------------
bool FileDataStream::write(const QByteArray &aData) {
    auto *file = dynamic_cast<QFile *>(m_stream.data());

    return (file->write(aData) == aData.size() && file->flush());
}

//------------------------------------------------------------------------
qint64 FileDataStream::size() const {
    auto *file = dynamic_cast<QFile *>(m_stream.data());

    return file->size();
}

//------------------------------------------------------------------------
