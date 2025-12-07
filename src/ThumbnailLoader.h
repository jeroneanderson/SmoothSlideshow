#ifndef THUMBNAILLOADER_H
#define THUMBNAILLOADER_H

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>
#include <QMap>
#include <QSet>
#include <QStringList>

struct CacheMetadata {
    qint64 lastModified;
    qint64 sizeBytes;
    qint64 lastAccess;
    QString cacheFile;
};

class ThumbnailLoader : public QObject {
    Q_OBJECT
public:
    explicit ThumbnailLoader(QObject* parent = nullptr);
    ~ThumbnailLoader();

    void setPaths(const QStringList& paths);
    void updatePriority(int page, int thumbsPerPage);
    void stop();

signals:
    void thumbnailReady(int index, QString path, QImage image);
    void cacheCleared();

public slots:
    void process();
    void cleanCache();
    void clearCache();

private:
    void loadCacheMetadata();
    void saveCacheMetadata();
    QString getCacheFilePath(const QString& path);

    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_abort;
    QStringList m_paths;
    int m_currentPage;
    int m_thumbsPerPage;
    
    QMap<QString, CacheMetadata> m_metadata; // Path -> Metadata
    QString m_cacheDir;
    QString m_metadataFile;
};

#endif // THUMBNAILLOADER_H
