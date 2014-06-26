#include <QDebug>
#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QDateTime>

#include "o2gft.h"
#include "o2globals.h"
#include "o2settingsstore.h"


const char O2_OAUTH2_ID_TOKEN[] = "id_token";

const char O2_KEY_ID_TOKEN[] = "id_token.%1";


static const char *GftScope = "https://www.googleapis.com/auth/fusiontables";
static const char *GftEndpoint = "https://accounts.google.com/o/oauth2/auth";
static const char *GftTokenUrl = "https://accounts.google.com/o/oauth2/token";
static const char *GftRefreshUrl = "https://accounts.google.com/o/oauth2/token";


O2Gft::O2Gft(QObject *parent): O2(parent)
{
    setRequestUrl(GftEndpoint);
    setTokenUrl(GftTokenUrl);
    setRefreshTokenUrl(GftRefreshUrl);
    setScope(GftScope);
}

void O2Gft::onTokenReplyFinished()
{
    qDebug() << "O2Gft::onTokenReplyFinished";

    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (tokenReply->error() == QNetworkReply::NoError) {
        QByteArray replyData = tokenReply->readAll();
        QScriptEngine engine;
        QScriptValueIterator it(engine.evaluate("(" + QString(replyData) + ")"));
        QVariantMap tokens;

        while (it.hasNext()) {
            it.next();
            tokens.insert(it.name(), it.value().toVariant());
        }

        // Check for mandatory tokens
        if (tokens.contains(O2_OAUTH2_ACCESS_TOKEN)) {
            setToken(tokens.take(O2_OAUTH2_ACCESS_TOKEN).toString());

            //
            setIdToken(tokens.take(O2_OAUTH2_ID_TOKEN).toString());

            //
            bool ok = false;
            int expiresIn = tokens.take(O2_OAUTH2_EXPIRES_IN).toInt(&ok);
            if (ok) {
                qDebug() << "Token expires in" << expiresIn << "seconds";
                setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + expiresIn);
            }
            setRefreshToken(tokens.take(O2_OAUTH2_REFRESH_TOKEN).toString());
            // Set extra tokens if any
            if (!tokens.isEmpty()) {
                setExtraTokens(tokens);
            }
            timedReplies_.remove(tokenReply);
            emit linkedChanged();
            emit tokenChanged();
            emit linkingSucceeded();
        } else {
            qWarning() << "O2::onTokenReplyFinished: oauth_token missing from response" << replyData;
            emit linkedChanged();
            emit tokenChanged();
            emit linkingFailed();
        }
    }
    tokenReply->deleteLater();
}

QString O2Gft::idToken()
{
    QString key = QString(O2_KEY_ID_TOKEN).arg(clientId_);
    return store_->value(key);
}

void O2Gft::setIdToken(const QString &v)
{
    QString key = QString(O2_KEY_ID_TOKEN).arg(clientId_);
    store_->setValue(key, v);
}
