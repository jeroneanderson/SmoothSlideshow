#include "ConfigManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDebug>

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager()
    : m_recursive(true), m_slideDuration(3.0), m_transitionTime(0.5),
      m_randomOrder(false), m_continuousLoop(true), m_cacheMaxSizeMB(512.0)
{
    m_configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/Endless_Slides";
    m_configFile = m_configDir + "/config.json";
    
    QDir dir;
    if (!dir.exists(m_configDir)) {
        dir.mkpath(m_configDir);
    }
}

void ConfigManager::load() {
    QFile file(m_configFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    m_lastFolder = obj["last_folder"].toString();
    if (obj.contains("recursive")) m_recursive = obj["recursive"].toBool();
    
    if (obj.contains("slide_duration")) {
        double val = obj["slide_duration"].toDouble();
        if (val > 0.1) m_slideDuration = val;
    }
    if (obj.contains("transition_time")) {
        double val = obj["transition_time"].toDouble();
        if (val >= 0.0) m_transitionTime = val;
    }
    if (obj.contains("random_order")) m_randomOrder = obj["random_order"].toBool();
    if (obj.contains("continuous_loop")) m_continuousLoop = obj["continuous_loop"].toBool();
    
    if (obj.contains("cache_max_size_mb")) {
        double val = obj["cache_max_size_mb"].toDouble();
        if (val >= 1.0) m_cacheMaxSizeMB = val;
    }
}

void ConfigManager::save() {
    QJsonObject obj;
    obj["last_folder"] = m_lastFolder;
    obj["recursive"] = m_recursive;
    obj["slide_duration"] = m_slideDuration;
    obj["transition_time"] = m_transitionTime;
    obj["random_order"] = m_randomOrder;
    obj["continuous_loop"] = m_continuousLoop;
    obj["cache_max_size_mb"] = m_cacheMaxSizeMB;

    QJsonDocument doc(obj);
    QFile file(m_configFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
}

// Getters and Setters
QString ConfigManager::lastFolder() const { return m_lastFolder; }
void ConfigManager::setLastFolder(const QString& folder) { m_lastFolder = folder; }

bool ConfigManager::recursive() const { return m_recursive; }
void ConfigManager::setRecursive(bool recursive) { m_recursive = recursive; }

double ConfigManager::slideDuration() const { return m_slideDuration; }
void ConfigManager::setSlideDuration(double duration) { m_slideDuration = duration; }

double ConfigManager::transitionTime() const { return m_transitionTime; }
void ConfigManager::setTransitionTime(double time) { m_transitionTime = time; }

bool ConfigManager::randomOrder() const { return m_randomOrder; }
void ConfigManager::setRandomOrder(bool random) { m_randomOrder = random; }

bool ConfigManager::continuousLoop() const { return m_continuousLoop; }
void ConfigManager::setContinuousLoop(bool loop) { m_continuousLoop = loop; }

double ConfigManager::cacheMaxSizeMB() const { return m_cacheMaxSizeMB; }
void ConfigManager::setCacheMaxSizeMB(double size) { m_cacheMaxSizeMB = size; }
