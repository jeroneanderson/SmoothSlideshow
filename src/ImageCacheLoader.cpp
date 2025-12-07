#include "ImageCacheLoader.h"
#include <QImageReader>

void ImageCacheLoader::requestImage(const QString& path, const QSize& targetSize) {
    QMutexLocker locker(&m_mutex);
    // Remove if already in queue?
    // Simple LIFO or FIFO? FIFO is usually better for "next slide" type things.
    // If the queue is too huge, we might clear old requests.
    if (m_queue.size() > 5) m_queue.removeFirst(); // keep it small
    
    m_queue.append({path, targetSize});
    m_cond.wakeOne();
}

void ImageCacheLoader::run() {
    while (!isInterruptionRequested()) {
        Request req;
        {
            QMutexLocker locker(&m_mutex);
            if (m_queue.isEmpty()) {
                m_cond.wait(&m_mutex);
                if (isInterruptionRequested()) break;
            }
            if (m_queue.isEmpty()) continue;
            req = m_queue.takeFirst();
        }
        
        QImageReader reader(req.path);
        // We want to scale to fit targetSize but keep aspect ratio
        QSize orig = reader.size();
        if (orig.isValid()) {
            QSize scaled = orig.scaled(req.targetSize, Qt::KeepAspectRatio);
            reader.setScaledSize(scaled);
            QImage img = reader.read();
            if (!img.isNull()) {
                emit imageLoaded(req.path, img);
            }
        }
    }
}
