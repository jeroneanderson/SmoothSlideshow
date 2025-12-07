#include "SlideshowWidget.h"
#include <QPainter>
#include <QDebug>
#include "ConfigManager.h"

SlideshowWidget::SlideshowWidget(QWidget *parent)
    : QWidget(parent), m_currentIndex(-1), m_nextIndex(-1), 
      m_running(false), m_paused(false), m_isTransitioning(false), m_opacity(0.0)
{
    setAttribute(Qt::WA_OpaquePaintEvent); // Optimization
    setAutoFillBackground(false); 
    
    m_animationTimer = new QTimer(this);
    m_animationTimer->setInterval(16); // ~60 FPS
    connect(m_animationTimer, &QTimer::timeout, this, &SlideshowWidget::updateAnimation);
    
    m_slideTimer = new QTimer(this);
    m_slideTimer->setSingleShot(true);
    connect(m_slideTimer, &QTimer::timeout, this, &SlideshowWidget::nextSlide);
    
    m_imageLoader = new ImageCacheLoader(this);
    connect(m_imageLoader, &ImageCacheLoader::imageLoaded, this, &SlideshowWidget::onImageLoaded);
}

SlideshowWidget::~SlideshowWidget() {
    // Timers are children -> auto delete
}

void SlideshowWidget::setImagePaths(const QStringList& paths) {
    m_paths = paths;
}

void SlideshowWidget::startSlideshow(int startIndex) {
    if (m_paths.isEmpty()) return;
    
    m_currentIndex = startIndex;
    if (m_currentIndex < 0 || m_currentIndex >= m_paths.size()) m_currentIndex = 0;
    
    m_running = true;
    m_paused = false;
    m_isTransitioning = false;
    m_opacity = 0.0;
    
    // Load current directly for instant start?
    // We'll request it and show black until loaded
    m_currentImage = QImage();
    m_nextImage = QImage();
    m_nextIndex = -1;
    
    m_imageLoader->requestImage(m_paths[m_currentIndex], size());
    
    // Schedule next slide
    double dur = ConfigManager::instance().slideDuration();
    m_slideTimer->start(dur * 1000);
}

void SlideshowWidget::stopSlideshow() {
    m_running = false;
    m_slideTimer->stop();
    m_animationTimer->stop();
    // Do not kill the loader thread here, keep it alive for next run
    // m_imageLoader->requestInterruption(); 
}

void SlideshowWidget::pause() {
    m_paused = true;
    m_slideTimer->stop(); // freeze timer
    m_animationTimer->stop(); 
}

void SlideshowWidget::resume() {
    if (m_paused) {
        m_paused = false;
        // restart timer? or just resume?
        // simple resume:
        if (!m_isTransitioning) {
             m_slideTimer->start(ConfigManager::instance().slideDuration() * 1000);
        } else {
             m_animationTimer->start();
        }
    }
}

void SlideshowWidget::nextSlide() {
    if (m_paths.isEmpty() || !m_running || m_paused) return;
    
    int next = m_currentIndex + 1;
    if (next >= m_paths.size()) {
        if (ConfigManager::instance().continuousLoop()) {
            next = 0;
        } else {
            stopSlideshow();
            return;
        }
    }
    
    transitionToImage(next);
}

void SlideshowWidget::prevSlide() {
    // Manual nav
    if (m_paths.isEmpty()) return;
    
    int prev = m_currentIndex - 1;
    if (prev < 0) prev = m_paths.size() - 1;
    
    transitionToImage(prev);
}

void SlideshowWidget::transitionToImage(int index) {
    if (index < 0 || index >= m_paths.size()) return;
    
    m_nextIndex = index;
    // Request image
    m_imageLoader->requestImage(m_paths[m_nextIndex], size());
    
    // We wait for onImageLoaded to actually start the animation
}

void SlideshowWidget::onImageLoaded(QString path, QImage image) {
    // Check if this is the image we are waiting for
    // If we are starting up (currentIndex defined but m_currentImage null)
    if (m_currentImage.isNull() && !m_paths.isEmpty() && path == m_paths[m_currentIndex]) {
        m_currentImage = image;
        update();
        return;
    }
    
    // If we are transitioning
    if (m_nextIndex != -1 && path == m_paths[m_nextIndex]) {
        m_nextImage = image;
        m_isTransitioning = true;
        m_opacity = 0.0;
        m_animationTimer->start();
    }
}

void SlideshowWidget::updateAnimation() {
    if (!m_isTransitioning) {
        m_animationTimer->stop();
        return;
    }
    
    double transitionTime = ConfigManager::instance().transitionTime();
    double step = 0.016 / (transitionTime > 0 ? transitionTime : 0.016); // roughly
    
    m_opacity += step;
    
    if (m_opacity >= 1.0) {
        m_opacity = 1.0;
        m_isTransitioning = false;
        m_animationTimer->stop();
        
        m_currentIndex = m_nextIndex;
        m_currentImage = m_nextImage;
        m_nextImage = QImage();
        m_nextIndex = -1;
        
        // Schedule next
        if (m_running && !m_paused) {
             m_slideTimer->start(ConfigManager::instance().slideDuration() * 1000);
        }
    }
    
    update();
}

void SlideshowWidget::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black); // Background
    
    if (m_currentImage.isNull()) return;
    
    // Draw Current
    // Center it
    int x1 = (width() - m_currentImage.width()) / 2;
    int y1 = (height() - m_currentImage.height()) / 2;
    p.drawImage(x1, y1, m_currentImage);
    
    if (m_isTransitioning && !m_nextImage.isNull()) {
        p.setOpacity(m_opacity);
        int x2 = (width() - m_nextImage.width()) / 2;
        int y2 = (height() - m_nextImage.height()) / 2;
        p.drawImage(x2, y2, m_nextImage);
    }
}
