/* @file Реализация потока данных в памяти. */

#include <QtCore/QBuffer>

#include <NetworkTaskManager/MemoryDataStream.h>

MemoryDataStream::MemoryDataStream() : DataStream(new QBuffer()) {}

//------------------------------------------------------------------------
bool MemoryDataStream::clear() {
    auto *buf = dynamic_cast<QBuffer *>(m_stream.data());

    buf->close();
    buf->setData(QByteArray());
    buf->open(QIODevice::ReadWrite);

    return true;
}

//------------------------------------------------------------------------
