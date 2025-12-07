#ifndef IMAGECACHELOADER_H
#define IMAGECACHELOADER_H

#include <QObject>
#include <QImage>
#include <QMap>
#include <QMutex>
#include <QCache>
#include <QThread>
#include <QWaitCondition>

class ImageCacheLoader : public QThread {
    Q_OBJECT
public:
    explicit ImageCacheLoader(QObject* parent = nullptr) : QThread(parent) {
        start();
    }
    ~ImageCacheLoader() {
        requestInterruption();
        m_cond.wakeAll();
        wait();
    }

    void requestImage(const QString& path, const QSize& targetSize);

signals:
    void imageLoaded(QString path, QImage image);

protected:
    void run() override;

private:
    struct Request {
        QString path;
        QSize targetSize;
    };
    QList<Request> m_queue;
    QMutex m_mutex;
    QWaitCondition m_cond;
    
    // QCache<QString, QImage> m_cache; // Could add caching if needed
};

#endif // IMAGECACHELOADER_H
