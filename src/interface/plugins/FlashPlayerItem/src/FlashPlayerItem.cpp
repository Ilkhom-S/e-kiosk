/* @file Item, проигрыватель flash */

#include "FlashPlayerItem.h"

#include <QtWebKitWidgets/QWebFrame>

FlashPlayerItem::FlashPlayerItem(QDeclarativeItem *aParent) : QDeclarativeItem(aParent) {
    setFlag(QGraphicsItem::Item_HasNoContents, false);

    QWebSettings *defaultSettings = QWebSettings::globalSettings();
    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, true);
    defaultSettings->setAttribute(QWebSettings::PluginsEnabled, true);

    m_WebView = new QGraphicsWebView(this);

    connect(m_WebView->page()->mainFrame(),
            SIGNAL(javaScriptWindowObjectCleared()),
            this,
            SLOT(populateJavaScriptWindowObject()));

    m_HtmlWrapper = "<body marginwidth='0' marginheight='0'> \
								 <embed width='%1' height='%2' name='plugin' src='%3' wmode='transparent' type='application/x-shockwave-flash'> \
								 <script>function click(aParameters) { handler.onClicked(aParameters); }</script></body>";
}

//--------------------------------------------------------------------------
FlashPlayerItem::~FlashPlayerItem() {}

//--------------------------------------------------------------------------
QString FlashPlayerItem::getMovie() const {
    return m_Movie;
}

//--------------------------------------------------------------------------
void FlashPlayerItem::setMovie(const QString &aMovie) {
    m_Movie = aMovie;

    QString html = QString(m_HtmlWrapper)
                       .arg(QString::number(m_Geometry.width()))
                       .arg(QString::number(m_Geometry.height()))
                       .arg(m_Movie);

    m_WebView->setHtml(html);
}

//--------------------------------------------------------------------------
void FlashPlayerItem::geometryChanged(const QRectF &aNewGeometry, const QRectF &aOldGeometry) {
    if (aNewGeometry != aOldGeometry) {
        m_WebView->setGeometry(QRectF(0., 0., aNewGeometry.width(), aNewGeometry.height()));
        m_Geometry = aNewGeometry;

        if (!m_Movie.isEmpty()) {
            QString html = QString(m_HtmlWrapper)
                               .arg(QString::number(m_Geometry.width()))
                               .arg(QString::number(m_Geometry.height()))
                               .arg(m_Movie);

            m_WebView->page()->mainFrame()->setHtml(html);
        }
    }

    QDeclarativeItem::geometryChanged(aNewGeometry, aOldGeometry);
}

//--------------------------------------------------------------------------
void FlashPlayerItem::populateJavaScriptWindowObject() {
    m_WebView->page()->mainFrame()->addToJavaScriptWindowObject("handler", this);
}

//--------------------------------------------------------------------------
