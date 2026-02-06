/* @file Item, проигрыватель flash */

#pragma once

#include <QtCore/QSharedPointer>
#include <QtDeclarative/QDeclarativeItem>
#include <QtWebKitWidgets/QGraphicsWebView>

//--------------------------------------------------------------------------
class FlashPlayerItem : public QDeclarativeItem {
    Q_OBJECT
    Q_PROPERTY(QString movie READ getMovie WRITE setMovie)

public:
    FlashPlayerItem(QDeclarativeItem *aParent = 0);
    ~FlashPlayerItem();

protected:
    virtual void geometryChanged(const QRectF &aNewGeometry, const QRectF &aOldGeometry);

private:
    QString getMovie() const;
    void setMovie(const QString &aMovie);

private slots:
    void populateJavaScriptWindowObject();

public slots:
    void onClicked(const QVariant &aParameter) { emit clicked(aParameter); }

signals:
    void clicked(const QVariant &aParameters);

private:
    QGraphicsWebView *mWebView;
    QString mMovie;
    QString mHtmlWrapper;
    QRectF mGeometry;
};

//--------------------------------------------------------------------------
