#include "MainWindow.h"
#include "ConfigManager.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QResizeEvent>
#include <QDirIterator>
#include <QKeyEvent>
#include <QDebug>
#include <QDateTime>
#include <QPainter>
#include <algorithm> // for std::shuffle
#include <random>    // for std::default_random_engine

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_currentPage(0), m_totalPages(0), m_thumbsPerPage(20),
      m_controlsVisible(true)
{
    // Window Setup
    setWindowTitle("Smooth Slideshow C++ v1.0.45");
    // Window Setup
    setWindowTitle("Smooth Slideshow C++ v1.0.45");
    // Styling is handled globally in main.cpp via Fusion style and QPalette
                  
    // Threading Setup
    m_thumbThread = new QThread(this);
    m_thumbLoader = new ThumbnailLoader();
    m_thumbLoader->moveToThread(m_thumbThread);
    
    connect(m_thumbThread, &QThread::started, m_thumbLoader, &ThumbnailLoader::process);
    connect(m_thumbLoader, &ThumbnailLoader::thumbnailReady, this, &MainWindow::onThumbnailReady);
    connect(m_thumbLoader, &ThumbnailLoader::cacheCleared, this, [this](){
        QMessageBox::information(this, "Cache Cleared", "Thumbnail cache has been cleared.");
        populateThumbnails();
    });
    
    m_thumbThread->start();

    setupUi();
    
    // Load config
    ConfigManager::instance().load();
    
    // Apply Config to UI
    m_chkRecursive->setChecked(ConfigManager::instance().recursive());
    m_chkRandom->setChecked(ConfigManager::instance().randomOrder());
    m_chkLoop->setChecked(ConfigManager::instance().continuousLoop());
    m_txtDuration->setText(QString::number(ConfigManager::instance().slideDuration()));
    m_txtTransition->setText(QString::number(ConfigManager::instance().transitionTime()));
    m_txtCacheSize->setText(QString::number(ConfigManager::instance().cacheMaxSizeMB()));
    
    QString lastFolder = ConfigManager::instance().lastFolder();
    if (!lastFolder.isEmpty() && QDir(lastFolder).exists()) {
        m_txtFolderDisplay->setText(lastFolder);
        // Delay populate to ensure UI is ready
        QTimer::singleShot(100, this, &MainWindow::populateThumbnails);
    }
}

MainWindow::~MainWindow() {
    m_thumbLoader->stop();
    m_thumbThread->quit();
    m_thumbThread->wait();
    delete m_thumbLoader; // safe because thread stopped
}

void MainWindow::setupUi() {
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    QVBoxLayout* layout = new QVBoxLayout(m_centralWidget);
    layout->setContentsMargins(0,0,0,0);
    
    m_stackedWidget = new QStackedWidget(this);
    layout->addWidget(m_stackedWidget);
    
    // --- Page 0: Grid View ---
    m_gridPage = new QWidget();
    m_mainLayout = new QVBoxLayout(m_gridPage);
    
    // Top Controls
    m_topFrame = new QWidget();
    QHBoxLayout* topLayout = new QHBoxLayout(m_topFrame);
    m_btnSelectFolder = new QPushButton("Select Folder");
    m_btnStart = new QPushButton("Start Slideshow");
    m_btnResume = new QPushButton("Resume Slideshow");
    m_btnQuit = new QPushButton("Quit Application");
    m_lblVersion = new QLabel("v1.0.45");
    
    topLayout->addWidget(m_btnSelectFolder);
    topLayout->addWidget(m_btnStart);
    topLayout->addWidget(m_btnResume);
    topLayout->addWidget(m_btnQuit);
    topLayout->addStretch();
    topLayout->addWidget(m_lblVersion);
    
    m_mainLayout->addWidget(m_topFrame);
    
    // Options
    m_optionsFrame = new QWidget();
    QHBoxLayout* optLayout = new QHBoxLayout(m_optionsFrame);
    
    m_chkRecursive = new QCheckBox("Search Subfolders");
    m_chkRandom = new QCheckBox("Random Order");
    m_chkLoop = new QCheckBox("Continuous Loop");
    
    optLayout->addWidget(m_chkRecursive);
    optLayout->addWidget(m_chkRandom);
    optLayout->addWidget(m_chkLoop);
    
    optLayout->addWidget(new QLabel("Duration:"));
    m_txtDuration = new QLineEdit(); m_txtDuration->setFixedWidth(50);
    optLayout->addWidget(m_txtDuration);
    
    optLayout->addWidget(new QLabel("Transition:"));
    m_txtTransition = new QLineEdit(); m_txtTransition->setFixedWidth(50);
    optLayout->addWidget(m_txtTransition);
    
    optLayout->addWidget(new QLabel("Cache (MB):"));
    m_txtCacheSize = new QLineEdit(); m_txtCacheSize->setFixedWidth(50);
    optLayout->addWidget(m_txtCacheSize);
    
    m_btnClearCache = new QPushButton("Clear Cache");
    optLayout->addWidget(m_btnClearCache);

    optLayout->addWidget(new QLabel("Zoom:"));
    m_sliderZoom = new QSlider(Qt::Horizontal);
    m_sliderZoom->setRange(50, 300);
    m_sliderZoom->setValue(150);
    m_sliderZoom->setFixedWidth(100);
    optLayout->addWidget(m_sliderZoom);

    optLayout->addStretch();
    
    m_mainLayout->addWidget(m_optionsFrame);
    
    // Folder Display
    m_lblFolder = new QLabel("Current Folder:");
    m_mainLayout->addWidget(m_lblFolder);
    m_txtFolderDisplay = new QTextEdit();
    m_txtFolderDisplay->setFixedHeight(30);
    m_txtFolderDisplay->setReadOnly(true);
    m_mainLayout->addWidget(m_txtFolderDisplay);
    
    m_lblCachePath = new QLabel("Cache Path: " + QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/Endless_Slides/thumbnails");
    m_lblCachePath->setStyleSheet("color: #888; font-size: 10px;");
    m_mainLayout->addWidget(m_lblCachePath);
    
    // Thumbnails
    m_listWidget = new QListWidget();
    m_listWidget->setViewMode(QListWidget::IconMode);
    m_listWidget->setIconSize(QSize(150, 150));
    m_listWidget->setResizeMode(QListWidget::Adjust);
    m_listWidget->setGridSize(QSize(170, 200)); // Spacing
    m_listWidget->setSpacing(5);
    m_listWidget->setStyleSheet("QListWidget::item { color: #ccc; } QListWidget::item:selected { background-color: #444; }");
    m_mainLayout->addWidget(m_listWidget);
    
    // Pagination
    m_paginationFrame = new QWidget();
    QHBoxLayout* pageLayout = new QHBoxLayout(m_paginationFrame);
    m_btnPrevPage = new QPushButton("< Prev");
    m_lblPage = new QLabel("Page 0/0");
    m_btnNextPage = new QPushButton("Next >");
    
    pageLayout->addStretch();
    pageLayout->addWidget(m_btnPrevPage);
    pageLayout->addWidget(m_lblPage);
    pageLayout->addWidget(m_btnNextPage);
    pageLayout->addStretch();
    
    m_mainLayout->addWidget(m_paginationFrame);
    
    m_stackedWidget->addWidget(m_gridPage);
    
    // --- Page 1: Slideshow ---
    m_slideshowPage = new SlideshowWidget();
    m_stackedWidget->addWidget(m_slideshowPage);
    
    // Connections
    connect(m_btnSelectFolder, &QPushButton::clicked, this, &MainWindow::selectFolder);
    connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::startSlideshow);
    connect(m_btnResume, &QPushButton::clicked, this, &MainWindow::resumeSlideshow);
    connect(m_btnQuit, &QPushButton::clicked, this, &MainWindow::quitApplication);
    connect(m_btnClearCache, &QPushButton::clicked, this, &MainWindow::clearCache);
    
    connect(m_btnNextPage, &QPushButton::clicked, this, &MainWindow::nextPage);
    connect(m_btnPrevPage, &QPushButton::clicked, this, &MainWindow::prevPage);
    
    connect(m_chkRecursive, &QCheckBox::stateChanged, this, &MainWindow::saveSettings);
    connect(m_chkRandom, &QCheckBox::stateChanged, [this](int){ saveSettings(); populateThumbnails(); });
    connect(m_chkLoop, &QCheckBox::stateChanged, this, &MainWindow::saveSettings);
    
    connect(m_txtDuration, &QLineEdit::editingFinished, this, &MainWindow::saveSettings);
    connect(m_txtTransition, &QLineEdit::editingFinished, this, &MainWindow::saveSettings);
    connect(m_txtCacheSize, &QLineEdit::editingFinished, this, &MainWindow::saveSettings);
    
    connect(m_listWidget, &QListWidget::itemClicked, this, &MainWindow::onThumbnailClicked);
    
    connect(m_sliderZoom, &QSlider::valueChanged, [this](int value){
         m_listWidget->setIconSize(QSize(value, value));
         m_listWidget->setGridSize(QSize(value + 20, value + 50)); // More vertical space for text
         calculatePagination();
         displayCurrentPage();
    });
}

void MainWindow::selectFolder() {
    QString folder = QFileDialog::getExistingDirectory(this, "Select Image Folder", 
                        ConfigManager::instance().lastFolder());
    if (folder.isEmpty()) return;
    
    m_txtFolderDisplay->setText(folder);
    ConfigManager::instance().setLastFolder(folder);
    ConfigManager::instance().save();
    
    populateThumbnails();
}

void MainWindow::populateThumbnails() {
    m_listWidget->clear();
    m_allImagePaths.clear();
    
    QString folder = ConfigManager::instance().lastFolder();
    bool recursive = ConfigManager::instance().recursive();
    
    if (folder.isEmpty() || !QDir(folder).exists()) return;
    
    // Collect images
    QStringList exts = {"*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif"};
    QDirIterator it(folder, exts, QDir::Files, 
                   recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
                   
    while (it.hasNext()) {
        m_allImagePaths << it.next();
    }
    
    // Sort or Shuffle
    if (ConfigManager::instance().randomOrder()) {
        auto rng = std::default_random_engine(QDateTime::currentMSecsSinceEpoch());
        std::shuffle(m_allImagePaths.begin(), m_allImagePaths.end(), rng);
    } else {
        m_allImagePaths.sort();
    }
    
    m_displayImagePaths = m_allImagePaths;
    m_slideshowPage->setImagePaths(m_displayImagePaths);
    
    // Reset pagination
    m_currentPage = 0;
    calculatePagination();
    displayCurrentPage();
}

void MainWindow::calculatePagination() {
    int w = m_listWidget->width();
    int h = m_listWidget->height();
    if (w < 100) w = 800; // default if not shown yet
    if (h < 100) h = 600;
    
    
    // Estimation
    int itemW = m_listWidget->gridSize().width();
    int itemH = m_listWidget->gridSize().height();
    if (itemW <= 0) itemW = 170; // fallback
    if (itemH <= 0) itemH = 180;
    
    m_cols = qMax(1, w / itemW);
    m_rows = qMax(1, h / itemH);
    m_thumbsPerPage = m_cols * m_rows;
    
    if (m_displayImagePaths.isEmpty()) {
        m_totalPages = 0;
    } else {
        m_totalPages = (m_displayImagePaths.size() + m_thumbsPerPage - 1) / m_thumbsPerPage;
    }
}

void MainWindow::displayCurrentPage() {
    m_listWidget->clear();
    
    if (m_displayImagePaths.isEmpty()) {
        m_lblPage->setText("Page 0/0");
        return;
    }
    
    // Ensure bounds
    if (m_currentPage >= m_totalPages) m_currentPage = m_totalPages - 1;
    if (m_currentPage < 0) m_currentPage = 0;
    
    m_lblPage->setText(QString("Page %1/%2").arg(m_currentPage + 1).arg(m_totalPages));
    
    int startIdx = m_currentPage * m_thumbsPerPage;
    int endIdx = qMin(startIdx + m_thumbsPerPage, m_displayImagePaths.size());
    
    // Prepare items with placeholders
    for (int i = startIdx; i < endIdx; ++i) {
        QString path = m_displayImagePaths[i];
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(QFileInfo(path).fileName());
        // item->setData(Qt::UserRole, path); // Store path
        item->setData(Qt::UserRole, i); // Store global index
        
        // Placeholder icon?
        QSize iconSize = m_listWidget->iconSize();
        QPixmap placeholder(iconSize);
        placeholder.fill(Qt::black);
        item->setIcon(QIcon(placeholder));
        
        m_listWidget->addItem(item);
    }
    
    // Update thread priority
    QStringList pagePaths;
    for (int i = startIdx; i < endIdx; ++i) pagePaths << m_displayImagePaths[i];
    
    m_thumbLoader->setPaths(m_displayImagePaths); // Send all, but...
    m_thumbLoader->updatePriority(m_currentPage, m_thumbsPerPage);
}

void MainWindow::onThumbnailReady(int index, QString path, QImage image) {
    // Check if this index is on current page
    int startIdx = m_currentPage * m_thumbsPerPage;
    int endIdx = startIdx + m_thumbsPerPage;
    
    if (index >= startIdx && index < endIdx) {
        int localRow = index - startIdx;
        if (localRow < m_listWidget->count()) {
            QListWidgetItem* item = m_listWidget->item(localRow);
            if (item) {
                // Determine layout size
                QSize iconSize = m_listWidget->iconSize();
                QPixmap canvas(iconSize);
                canvas.fill(Qt::transparent);
                
                // Draw centered
                QPainter p(&canvas);
                QPixmap thumb = QPixmap::fromImage(image);
                int x = (iconSize.width() - thumb.width()) / 2;
                int y = (iconSize.height() - thumb.height()) / 2;
                p.drawPixmap(x, y, thumb);
                p.end();

                item->setIcon(QIcon(canvas));
            }
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    if (m_stackedWidget->currentIndex() == 0) {
        // Debounce?
        QTimer::singleShot(200, this, [this](){
            calculatePagination();
            // Only refresh if page count changed significantly? 
            // Or just refresh to fit new layout
            displayCurrentPage();
        });
    }
}

void MainWindow::nextPage() {
    if (m_currentPage < m_totalPages - 1) {
        m_currentPage++;
        displayCurrentPage();
    }
}

void MainWindow::prevPage() {
    if (m_currentPage > 0) {
        m_currentPage--;
        displayCurrentPage();
    }
}

void MainWindow::startSlideshow() {
    m_slideshowPage->startSlideshow(0);
    toggleControls();
}

void MainWindow::onThumbnailClicked(QListWidgetItem* item) {
    int idx = item->data(Qt::UserRole).toInt();
    m_slideshowPage->startSlideshow(idx);
    toggleControls();
}

void MainWindow::resumeSlideshow() {
    m_slideshowPage->resume();
    toggleControls();
}

void MainWindow::toggleControls() {
    if (m_stackedWidget->currentIndex() == 0) {
        // Grid -> Slideshow
        m_stackedWidget->setCurrentIndex(1);
        // Fullscreen? 
        if (!isFullScreen()) showFullScreen();
    } else {
        // Slideshow -> Grid
        m_slideshowPage->pause();
        m_stackedWidget->setCurrentIndex(0);
        // showNormal(); // Optional, keeps fullscreen naturally
        if (isFullScreen()) showNormal();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        toggleControls();
    } else if (event->key() == Qt::Key_Space) {
        if (m_stackedWidget->currentIndex() == 1) {
            if (m_slideshowPage->isPaused()) m_slideshowPage->resume();
            else m_slideshowPage->pause();
        }
    } else if (event->key() == Qt::Key_Right) {
        if (m_stackedWidget->currentIndex() == 1) m_slideshowPage->nextSlide();
        else nextPage();
    } else if (event->key() == Qt::Key_Left) {
        if (m_stackedWidget->currentIndex() == 1) m_slideshowPage->prevSlide();
        else prevPage();
    }
}

void MainWindow::saveSettings() {
    ConfigManager& cfg = ConfigManager::instance();
    cfg.setRecursive(m_chkRecursive->isChecked());
    cfg.setRandomOrder(m_chkRandom->isChecked());
    cfg.setContinuousLoop(m_chkLoop->isChecked());
    cfg.setSlideDuration(m_txtDuration->text().toDouble());
    cfg.setTransitionTime(m_txtTransition->text().toDouble());
    cfg.setCacheMaxSizeMB(m_txtCacheSize->text().toDouble());
    cfg.save();
}

void MainWindow::quitApplication() {
    close();
}

void MainWindow::clearCache() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Cache", "Are you sure you want to clear the thumbnail cache?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QMetaObject::invokeMethod(m_thumbLoader, "clearCache", Qt::QueuedConnection);
    }
}
