#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QSettings>
#include <QVariant>
#include <QDir>
#include <QStandardPaths>

class ConfigManager {
public:
    static ConfigManager& instance();

    void load();
    void save();

    QString lastFolder() const;
    void setLastFolder(const QString& folder);

    bool recursive() const;
    void setRecursive(bool recursive);

    double slideDuration() const;
    void setSlideDuration(double duration);

    double transitionTime() const;
    void setTransitionTime(double time);

    bool randomOrder() const;
    void setRandomOrder(bool random);

    bool continuousLoop() const;
    void setContinuousLoop(bool loop);
    
    double cacheMaxSizeMB() const;
    void setCacheMaxSizeMB(double size);

private:
    ConfigManager();
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    QString m_lastFolder;
    bool m_recursive;
    double m_slideDuration;
    double m_transitionTime;
    bool m_randomOrder;
    bool m_continuousLoop;
    double m_cacheMaxSizeMB;

    QString m_configDir;
    QString m_configFile;
};

#endif // CONFIGMANAGER_H
