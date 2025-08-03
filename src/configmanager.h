#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H


#include <QObject>
#include <QJsonObject>

class ConfigManager : public QObject {
    Q_OBJECT
public:
    explicit ConfigManager(QObject *parent = nullptr);
    bool loadGripProfile(const QString &filePath);
    QJsonObject gripConfig() const;

private:
    QJsonObject m_gripConfig;
};
#endif // CONFIGMANAGER_H
