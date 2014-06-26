#ifndef O2GFT_H
#define O2GFT_H

#include "o2.h"

/// Google Fusion Tables' dialect of OAuth 2.0
class O2Gft: public O2 {
    Q_OBJECT

public:
    /// ID token.
    Q_PROPERTY(QString idToken READ idToken WRITE setIdToken NOTIFY idTokenChanged)
    QString idToken();
    void setIdToken(const QString &v);

public:
    explicit O2Gft(QObject *parent = 0);

protected slots:
    virtual void onTokenReplyFinished();

signals:
    void idTokenChanged();
};

#endif // O2GFT_H
