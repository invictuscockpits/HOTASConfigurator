#include "configmanager.h"
#include <QFile>
#include <QJsonDocument>

ConfigManager::ConfigManager(QObject *parent) : QObject(parent) {}

bool ConfigManager::loadGripProfile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    m_gripConfig = doc.object();
    return true;
}

QJsonObject ConfigManager::gripConfig() const {
    return m_gripConfig;
}
