#ifndef SLIDESHOWWIDGET_H
#define SLIDESHOWWIDGET_H

#include <QWidget>
#include <QImage>
#include <QTimer>
#include <QThread>
#include "ImageCacheLoader.h" 

// Forward decl
class SlideshowWidget : public QWidget {
    Q_OBJECT
public:
    explicit SlideshowWidget(QWidget *parent = nullptr);
    ~SlideshowWidget();

    void setImagePaths(const QStringList& paths);
    void startSlideshow(int startIndex);
    void stopSlideshow();
    void nextSlide();
    void prevSlide();
    void pause();
    void resume();
    
    bool isRunning() const { return m_running; }
    bool isPaused() const { return m_paused; }

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateAnimation();
    void onImageLoaded(QString path, QImage image);

private:
    void transitionToImage(int index);

    QStringList m_paths;
    int m_currentIndex;
    int m_nextIndex;
    
    QImage m_currentImage;
    QImage m_nextImage;
    QImage m_blendedImage; // If doing CPU blending
    
    bool m_running;
    bool m_paused;
    bool m_isTransitioning;
    float m_opacity; // 0.0 to 1.0 (0=current, 1=next)
    
    QTimer* m_animationTimer;
    QTimer* m_slideTimer;
    
    // Helper to load async?
    // For simplicity, we might load in main thread if performant enough or use a worker.
    // Given the "native app speed" requirement, we definitely want async loading.
    ImageCacheLoader* m_imageLoader; 
};

#endif // SLIDESHOWWIDGET_H
