/* @file Реализация потока данных в памяти. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QBuffer>
#include <Common/QtHeadersEnd.h>

// Project
#include "MemoryDataStream.h"

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
