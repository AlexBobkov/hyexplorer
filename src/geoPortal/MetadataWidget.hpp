#pragma once

#include <QLabel>

class MetadataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MetadataWidget();
    virtual ~MetadataWidget();

public slots:
    void setSceneId(const QString& sceneid);

private:
    void initUi();
};
