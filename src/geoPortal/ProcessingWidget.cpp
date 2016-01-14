#include "ProcessingWidget.hpp"
#include "Storage.hpp"
#include "Utils.hpp"
#include "Operations.hpp"

#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QSqlQueryModel>
#include <QTableView>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QDirIterator>

using namespace portal;

ProcessingWidget::ProcessingWidget(const DataManagerPtr& dataManager, QWidget* parent) :
QWidget(parent),
_dataManager(dataManager)
{
    QDirIterator itr("matlab", QDir::Files | QDir::Executable, QDirIterator::Subdirectories);
    while (itr.hasNext())
    {
        _tools << itr.next();
    }

    qDebug() << _tools;

    initUi();
}

ProcessingWidget::~ProcessingWidget()
{
}

void ProcessingWidget::initUi()
{
    _ui.setupUi(this);

    setEnabled(false);

    //---------------------------------------

    QSettings settings;
    _ui.bandSpinBox->setValue(settings.value("ProcessingWidget/BandValue", 1).toInt());
    connect(_ui.bandSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int i)
    {
        QSettings settings;
        settings.setValue("ProcessingWidget/BandValue", _ui.bandSpinBox->value());
    });

    //---------------------------------------

    for (const auto& tool : _tools)
    {
        _ui.toolComboBox->addItem(QFileInfo(tool).fileName());
    }

    if (_ui.toolComboBox->count() == 0)
    {
        _ui.processButton->setEnabled(false);
    }

    connect(_ui.processButton, &QPushButton::clicked, this, &ProcessingWidget::startImageCorrection);
    connect(_ui.showProcessedTableButton, &QPushButton::clicked, this, &ProcessingWidget::showTableWithProcessedFiles);
}

void ProcessingWidget::setScene(const ScenePtr& scene)
{
    if (!scene)
    {
        return;
    }

    if (_scene == scene)
    {
        return;
    }

    _scene = scene;
    _clipInfo.reset();

    setEnabled(true);
    
    _ui.sceneLabel->setText(QString("Сцена %0.\nДля запуска обработки скачайте каналы").arg(_scene->sceneId()));
    _ui.processButton->setEnabled(false);
}

void ProcessingWidget::setSceneAndClip(const ScenePtr& scene, const ClipInfoPtr& clipInfo)
{
    if (!scene)
    {
        return;
    }

    _scene = scene;
    _clipInfo = clipInfo;

    setEnabled(true);

    QString text = QString("Сцена %0.\n").arg(_scene->sceneId());
    if (!_clipInfo->isFullSize())
    {
        text += QString("Фрагмент %0").arg(_clipInfo->uniqueName());
    }
    else
    {
        text += QString("Целиком");
    }

    _ui.sceneLabel->setText(text);
    _ui.processButton->setEnabled(_ui.toolComboBox->count() != 0);

    _ui.bandSpinBox->setMinimum(_clipInfo->minBand());
    _ui.bandSpinBox->setMaximum(_clipInfo->maxBand());
}

void ProcessingWidget::startImageCorrection()
{
    QString program = _tools[_ui.toolComboBox->currentIndex()];

    qDebug() << "Program" << program;

    QString inputFilepath = Storage::sceneBandPath(_scene, _ui.bandSpinBox->value(), _clipInfo);
    if (!QFileInfo::exists(inputFilepath))
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Файл для обработки %0 не найден").arg(inputFilepath));
        return;
    }

    setEnabled(false);

    QString outputFilepath = Storage::processedFilePath(_scene, _ui.bandSpinBox->value(), genetrateRandomName());

    ProcessingOperation* op = new ProcessingOperation(_scene, _ui.bandSpinBox->value(), program, inputFilepath, outputFilepath, &_dataManager->networkAccessManager(), this);
    connect(op, &ProcessingOperation::finished, this, [op, this, outputFilepath]()
    {
        op->deleteLater();
        op->setParent(0);

        QMessageBox::information(qApp->activeWindow(), tr("Обработка"), tr("Обработка завершена. Обработанный файл был загружен на сервер"));

        setEnabled(true);

        openExplorer(outputFilepath);
    });

    connect(op, &ProcessingOperation::error, this, [op, this](const QString& text)
    {
        op->deleteLater();
        op->setParent(0);

        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), text);

        setEnabled(true);
    });
}

void ProcessingWidget::showTableWithProcessedFiles()
{
    QDialog tableWindow;

    QVBoxLayout* layout = new QVBoxLayout;

    QSqlQueryModel* model = new QSqlQueryModel(&tableWindow);
    model->setQuery(QString("SELECT processingtime, appname, band, params, filename FROM public.processedimages where sceneid='%0'").arg(_scene->sceneId()));
    model->setHeaderData(0, Qt::Horizontal, tr("Время обработки"));
    model->setHeaderData(1, Qt::Horizontal, tr("Инструмент"));
    model->setHeaderData(2, Qt::Horizontal, tr("Канал"));
    model->setHeaderData(3, Qt::Horizontal, tr("Параметры обработки"));
    model->setHeaderData(4, Qt::Horizontal, tr("Имя файла"));

    QTableView* view = new QTableView(&tableWindow);
    view->setModel(model);
    view->resizeColumnsToContents();
    layout->addWidget(view);

    QPushButton* button = new QPushButton(tr("Скачать"), &tableWindow);
    button->setMaximumWidth(100);
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, [button, this, view, model]()
    {
        if (!view->currentIndex().isValid())
        {
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Выберите строчку в таблице"));
            return;
        }

        int row = view->currentIndex().row();
        QString filename = model->record(row).value("filename").toString();

        downloadProcessedFile(filename);
    });

    tableWindow.setLayout(layout);
    tableWindow.setWindowTitle(tr("Обработанные файлы для сцены %0").arg(_scene->sceneId()));
    tableWindow.resize(800, 300);
    tableWindow.exec();
}

void ProcessingWidget::downloadProcessedFile(const QString& filename)
{
    QString filepath = Storage::processedFileDir(_scene).filePath(filename);
    if (QFileInfo::exists(filepath))
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Файл уже получен"));

        openExplorer(filepath);
        return;
    }

    QUrl url = QString("http://virtualglobe.ru/geoportal/Hyperion/scenes/%0/processed/%1").arg(_scene->sceneId()).arg(filename);

    QNetworkReply* reply = _dataManager->networkAccessManager().get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [reply, this]()
    {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << "Error " << reply->error() << " " << reply->errorString();
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Не удалось скачать файл. Ошибка %0 %1").arg(reply->error()).arg(reply->errorString()));
            return;
        }

        QByteArray data = reply->readAll();
        if (data.isNull() || data.isEmpty())
        {
            qDebug() << "Reply is null or empty";
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Не удалось скачать файл. Пустой ответ от сервера"));
            return;
        }

        QString path = Storage::processedFileDir(_scene).filePath(reply->url().fileName());

        QFile localFile(path);
        localFile.open(QIODevice::WriteOnly);
        localFile.write(data);
        localFile.close();

        QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Файл был успешно получен"));

        openExplorer(path);
    });
}