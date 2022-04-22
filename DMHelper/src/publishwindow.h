#ifndef PUBLISHWINDOW_H
#define PUBLISHWINDOW_H

#include <QMainWindow>

class PublishGLFrame;
class PublishGLRenderer;

class PublishWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit PublishWindow(const QString& title, QWidget *parent = nullptr);

    PublishGLRenderer* getRenderer() const;

public slots:
    void setBackgroundColor();
    void setBackgroundColor(const QColor& color);

    void setRenderer(PublishGLRenderer* renderer);

protected:
    virtual void keyPressEvent(QKeyEvent * event);

    void setBackgroundColorStyle(const QColor& color);

private:
    PublishGLFrame* _publishFrame;
    QColor _globalColor;
    bool _globalColorSet;
};

#endif // PUBLISHWINDOW_H
