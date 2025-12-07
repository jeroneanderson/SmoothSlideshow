#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include <QListWidget>
#include <QSlider> 

#include "ThumbnailLoader.h"
#include "SlideshowWidget.h"

// Forward decl
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // UI Actions
    void selectFolder();
    void startSlideshow();
    void resumeSlideshow();
    void quitApplication();
    void toggleControls(); // Escape key

    // Settings
    void saveSettings();
    void clearCache();
    
    // Pagination
    void nextPage();
    void prevPage();

    
    // Thumbnails
    void onThumbnailReady(int index, QString path, QImage image);
    void onThumbnailClicked(QListWidgetItem* item);

private:
    void setupUi();
    void populateThumbnails();
    void displayCurrentPage();
    void calculatePagination();

    // UI Elements
    QWidget* m_centralWidget;
    QStackedWidget* m_stackedWidget;
    
    // Page 0: Grid View
    QWidget* m_gridPage;
    QVBoxLayout* m_mainLayout;
    
    // Top Controls
    QWidget* m_topFrame;
    QPushButton* m_btnSelectFolder;
    QPushButton* m_btnStart;
    QPushButton* m_btnResume;
    QPushButton* m_btnQuit;
    QLabel* m_lblVersion;
    
    // Options
    QWidget* m_optionsFrame;
    QCheckBox* m_chkRecursive;
    QCheckBox* m_chkRandom;
    QCheckBox* m_chkLoop;
    QLineEdit* m_txtDuration;
    QLineEdit* m_txtTransition;
    QLineEdit* m_txtCacheSize;
    QPushButton* m_btnClearCache;
    QSlider* m_sliderZoom;
    
    // Folder Display
    QLabel* m_lblFolder;
    QTextEdit* m_txtFolderDisplay;
    QLabel* m_lblCachePath;
    
    // Thumbs
    QListWidget* m_listWidget; // Using QListWidget for IconMode
    
    // Pagination
    QWidget* m_paginationFrame;
    QPushButton* m_btnPrevPage;
    QLabel* m_lblPage;
    QPushButton* m_btnNextPage;
    
    // Page 1: Slideshow
    SlideshowWidget* m_slideshowPage;

    // Logic
    ThumbnailLoader* m_thumbLoader;
    QThread* m_thumbThread;
    
    QStringList m_allImagePaths;
    QStringList m_displayImagePaths; // might be shuffled
    
    int m_currentPage;
    int m_totalPages;
    int m_thumbsPerPage;
    int m_cols;
    int m_rows;
    
    bool m_controlsVisible;
};

#endif // MAINWINDOW_H
