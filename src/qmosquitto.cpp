#include "qmosquitto.h"

#include <QDebug>

QMosquitto::QMosquitto(QObject *parent) :
    QObject(parent),
    mosquittopp(NULL, true),
    m_timer(0),
    m_hostname("localhost"),
    m_port(1883),
    m_keepalive(60),
    m_cleanSession(true),
    m_isConnected(false),
    m_clientId(),
    m_tlsEnabled(false),
    m_tls_insecure(false)
{

}

QMosquitto::~QMosquitto()
{
    m_topics.clear();
}

void QMosquitto::addTopicMatch(const QString topic, int topic_d)
{
    m_topics.insert(topic.trimmed(), topic_d);
}

int QMosquitto::removeTopicMatch(const QString topic)
{
   return m_topics.remove(topic);
}

bool QMosquitto::connectSocketNotifiers()
{
    int s=socket();
    if (s==-1) {
        qWarning() << "Failed to get mosquitto connection socket";
        return false;
    }

    m_notifier_read = new QSocketNotifier(s, QSocketNotifier::Read, this);
    QObject::connect(m_notifier_read, SIGNAL(activated(int)), this, SLOT(loopRead()));

    m_notifier_write = new QSocketNotifier(s, QSocketNotifier::Write, this);
    QObject::connect(m_notifier_write, SIGNAL(activated(int)), this, SLOT(loopWrite()));

    return true;
}

int QMosquitto::connectToHost()
{
    int r;

    if (m_isConnected)
        return MOSQ_ERR_CONN_PENDING;

    r=mosquittopp::reinitialise(m_clientId.isEmpty() ? NULL : m_clientId.toLocal8Bit().data(), m_cleanSession);
    if (r!=MOSQ_ERR_SUCCESS) {
        qWarning() << "Failed to set client id";
        return r;
    }

    if (m_username.isEmpty()==false) {
        r=mosquittopp::username_pw_set(m_username.toLocal8Bit().data(), m_password.isEmpty() ? NULL : m_password.toLocal8Bit().data());
        if (r!=MOSQ_ERR_SUCCESS) {
            qWarning() << "Failed to set username/password";
            return r;
        }
    }

    r=mosquittopp::connect_async(m_hostname.toLocal8Bit().data(), m_port, m_keepalive);
    if (r!=MOSQ_ERR_SUCCESS) {
        qWarning() << "Connection failure";
        return r;
    }

    if (m_tlsEnabled) {
        tls_set(m_tls_ca.toLatin1().data(), m_tls_capath.toLatin1().data(), m_tls_cert.toLatin1().data(), m_tls_key.toLatin1().data(), NULL);
        tls_insecure_set(m_tls_insecure);
    }

    if (connectSocketNotifiers()==false) {
        return MOSQ_ERR_NOT_FOUND;
    }

    emit connecting();

    return r;
}

int QMosquitto::reconnectToHost()
{
    int r;

    r=reconnect_async();

    shutdown();

    if (r!=MOSQ_ERR_SUCCESS) {
        qWarning() << "Connection failure";
        return r;
    }

    if (connectSocketNotifiers()==false) {
        return MOSQ_ERR_NOT_FOUND;
    }

    emit connecting();

    return r;
}

void QMosquitto::timerEvent(QTimerEvent *event)
{
    int r;

    Q_UNUSED(event)

    r=loop_misc();
    switch (r) {
    case MOSQ_ERR_SUCCESS:

        break;
    default:
        qWarning() << "Misc fail " << r;
        return;
        break;
    }

    if (want_write()==true) {
        // qDebug("NWWW");
        // loopWrite();
        m_notifier_write->setEnabled(true);
    }
}

void QMosquitto::loopRead()
{
    int r;

    // qDebug("LR");
    m_notifier_write->setEnabled(true);
    r=loop_read();
    if (r!=MOSQ_ERR_SUCCESS) {
        qWarning() << "Read fail " << r;
    }

}

void QMosquitto::loopWrite()
{
    int r;

    m_notifier_write->setEnabled(false);

    r=loop_misc();
    if (r!=MOSQ_ERR_SUCCESS) {
        qWarning() << "Misc fail " << r;
    }

    if (want_write()==false) {
        return;
    }

    // qDebug("LW");
    r=loop_write();
    if (r!=MOSQ_ERR_SUCCESS) {
        qWarning() << "Write fail " << r;
    }

}

void QMosquitto::shutdown()
{
    if (m_timer>0) {
        killTimer(m_timer);
        m_timer=0;
    }

    if (m_notifier_read) {
        delete m_notifier_read;
        m_notifier_read=NULL;
    }
    if (m_notifier_write) {
        delete m_notifier_write;
        m_notifier_write=NULL;
    }

}

int QMosquitto::disconnectFromHost()
{
    if (m_isConnected==false)
        return MOSQ_ERR_NO_CONN;
    int r=mosquittopp::disconnect();

    return r;
}

int QMosquitto::subscribe(const QString topic, int qos)
{
    m_notifier_write->setEnabled(true);
    int r=mosquittopp::subscribe(&m_smid, topic.toLocal8Bit().data(), qos);

    return r;
}

int QMosquitto::unsubscribe(const QString topic)
{
    m_notifier_write->setEnabled(true);
    int r=mosquittopp::unsubscribe(&m_mid, topic.toLocal8Bit().data());

    return r;
}

int QMosquitto::publish(const QString topic, QString data, int qos, bool retain)
{
    m_notifier_write->setEnabled(true);
    int r=mosquittopp::publish(&m_pmid, topic.toLocal8Bit().data(), data.size(), data.toLocal8Bit().data(), qos, retain);

    return r;
}

int QMosquitto::publish(const QString topic, QByteArray data, int qos, bool retain)
{
    m_notifier_write->setEnabled(true);
    int r=mosquittopp::publish(&m_pmid, topic.toLocal8Bit().data(), data.size(), data.data(), qos, retain);

    return r;
}

int QMosquitto::setWill(const QString topic, QString data, int qos, bool retain)
{
    m_notifier_write->setEnabled(true);
    int r=will_set(topic.toLocal8Bit().data(), data.size(), data.toLocal8Bit().data(), qos, retain);

    return r;
}

void QMosquitto::clearWill()
{
    m_notifier_write->setEnabled(true);
    will_clear();
}

void QMosquitto::on_connect(int rc)
{
    Q_UNUSED(rc)

    m_isConnected=true;
    emit connected();
    emit isConnectedeChanged(m_isConnected);
    m_timer=startTimer(1000);
}

void QMosquitto::on_disconnect(int rc)
{
    Q_UNUSED(rc)

    m_isConnected=false;

    shutdown();

    emit disconnected();
    emit isConnectedeChanged(m_isConnected);
}

void QMosquitto::on_message(const mosquitto_message *message)
{
    QString topic(message->topic);
    QString data=QString::fromLocal8Bit((char *)message->payload, message->payloadlen);

    emit msg(topic, data);

    if (m_topics.contains(topic))
        emit topicMatch(m_topics.value(topic));
}

void QMosquitto::on_error()
{
    qWarning("on_error");
    emit error();
}

void QMosquitto::on_log(int level, const char *str)
{
    // qDebug() << "mqtt: " << level << str;
}

void QMosquitto::setClientId(QString clientId)
{
    if (m_clientId == clientId)
        return;

    m_clientId = clientId;
    emit clientIdChanged(clientId);
}

void QMosquitto::setCleanSession(bool cleanSession)
{
    if (m_cleanSession == cleanSession)
        return;

    m_cleanSession = cleanSession;
    emit cleanSessionChanged(cleanSession);
}
