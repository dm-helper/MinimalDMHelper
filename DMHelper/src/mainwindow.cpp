#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "publishwindow.h"
#include "dmh_vlc.h"
#include "publishglmapvideorenderer.h"
#include <QDebug>
#include <QDir>
#include <QLibraryInfo>
#include <QSurfaceFormat>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _pubWindow(nullptr)
{
    qDebug() << "[MainWindow] Initializing Main";

    qDebug() << "[MainWindow]     Working Directory: " << QDir::currentPath();
    qDebug() << "[MainWindow]     Executable Directory: " << QCoreApplication::applicationDirPath();

    qDebug() << "[MainWindow] Qt Information";
    qDebug() << "[MainWindow]     Qt Version: " << QLibraryInfo::version().toString();
    qDebug() << "[MainWindow]     Is Debug? " << QLibraryInfo::isDebugBuild();
    qDebug() << "[MainWindow]     PrefixPath: " << QLibraryInfo::location(QLibraryInfo::PrefixPath);
    qDebug() << "[MainWindow]     DocumentationPath: " << QLibraryInfo::location(QLibraryInfo::DocumentationPath);
    qDebug() << "[MainWindow]     HeadersPath: " << QLibraryInfo::location(QLibraryInfo::HeadersPath);
    qDebug() << "[MainWindow]     LibrariesPath: " << QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    qDebug() << "[MainWindow]     LibraryExecutablesPath: " << QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath);
    qDebug() << "[MainWindow]     BinariesPath: " << QLibraryInfo::location(QLibraryInfo::BinariesPath);
    qDebug() << "[MainWindow]     PluginsPath: " << QLibraryInfo::location(QLibraryInfo::PluginsPath);
    qDebug() << "[MainWindow]     ImportsPath: " << QLibraryInfo::location(QLibraryInfo::ImportsPath);
    qDebug() << "[MainWindow]     Qml2ImportsPath: " << QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath);
    qDebug() << "[MainWindow]     ArchDataPath: " << QLibraryInfo::location(QLibraryInfo::ArchDataPath);
    qDebug() << "[MainWindow]     DataPath: " << QLibraryInfo::location(QLibraryInfo::DataPath);
    qDebug() << "[MainWindow]     TranslationsPath: " << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    qDebug() << "[MainWindow]     ExamplesPath: " << QLibraryInfo::location(QLibraryInfo::ExamplesPath);
    qDebug() << "[MainWindow]     TestsPath: " << QLibraryInfo::location(QLibraryInfo::TestsPath);
    qDebug() << "[MainWindow]     SettingsPath: " << QLibraryInfo::location(QLibraryInfo::SettingsPath);

    QSurfaceFormat fmt;
    qDebug() << "[MainWindow] OpenGL Information";
    qDebug() << "[MainWindow]     Version: " << fmt.majorVersion() << "." << fmt.minorVersion();
    qDebug() << "[MainWindow]     Device pixel ratio: " << this->devicePixelRatio();

    ui->setupUi(this);
    QScreen* screen = QGuiApplication::primaryScreen();
    if(screen)
        resize(screen->availableSize().width() * 4 / 5, screen->availableSize().height() * 4 / 5);

    qDebug() << "[MainWindow] Creating Player's Window";
    _pubWindow = new PublishWindow(QString("DMHelper Player's Window"));
    _pubWindow->setBackgroundColor(Qt::blue);
    qDebug() << "[MainWindow] Player's Window Created";

    connect(ui->btnPublish, &QAbstractButton::clicked, this, &MainWindow::showPublishWindow);

    qDebug() << "[MainWindow] Main Initialization complete";
}

MainWindow::~MainWindow()
{
    delete _pubWindow;
    delete ui;

    DMH_VLC::Shutdown();
}

void MainWindow::showPublishWindow(bool clicked)
{
    _pubWindow->show();

    if(clicked)
        _pubWindow->setRenderer(new PublishGLMapVideoRenderer());
    else
        _pubWindow->setRenderer(nullptr);
}
