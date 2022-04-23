#ifndef PUBLISHGLFRAME_H
#define PUBLISHGLFRAME_H

#include <QOpenGLWidget>

class PublishGLMapRenderer;

class PublishGLFrame : public QOpenGLWidget
{
    Q_OBJECT
public:
    PublishGLFrame(QWidget *parent = nullptr);
    virtual ~PublishGLFrame() override;

    bool isInitialized() const;
    PublishGLMapRenderer* getRenderer() const;

public slots:
    void cleanup();
    void updateWidget();

    void setRenderer(PublishGLMapRenderer* renderer);
    void clearRenderer();

    void setBackgroundColor(const QColor& color);

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

protected slots:
    void initializeRenderer();

private:
    bool _initialized;
    QSize _targetSize;
    PublishGLMapRenderer* _renderer;

};

#endif // PUBLISHGLFRAME_H
