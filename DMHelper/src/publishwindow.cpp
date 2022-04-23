#include "publishwindow.h"
#include "publishglframe.h"
#include "publishglmaprenderer.h"
#include <QKeyEvent>
#include <QDebug>

PublishWindow::PublishWindow(const QString& title, QWidget *parent) :
    QMainWindow(parent),
    _publishFrame(nullptr),
    _globalColor(0, 0, 0, 255),
    _globalColorSet(false)
{
    setWindowTitle(title);

    _publishFrame = new PublishGLFrame(this);
    setCentralWidget(_publishFrame);
    resize(800, 600);

    setBackgroundColor();
}

PublishGLMapRenderer* PublishWindow::getRenderer() const
{
    return _publishFrame ? _publishFrame->getRenderer() : nullptr;
}

void PublishWindow::setBackgroundColor()
{
    if(!_globalColorSet)
    {
        setBackgroundColorStyle(_globalColor);
        _globalColorSet = true;
    }
}

void PublishWindow::setBackgroundColor(const QColor& color)
{
    if(_globalColor != color)
    {
        _globalColor = color;
        _globalColorSet = false;
        setBackgroundColor();
    }
}

void PublishWindow::setRenderer(PublishGLMapRenderer* renderer)
{
    if(renderer)
        renderer->setBackgroundColor(_globalColor);

    _publishFrame->setRenderer(renderer);
}

void PublishWindow::keyPressEvent(QKeyEvent * event)
{
    if(event->key() == Qt::Key_Escape)
    {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
    else if(event->key() == Qt::Key_F)
    {
        setWindowState(windowState() ^ Qt::WindowFullScreen);
    }
    else
    {
        QMainWindow::keyPressEvent(event);
    }
}

void PublishWindow::setBackgroundColorStyle(const QColor& color)
{
    // Make sure the OpenGL background is also set properly
    _publishFrame->setBackgroundColor(color);

    QString styleString("background-color: rgba(");
    styleString += QString::number(color.red());
    styleString += QString(",");
    styleString += QString::number(color.green());
    styleString += QString(",");
    styleString += QString::number(color.blue());
    styleString += QString(",255);");

    qDebug() << "[PublishWindow] changing background color to: " << color << ", string: " << styleString;

    setStyleSheet(styleString);
    _publishFrame->setStyleSheet(styleString);
}
