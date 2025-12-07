#include "ThumbnailLoader.h"
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QImageReader>
#include <QDateTime>
#include <QDebug>
#include "ConfigManager.h"

ThumbnailLoader::ThumbnailLoader(QObject* parent) 
    : QObject(parent), m_abort(false), m_currentPage(0), m_thumbsPerPage(20)
{
    m_cacheDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/Endless_Slides/thumbnails";
    m_metadataFile = m_cacheDir + "/cache_metadata.json";
    
    QDir dir;
    if (!dir.exists(m_cacheDir)) {
        dir.mkpath(m_cacheDir);
    }
    
    loadCacheMetadata();
}

ThumbnailLoader::~ThumbnailLoader() {
    stop();
    saveCacheMetadata();
}

void ThumbnailLoader::stop() {
    QMutexLocker locker(&m_mutex);
    m_abort = true;
    m_condition.wakeOne();
}

void ThumbnailLoader::setPaths(const QStringList& paths) {
    QMutexLocker locker(&m_mutex);
    m_paths = paths;
    m_condition.wakeOne();
}

void ThumbnailLoader::updatePriority(int page, int thumbsPerPage) {
    QMutexLocker locker(&m_mutex);
    m_currentPage = page;
    m_thumbsPerPage = thumbsPerPage;
    m_condition.wakeOne();
}

void ThumbnailLoader::process() {
    // This runs in the worker thread
    forever {
        QStringList pathsCopy;
        int currentPage;
        int thumbsPerPage;
        
        {
            QMutexLocker locker(&m_mutex);
            if (m_abort) break;
            
            // Wait until we have paths
            if (m_paths.isEmpty()) {
                m_condition.wait(&m_mutex);
                if (m_abort) break;
            }
            
            pathsCopy = m_paths;
            currentPage = m_currentPage;
            thumbsPerPage = m_thumbsPerPage;
        }
        
        if (pathsCopy.isEmpty()) continue;

        // Prioritize current page
        int startIdx = currentPage * thumbsPerPage;
        int endIdx = qMin(startIdx + thumbsPerPage, pathsCopy.size());
        
        QList<int> priorityIndices;
        // High priority: Current page
        for (int i = startIdx; i < endIdx; ++i) priorityIndices << i;
        
        // Low priority: Rest of the images
        for (int i = 0; i < pathsCopy.size(); ++i) {
            if (i < startIdx || i >= endIdx) priorityIndices << i;
        }

        bool workDone = false;
        
        for (int idx : priorityIndices) {
            if (m_abort) break;
            
            // Check if priority changed mid-loop
            {
                 QMutexLocker locker(&m_mutex);
                 if (m_currentPage != currentPage || m_paths.size() != pathsCopy.size()) {
                     break; // Restart loop with new priority
                 }
            }

            QString path = pathsCopy[idx];
            QString cachePath = getCacheFilePath(path);
            QFileInfo fi(path);
            
            if (!fi.exists()) continue; // File deleted?

            qint64 mtime = fi.lastModified().toMSecsSinceEpoch();
            bool cachedParamsMatch = false;

            if (m_metadata.contains(path)) {
                if (m_metadata[path].lastModified == mtime && QFile::exists(cachePath)) {
                    cachedParamsMatch = true;
                }
            }

            if (cachedParamsMatch) {
                // Emit cached
                 // We don't necessarily need to re-emit if the UI already has it, 
                 // but for simplicity we rely on the UI to ignore duplicates or we could track emitted here.
                 // For this robust impl, we load the cached file and emit.
                 // Optimization: The UI passes a list of "needed" indices? 
                 // For now, let's just create QImage and emit. 
                 
                 QImage img(cachePath);
                 if (!img.isNull()) {
                     emit thumbnailReady(idx, path, img);
                     m_metadata[path].lastAccess = QDateTime::currentMSecsSinceEpoch();
                 }
            } else {
                // Generate
                QImageReader reader(path);
                
                // Scale efficiently
                QSize originalSize = reader.size();
                // Target size 150x150
                if (originalSize.isValid()) {
                    int dim = 300; // Match max slider zoom
                   if (originalSize.width() > dim || originalSize.height() > dim) {
                       reader.setScaledSize(originalSize.scaled(dim, dim, Qt::KeepAspectRatio));
                   }
                
                   QImage img = reader.read();
                   if (!img.isNull()) {
                       img.save(cachePath, "JPG", 85);
                       
                       CacheMetadata meta;
                       meta.lastModified = mtime;
                       meta.lastAccess = QDateTime::currentMSecsSinceEpoch();
                       meta.cacheFile = cachePath;
                       meta.sizeBytes = QFileInfo(cachePath).size();
                       
                       m_metadata[path] = meta;
                       
                       emit thumbnailReady(idx, path, img);
                       workDone = true;
                   }
                }
            }
            
            // Small sleep to not hog CPU?
            // Actually QThread::yieldCurrentThread() or generic sleep
            // But we want it fast on Pi.
        }
        
        // If we finished the whole list or loop broke, wait again
        if (!workDone) {
             QMutexLocker locker(&m_mutex);
             if (!m_abort) m_condition.wait(&m_mutex, 1000); // verify cache cleanup periodically
             
             // Run cleanup occasionally
             QMetaObject::invokeMethod(this, "cleanCache", Qt::QueuedConnection);
        }
    }
}

QString ThumbnailLoader::getCacheFilePath(const QString& path) {
    QByteArray hash = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha256);
    return m_cacheDir + "/" + hash.toHex() + ".thumb";
}

void ThumbnailLoader::loadCacheMetadata() {
     QFile f(m_metadataFile);
     if (f.open(QIODevice::ReadOnly)) {
         QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
         QJsonObject root = doc.object();
         for (auto it = root.begin(); it != root.end(); ++it) {
             QJsonObject obj = it.value().toObject();
             CacheMetadata meta;
             meta.lastModified = (qint64)obj["last_modified"].toDouble();
             meta.sizeBytes = (qint64)obj["size_bytes"].toDouble();
             meta.lastAccess = (qint64)obj["last_access"].toDouble();
             meta.cacheFile = obj["cache_file"].toString();
             m_metadata[it.key()] = meta;
         }
     }
}

void ThumbnailLoader::saveCacheMetadata() {
    QJsonObject root;
    for (auto it = m_metadata.begin(); it != m_metadata.end(); ++it) {
        QJsonObject obj;
        obj["last_modified"] = (double)it.value().lastModified;
        obj["size_bytes"] = (double)it.value().sizeBytes;
        obj["last_access"] = (double)it.value().lastAccess;
        obj["cache_file"] = it.value().cacheFile;
        root[it.key()] = obj;
    }
    
    QFile f(m_metadataFile);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(root).toJson());
    }
}

void ThumbnailLoader::cleanCache() {
    double maxMB = ConfigManager::instance().cacheMaxSizeMB();
    qint64 maxBytes = (qint64)(maxMB * 1024 * 1024);
    
    qint64 currentBytes = 0;
    QList<QPair<qint64, QString>> items; // lastAccess, key
    
    for (auto it = m_metadata.begin(); it != m_metadata.end(); ++it) {
        currentBytes += it.value().sizeBytes;
        items.append(qMakePair(it.value().lastAccess, it.key()));
    }
    
    if (currentBytes <= maxBytes) return;
    
    // Sort by LRU (oldest access first)
    std::sort(items.begin(), items.end(), [](const auto& a, const auto& b){
        return a.first < b.first;
    });
    
    for (const auto& item : items) {
        if (currentBytes <= maxBytes * 0.9) break; // Clean down to 90%
        
        QString key = item.second;
        QString file = m_metadata[key].cacheFile;
        
        if (QFile::remove(file)) {
            currentBytes -= m_metadata[key].sizeBytes;
            m_metadata.remove(key);
        }
    }
}

void ThumbnailLoader::clearCache() {
    QMutexLocker locker(&m_mutex);
    
    // Robust clear: Delete all files in directory
    QDir dir(m_cacheDir);
    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QString &file : files) {
        dir.remove(file);
    }
    
    m_metadata.clear();
    saveCacheMetadata();
    emit cacheCleared();
}
