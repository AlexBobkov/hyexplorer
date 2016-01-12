#include "ProcessingWidget.hpp"
#include "Storage.hpp"
#include "Utils.hpp"

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

using namespace portal;

ProcessingWidget::ProcessingWidget(const DataManagerPtr& dataManager, QWidget* parent) :
QWidget(parent),
_dataManager(dataManager)
{
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
        
    connect(_ui.processButton, &QPushButton::clicked, this, &ProcessingWidget::startImageCorrection);    
    connect(_ui.showProcessedTableButton, &QPushButton::clicked, this, &ProcessingWidget::showTableWithProcessedFiles);
}

void ProcessingWidget::setSceneAndClip(const ScenePtr& scene, const ClipInfoPtr& clipInfo)
{
    //if (!isEnabled())
    //{
    //    qDebug() << "Attemp to change scene and clip during processing";
    //    return;
    //}

    _scene = scene;
    _clipInfo = clipInfo;

    setEnabled(true);

    QString text = QString("Сцена %0. ").arg(_scene->sceneId());
    if (!_clipInfo->isFullSize())
    {
        text += QString("Фрагмент %0").arg(_clipInfo->uniqueName());
    }
    else
    {
        text += QString("Целиком");
    }

    _ui.sceneLabel->setText(text);

    _ui.bandSpinBox->setMinimum(_clipInfo->minBand());
    _ui.bandSpinBox->setMaximum(_clipInfo->maxBand());
}

void ProcessingWidget::startImageCorrection()
{
    QString program = "matlab/ImageCorrectionTool/ImageCorrectionTool.exe";
    if (!QFileInfo::exists(program))
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Программа для коррекции изображений matlab/ImageCorrectionTool.exe не найдена"));
        return;
    }

    QString filepath = Storage::sceneBandPath(_scene, _ui.bandSpinBox->value(), _clipInfo);
    if (!QFileInfo::exists(filepath))
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Файл для обработки %0 не найден").arg(filepath));
        return;
    }

    //------------------------------------

    {
        QFile data("matlab/ImageCorrectionTool/data.txt");
        data.open(QFile::WriteOnly);

        QTextStream out(&data);
        out << filepath.toLocal8Bit() << "\n";

        data.close();
    }

    //------------------------------------

    {
        _proccessedOutputFilepath = Storage::processedFilePath(_scene, _ui.bandSpinBox->value(), genetrateRandomName());

        QFile result("matlab/ImageCorrectionTool/result.txt");
        result.open(QFile::WriteOnly);

        QTextStream out(&result);
        out << _proccessedOutputFilepath.toLocal8Bit() << "\n";

        result.close();
    }

    //------------------------------------
        
    qDebug() << "Image correction started";

    setEnabled(false);

    emit processingStarted();
    
    QProcess* matlabProcess = new QProcess(this);
    matlabProcess->setWorkingDirectory("matlab/ImageCorrectionTool");

    connect(matlabProcess, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, [matlabProcess, this](QProcess::ProcessError error)
    {
        qDebug() << "Image correnction failed. Error code" << error;

        matlabProcess->deleteLater();

        setEnabled(true);

        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Ошибка при выполнении обработки. Код %0").arg(error));

        emit processingFinished();
    });

    connect(matlabProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [matlabProcess, this](int exitCode, QProcess::ExitStatus exitStatus)
    {
        qDebug() << "Image correnction finished. Exit code" << exitCode << "exit status" << exitStatus;
                
        matlabProcess->deleteLater();

        if (QFileInfo::exists(_proccessedOutputFilepath))
        {
            openExplorer(_proccessedOutputFilepath);

            uploadProccessedFile();
        }
        else
        {
            setEnabled(true);

            QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Обработка прервана пользователем"));

            emit processingFinished();
        }
    });

    matlabProcess->start(program, QStringList());
}

void ProcessingWidget::uploadProccessedFile()
{
    qDebug() << "Upload " << _proccessedOutputFilepath;

    double contrast = 123.0;
    double sharpness = 45.0;
    int blocksize = 666;

    //------------------------------------

    QFileInfo fileInfo(_proccessedOutputFilepath);

    QString paramsFilepath = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName() + ".txt";
    if (!QFileInfo::exists(paramsFilepath))
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Не найден файл с параметрами обработки %0").arg(paramsFilepath));

        emit processingFinished();

        return;
    }

    QFile paramsFile(paramsFilepath);
    paramsFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&paramsFile);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        qDebug() << "Read line" << line;
    }

    //------------------------------------

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    //------------------------------------

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/octet-stream"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"file\";filename=\"%0\"").arg(fileInfo.fileName()));

    QFile* file = new QFile(_proccessedOutputFilepath);
    file->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

    multiPart->append(imagePart);

    //------------------------------------

    QHttpPart bandPart;
    bandPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"band\""));
    bandPart.setBody(QByteArray::number(_ui.bandSpinBox->value()));

    multiPart->append(bandPart);

    //------------------------------------

    QHttpPart contrastPart;
    contrastPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"contrast\""));
    contrastPart.setBody(QByteArray::number(contrast, 'f', 7));

    multiPart->append(contrastPart);

    //------------------------------------

    QHttpPart sharpnessPart;
    sharpnessPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"sharpness\""));
    sharpnessPart.setBody(QByteArray::number(sharpness, 'f', 7));

    multiPart->append(sharpnessPart);

    //------------------------------------

    QHttpPart blocksizePart;
    blocksizePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"blocksize\""));
    blocksizePart.setBody(QByteArray::number(blocksize));

    multiPart->append(blocksizePart);

    //------------------------------------

    QUrl url = QString("http://virtualglobe.ru/geoportalapi/processed/%0").arg(_scene->sceneId());

    QNetworkReply* reply = _dataManager->networkAccessManager().post(QNetworkRequest(url), multiPart);

    multiPart->setParent(reply); // delete the multiPart with the reply

    connect(reply, &QNetworkReply::finished, this, [reply, this]()
    {
        reply->deleteLater();

        setEnabled(true);

        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << "Error " << reply->error() << " " << reply->errorString();            

            QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Ошибка при загрузке файла на сервер %1 %2").arg(reply->error()).arg(reply->errorString()));

            emit processingFinished();

            return;
        }

        QMessageBox::information(qApp->activeWindow(), tr("Обработка"), tr("Обработка завершена. Обработанный файл был загружен на сервер"));

        emit processingFinished();
    });
}

void ProcessingWidget::showTableWithProcessedFiles()
{
    QDialog tableWindow;

    QVBoxLayout* layout = new QVBoxLayout;
    
    QSqlQueryModel* model = new QSqlQueryModel(&tableWindow);
    model->setQuery(QString("SELECT band, contrast, sharpness, blocksize, filename FROM public.processedimages where sceneid='%0'").arg(_scene->sceneId()));    
    model->setHeaderData(0, Qt::Horizontal, tr("Канал"));
    model->setHeaderData(1, Qt::Horizontal, tr("Контраст"));
    model->setHeaderData(2, Qt::Horizontal, tr("Резкость"));
    model->setHeaderData(3, Qt::Horizontal, tr("Размер блока"));

    QTableView* view = new QTableView(&tableWindow);    
    view->setModel(model);
    view->resizeColumnToContents(4);
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
    tableWindow.resize(600, 300);
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