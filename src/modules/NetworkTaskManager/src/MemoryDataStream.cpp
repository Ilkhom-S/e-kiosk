/* @file Реализация потока данных в памяти. */

#include "MemoryDataStream.h"

#include <QtCore/QBuffer>

MemoryDataStream::MemoryDataStream() : DataStream(new QBuffer()) {}

//------------------------------------------------------------------------
bool MemoryDataStream::clear() {
    QBuffer *buf = dynamic_cast<QBuffer *>(m_stream.data());

    buf->close();
    buf->setData(QByteArray());
    buf->open(QIODevice::ReadWrite);

    return true;
}

//------------------------------------------------------------------------
