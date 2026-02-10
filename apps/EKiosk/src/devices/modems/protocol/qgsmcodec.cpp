#include "qgsmcodec.h"

const unsigned short GUC = 0x10; // GSM Undefined character

// Table from GSM 07.05, Annex A, combined with the extension table
// from GSM 03.38, Section 6.2.1.1.
static const unsigned short Latin1GsmTable[256] = {
    //     0      1     2     3     4     5     6     7
    GUC,    GUC,  GUC,  GUC,    GUC,    GUC,    GUC,    GUC, // 0x07
    GUC,    GUC,  0x0a, GUC,    GUC,    0x0d,   GUC,    GUC, // 0x0f
    GUC,    GUC,  GUC,  GUC,    GUC,    GUC,    GUC,    GUC,  GUC,  GUC,  GUC,    GUC,    GUC,
    GUC,    GUC,  GUC,  0x20,   0x21,   0x22,   0x23,   0x02, 0x25, 0x26, 0x27,   0x28,   0x29,
    0x2a,   0x2b, 0x2c, 0x2d,   0x2e,   0x2f,   0x30,   0x31, 0x32, 0x33, 0x34,   0x35,   0x36,
    0x37,   0x38, 0x39, 0x3a,   0x3b,   0x3c,   0x3d,   0x3e, 0x3f, 0x00, 0x41,   0x42,   0x43,
    0x44,   0x45, 0x46, 0x47,   0x48,   0x49,   0x4a,   0x4b, 0x4c, 0x4d, 0x4e,   0x4f,   0x50,
    0x51,   0x52, 0x53, 0x54,   0x55,   0x56,   0x57,   0x58, 0x59, 0x5a, 0x1b3c, 0x1b2f, 0x1b3e,
    0x1b14, 0x11, GUC,  0x61,   0x62,   0x63,   0x64,   0x65, 0x66, 0x67, 0x68,   0x69,   0x6a,
    0x6b,   0x6c, 0x6d, 0x6e,   0x6f,   0x70,   0x71,   0x72, 0x73, 0x74, 0x75,   0x76,   0x77,
    0x78,   0x79, 0x7a, 0x1b28, 0x1b40, 0x1b29, 0x1b3d, GUC, // 0x7f

    GUC,    GUC,  GUC,  GUC,    GUC,    GUC,    GUC,    GUC,  GUC,  GUC,  GUC,    GUC,    GUC,
    GUC,    GUC,  GUC, // 0x8f
    GUC,    GUC,  GUC,  GUC,    GUC,    GUC,    GUC,    GUC,  GUC,  GUC,  GUC,    GUC,    GUC,
    GUC,    GUC,  GUC, // 0x9f
    GUC,    0x40, GUC,  0x01,   0x24,   0x03,   GUC,    0x5f, GUC,  GUC,  GUC,    GUC,    GUC,
    GUC,    GUC,  GUC,  GUC,    GUC,    GUC,    GUC,    GUC,  GUC,  GUC,  GUC,    GUC,    GUC,
    GUC,    GUC,  GUC,  GUC,    GUC,    0x60,   0x41,   0x41, 0x41, 0x41, 0x5b,   0x0e,   0x1c,
    0x09,   0x45, 0x1f, 0x45,   0x45,   0x49,   0x49,   0x49, 0x49, // 0xcf
    GUC,    0x5d, 0x4f, 0x4f,   0x4f,   0x4f,   0x5c,   GUC,  0x0b, 0x55, 0x55,   0x55,   0x5e,
    0x59,   GUC,  0x1e, 0x7f,   0x61,   0x61,   0x61,   0x7b, 0x0f, 0x1d, 0x09,   0x04,   0x05,
    0x65,   0x65, 0x07, 0x69,   0x69,   0x69,   GUC,    0x7d, 0x08, 0x6f, 0x6f,   0x6f,   0x7c,
    GUC,    0x0c, 0x06, 0x75,   0x75,   0x7e,   0x79,   GUC,  0x79};

// Same as above, but with no loss of information due to two
// Latin1 characters mapping to the same thing.
static const unsigned short Latin1GsmNoLossTable[256] = {
    //     0      1     2     3     4     5     6     7
    GUC,    GUC,  GUC,  GUC,    GUC,    GUC,    GUC,    GUC, // 0x07
    GUC,    GUC,  0x0a, GUC,    GUC,    0x0d,   GUC,    GUC, // 0x0f
    GUC,    GUC,  GUC,  GUC,    GUC,    GUC,    GUC,    GUC,  GUC,  GUC,  GUC,    GUC,    GUC,
    GUC,    GUC,  GUC,  0x20,   0x21,   0x22,   0x23,   0x02, 0x25, 0x26, 0x27,   0x28,   0x29,
    0x2a,   0x2b, 0x2c, 0x2d,   0x2e,   0x2f,   0x30,   0x31, 0x32, 0x33, 0x34,   0x35,   0x36,
    0x37,   0x38, 0x39, 0x3a,   0x3b,   0x3c,   0x3d,   0x3e, 0x3f, 0x00, 0x41,   0x42,   0x43,
    0x44,   0x45, 0x46, 0x47,   0x48,   0x49,   0x4a,   0x4b, 0x4c, 0x4d, 0x4e,   0x4f,   0x50,
    0x51,   0x52, 0x53, 0x54,   0x55,   0x56,   0x57,   0x58, 0x59, 0x5a, 0x1b3c, 0x1b2f, 0x1b3e,
    0x1b14, 0x11, GUC,  0x61,   0x62,   0x63,   0x64,   0x65, 0x66, 0x67, 0x68,   0x69,   0x6a,
    0x6b,   0x6c, 0x6d, 0x6e,   0x6f,   0x70,   0x71,   0x72, 0x73, 0x74, 0x75,   0x76,   0x77,
    0x78,   0x79, 0x7a, 0x1b28, 0x1b40, 0x1b29, 0x1b3d, GUC, // 0x7f

    GUC,    GUC,  GUC,  GUC,    GUC,    GUC,    GUC,    GUC,  GUC,  GUC,  GUC,    GUC,    GUC,
    GUC,    GUC,  GUC, // 0x8f
    GUC,    GUC,  GUC,  GUC,    GUC,    GUC,    GUC,    GUC,  GUC,  GUC,  GUC,    GUC,    GUC,
    GUC,    GUC,  GUC, // 0x9f
    GUC,    0x40, GUC,  0x01,   0x24,   0x03,   GUC,    0x5f, GUC,  GUC,  GUC,    GUC,    GUC,
    GUC,    GUC,  GUC,  GUC,    GUC,    GUC,    GUC,    GUC,  GUC,  GUC,  GUC,    GUC,    GUC,
    GUC,    GUC,  GUC,  GUC,    GUC,    0x60,   GUC,    GUC,  GUC,  GUC,  0x5b,   0x0e,   0x1c,
    0x09,   GUC,  0x1f, GUC,    GUC,    GUC,    GUC,    GUC,  GUC, // 0xcf
    GUC,    0x5d, GUC,  GUC,    GUC,    GUC,    0x5c,   GUC,  0x0b, GUC,  GUC,    GUC,    0x5e,
    GUC,    GUC,  0x1e, 0x7f,   GUC,    GUC,    GUC,    0x7b, 0x0f, 0x1d, GUC,    0x04,   0x05,
    GUC,    GUC,  0x07, GUC,    GUC,    GUC,    GUC,    0x7d, 0x08, GUC,  GUC,    GUC,    0x7c,
    GUC,    0x0c, 0x06, GUC,    GUC,    0x7e,   GUC,    GUC,  GUC};

// Conversion table for Greek Unicode code points 0x0390 - 0x03AF.
static const unsigned short GreekGsmTable[32] = {
    GUC,  GUC, GUC, 0x13, 0x10, GUC, GUC,  GUC, 0x19, GUC,  GUC, 0x14, GUC, GUC, 0x1a, GUC,
    0x16, GUC, GUC, 0x18, GUC,  GUC, 0x12, GUC, 0x17, 0x15, GUC, GUC,  GUC, GUC, GUC,  GUC};

const unsigned short UUC = 0xFFFE; // Unicode Undefined character

// Reversed version of latin1GSMTable.
static const unsigned short GsmLatin1Table[256] = {
    0x40,   0xa3,   0x24,   0xa5, 0xe8,   0xe9, 0xf9,   0xec,   0xf2,   0xc7,   0x0a,   0xd8,
    0xf8,   0x0d,   0xc5,   0xe5, 0x0394, 0x5f, 0x03a6, 0x0393, 0x039B, 0x03A9, 0x03A0, 0x03A8,
    0x03A3, 0x0398, 0x039E, 0x20, 0xc6,   0xe6, 0xdf,   0xc9,   0x20,   0x21,   0x22,   0x23,
    0xa4,   0x25,   0x26,   0x27, 0x28,   0x29, 0x2a,   0x2b,   0x2c,   0x2d,   0x2e,   0x2f,
    0x30,   0x31,   0x32,   0x33, 0x34,   0x35, 0x36,   0x37,   0x38,   0x39,   0x3a,   0x3b,
    0x3c,   0x3d,   0x3e,   0x3f, 0xa1,   0x41, 0x42,   0x43,   0x44,   0x45,   0x46,   0x47,
    0x48,   0x49,   0x4a,   0x4b, 0x4c,   0x4d, 0x4e,   0x4f,   0x50,   0x51,   0x52,   0x53,
    0x54,   0x55,   0x56,   0x57, 0x58,   0x59, 0x5a,   0xc4,   0xd6,   0xd1,   0xdc,   0xa7,
    0xbf,   0x61,   0x62,   0x63, 0x64,   0x65, 0x66,   0x67,   0x68,   0x69,   0x6a,   0x6b,
    0x6c,   0x6d,   0x6e,   0x6f, 0x70,   0x71, 0x72,   0x73,   0x74,   0x75,   0x76,   0x77,
    0x78,   0x79,   0x7a,   0xe4, 0xf6,   0xf1, 0xfc,   0xe0,   UUC,    UUC,    UUC,    UUC,
    UUC,    UUC,    UUC,    UUC,  UUC,    UUC,  UUC,    UUC,    UUC,    UUC,    UUC,    UUC,
    UUC,    UUC,    UUC,    UUC,  UUC,    UUC,  UUC,    UUC,    UUC,    UUC,    UUC,    UUC,
    UUC,    UUC,    UUC,    UUC,  UUC,    UUC,  UUC,    UUC,    UUC,    UUC,    UUC,    UUC,
    UUC,    UUC,    UUC,    UUC,  UUC,    UUC,  UUC,    UUC,    UUC,    UUC,    UUC,    UUC,
    UUC,    UUC,    UUC,    UUC,  UUC,    UUC,  UUC,    UUC,    UUC,    UUC,    UUC,    UUC,
    UUC,    UUC,    UUC,    UUC,  UUC,    UUC,  UUC,    UUC,    UUC,    UUC,    UUC,    UUC,
    UUC,    UUC,    UUC,    UUC,  UUC,    UUC,  UUC,    UUC,    UUC,    UUC,    UUC,    UUC,
    UUC,    UUC,    UUC,    UUC,  UUC,    UUC,  UUC,    UUC,    UUC,    UUC,    UUC,    UUC,
    UUC,    UUC,    UUC,    UUC,  UUC,    UUC,  UUC,    UUC,    UUC,    UUC,    UUC,    UUC,
    UUC,    UUC,    UUC,    UUC,  UUC,    UUC,  UUC,    UUC,    UUC,    UUC,    UUC,    UUC,
    UUC,    UUC,    UUC,    UUC};
static const unsigned short ExtensionLatin1Table[256] = {
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, 0x5e, UUC,    UUC, UUC, UUC,  UUC,  UUC, 0x20, UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, 0x7b, 0x7d, UUC, UUC,  UUC,  UUC,  UUC,  0x5c,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  0x5b, 0x7e, 0x5d, UUC,
    0x7c, UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  0x20ac, UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC,
    UUC,  UUC, UUC, UUC, UUC,  UUC,    UUC, UUC, UUC,  UUC,  UUC, UUC,  UUC,  UUC,  UUC,  UUC};

/*!
        \class QGsm_Codec
        \inpublicgroup QtBaseModule

        \brief The QGsm_Codec class represents the text codec for the GSM 7-bit
   encoding of Latin-1
        \ingroup telephony::serial

        The GSM specifications for SMS use a compact 7-bit encoding to represent
        Latin-1 characters, compared to the more usual 8-bit ISO-8859-1 encoding
        used on many computer systems.

        The QGsm_Codec class enables conversion back and forth between the GSM
        encoding and the normal Unicode encoding used by Qtopia.

        Application programs will rarely need to use this class, because
        the QSMSMessage class automatically converts between 7-bit and Unicode
        encodings as necessary.

        If an application program does need to use this class, it should call
        QAtUtils::codec() to obtain an instance of the codec.  Constructing
        QGsm_Codec objects directly is not recommended, due to how QTextCodec
        registers and deregisters codec implementations.

        The following example converts the \c input string into the compact
        7-bit encoding within \c output.

        \code
        QString input = "...";
        QByteArray output = QAtUtils::codec("gsm")->fromUnicode(input);
        \endcode

        This codec implementation conforms to 3GPP TS 03.38 and 3GPP TS 07.05,
        including the extension tables from 3GPP TS 03.38.

        \sa QSMSMessage, QAtUtils::codec()
*/

/*!
        Construct a new GSM text codec.  If \a noLoss is true, then the
        codec should not encode characters that may result in an
        ambiguous decoding.

        This constructor should not be used directly.  Instead, call
   QAtUtils::codec() to obtain an instance of this codec implementation.  This
   is due to how QTextCodec registers and deregisters codec implementations. The
   codec names to use with QAtUtils::codec() are \c gsm and \c gsm-noloss, for
   the regular and no-loss versions of the codec.

        \sa QAtUtils::codec()
*/
QGsm_Codec::QGsm_Codec(bool noLoss) : noLoss(noLoss) {}

/*!
        Destruct a GSM text codec.  This should not be used directly by
        application programs.
*/
QGsm_Codec::~QGsm_Codec() = default;

/*!
        Returns the name of this codec.
*/
QByteArray QGsm_Codec::name() const {
    if (noLoss) {
        return {"gsm-noloss"};
    }
    return {"gsm"};
}

/*!
        Returns the MIB value associated with this codec.
*/
int QGsm_Codec::mibEnum() const {
    if (noLoss) {
        return 61237;
    }
    return 61238;
}

/*!
        Convert a single Unicode character \a c into GSM 7-bit.
        Returns 0x10 if the character cannot be mapped.  Use of this
        function is discouraged.

        Note: this will not work for two-byte GSM encodings.  Use
        twoByteFrom_Unicode() instead.

        \sa singleToUnicode(), twoByteFrom_Unicode()
*/
char QGsm_Codec::singleFrom_Unicode(QChar c) {
    unsigned int ch = c.unicode();
    if (ch < 256) {
        return (char)(Latin1GsmTable[ch]);
    }
    if (ch >= 0x0390 && ch <= 0x03AF) {
        return (char)(GreekGsmTable[ch - 0x0390]);
}
    return (char)GUC;
}

/*!
        Convert a single GSM 7-bit character \a ch into Unicode.  Use of this
        function is discouraged.

        Note: this will not work for two-byte GSM encodings.  Use
        twoByteToUnicode() instead.

        \sa singleFrom_Unicode(), twoByteToUnicode()
*/
QChar QGsm_Codec::singleToUnicode(char ch) {
    return QChar((unsigned int)(GsmLatin1Table[((int)ch) & 0xFF]));
}

/*!
        Convert a Unicode character \a ch into its GSM-encoded counterpart.
        The return value will be greater than 256 if the Unicode character
        should be encoded as two bytes.

        \sa twoByteToUnicode()
*/
unsigned short QGsm_Codec::twoByteFrom_Unicode(QChar ch) {
    unsigned short c = ch.unicode();
    if (c == 0x20AC) { // Euro
        return 0x1b65;
    }
    if (c < 256) {
        return Latin1GsmTable[c];
}
    if (c >= 0x0390 && c <= 0x03AF) {
        return (char)(GreekGsmTable[c - 0x0390]);
}
    return GUC;
}

/*!
        Convert a single GSM-encoded character into its Unicode counterpart.
        If \a ch is greater than 256, then it represents a two-byte sequence.

        \sa twoByteFrom_Unicode()
*/
QChar QGsm_Codec::twoByteToUnicode(unsigned short ch) {
    if (ch < 256) {
        return QChar(GsmLatin1Table[ch]);
    }
    if ((ch & 0xFF00) != 0x1B00) {
        return QChar(0);
}
    unsigned short mapping = ExtensionLatin1Table[ch & 0xFF];
    if (mapping != UUC) {
        return QChar(mapping);
}
    return QChar(GsmLatin1Table[ch & 0xFF]);
}

/*!
        Convert the \a length bytes at \a in into Unicode.  The \c invalidChars
        field of \a state will be incremented if there are invalid characters
        within \a in.

        \sa convertFrom_Unicode()
*/
QString QGsm_Codec::convertToUnicode(const char *in, int length, ConverterState *state) const {
    QString str;
    unsigned short ch = 0;
    while (length > 0) {
        if (*in == 0x1B) {
            // Two-byte GSM sequence.
            ++in;
            --length;
            if (length <= 0) {
                if (state) {
                    (state->invalidChars)++;
                }
                break;
            }
            ch = ExtensionLatin1Table[((int)(*in)) & 0xFF];
            if (ch != UUC) {
                str += QChar((unsigned int)ch);
            } else {
                str += QChar(GsmLatin1Table[((int)(*in)) & 0xFF]);
                if (state) {
                    (state->invalidChars)++;
                }
            }
        } else {
            ch = GsmLatin1Table[((int)(*in)) & 0xFF];
            if (ch != UUC) {
                str += QChar((unsigned int)ch);
            } else if (state) {
                (state->invalidChars)++;
            }
        }
        ++in;
        --length;
    }
    return str;
}

/*!
        Convert the \a length characters at \a in into 7-bit GSM.  The \c
   invalidChars field of \a state will be incremented if there are invalid
   characters within \a in.

        \sa convertToUnicode()
*/
QByteArray
QGsm_Codec::convertFromUnicode(const QChar *in, int length, ConverterState *state) const {
    QByteArray result;
    unsigned int unicode = 0;
    if (noLoss) {
        while (length > 0) {
            unicode = (*in).unicode();
            if (unicode == 0x20AC) { // Euro
                result += (char)0x1B;
                result += (char)0x65;
            } else if (unicode < 256) {
                unsigned short code = Latin1GsmNoLossTable[unicode];
                if (code < 256) {
                    if (((char)code == GUC) && state) {
                        (state->invalidChars)++;
                    }
                    result += (char)code;
                } else {
                    result += (char)(code >> 8);
                    result += (char)code;
                }
            } else if (unicode >= 0x0390 && unicode <= 0x03AF) {
                char c = (char)(GreekGsmTable[unicode - 0x0390]);
                result += c;
                if (c == (char)GUC && unicode != 0x0394 && state) {
                    (state->invalidChars)++;
                }
            } else {
                result += (char)GUC;
                if (state) {
                    (state->invalidChars)++;
                }
            }
            ++in;
            --length;
        }
    } else {
        while (length > 0) {
            unicode = (*in).unicode();
            if (unicode == 0x20AC) { // Euro
                result += (char)0x1B;
                result += (char)0x65;
            } else if (unicode < 256) {
                unsigned short code = Latin1GsmTable[unicode];
                if (code < 256) {
                    result += (char)code;
                } else {
                    result += (char)(code >> 8);
                    result += (char)code;
                }
            } else if (unicode >= 0x0390 && unicode <= 0x03AF) {
                char c = (char)(GreekGsmTable[unicode - 0x0390]);
                result += c;
                if (c == (char)GUC && unicode != 0x0394 && state) {
                    (state->invalidChars)++;
                }
            } else {
                result += (char)GUC;
                if (state) {
                    (state->invalidChars)++;
                }
            }
            ++in;
            --length;
        }
    }
    return result;
}
