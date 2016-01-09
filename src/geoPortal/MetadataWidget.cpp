#include "MetadataWidget.hpp"

#include <QStandardPaths>
#include <QVBoxLayout>
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QTextStream>

using namespace portal;

MetadataWidget::MetadataWidget(QWidget* parent) :
QWidget(),
_browser(0),
_variantManager(0)
{
    initUi();

    _translations["sceneid"] = QString::fromUtf8("Идентификатор");
    _translations["scenetime"] = QString::fromUtf8("Дата и время съемки");
    _translations["pixelsize"] = QString::fromUtf8("Размер пикселя");
    _translations["sunazimuth"] = QString::fromUtf8("Азимут Солнца");
    _translations["sunelevation"] = QString::fromUtf8("Высота Солнца");

    _translations["orbitpath"] = QString::fromUtf8("Номер витка");
    _translations["orbitrow"] = QString::fromUtf8("Номер ряда");
    _translations["targetpath"] = QString::fromUtf8("Номер целевого витка");
    _translations["targetrow"] = QString::fromUtf8("Номер целевого ряда");
    _translations["processinglevel"] = QString::fromUtf8("Уровень обработки");
    _translations["cloudmax"] = QString::fromUtf8("Процент облачности");
    _translations["satelliteinclination"] = QString::fromUtf8("Наклонение орбиты спутника");
    _translations["lookangle"] = QString::fromUtf8("Угол между надиром и центром сцены");

    _translations["sitename"] = QString::fromUtf8("Имя объекта");
    _translations["comments"] = QString::fromUtf8("Комментарии");
    _translations["investigator"] = QString::fromUtf8("Исследователь");
    _translations["scenerotation"] = QString::fromUtf8("Вращение сцены");
    _translations["tape"] = QString::fromUtf8("Номер ленты");
    _translations["geover"] = QString::fromUtf8("Геокоррекция");
    _translations["rdnver"] = QString::fromUtf8("RDN");
    _translations["meansceneelev"] = QString::fromUtf8("Средняя высота сцены");
    _translations["minsceneelev"] = QString::fromUtf8("Минимальная высота сцены");
    _translations["maxsceneelev"] = QString::fromUtf8("Максимальная высота сцены");
    _translations["flight"] = QString::fromUtf8("Номер вылета");
    _translations["run"] = QString::fromUtf8("Номер захода");
}

MetadataWidget::~MetadataWidget()
{
}

void MetadataWidget::initUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);

    _variantManager = new QtVariantPropertyManager(this);

    _browser = new QtTreePropertyBrowser(this);
    _browser->setFactoryForManager(_variantManager, new QtVariantEditorFactory(this));
    layout->addWidget(_browser);
}

void MetadataWidget::setScene(const ScenePtr& scene)
{
    if (_scene == scene)
    {
        return;
    }

    _scene = scene;

    for (const auto& kv : _props)
    {
        _browser->removeProperty(kv.second);
    }

    for (const auto& kv : _scene->attribs())
    {
        if (kv.second.isValid() &&
            (kv.second.type() == QVariant::Double ||
            kv.second.type() == QVariant::Int ||
            kv.second.type() == QVariant::String ||
            kv.second.type() == QVariant::DateTime) &&
            _translations.find(kv.first) != _translations.end())
        {
            if (_props.find(kv.first) == _props.end())
            {
                auto text = _translations.find(kv.first)->second;

                QVariant::Type type = kv.second.type();
                if (kv.second.type() == QVariant::DateTime)
                {
                    type = QVariant::String;
                }

                QtVariantProperty* prop = _variantManager->addProperty(type, text);
                prop->setAttribute("readOnly", true);
                _props[kv.first] = prop;
            }

            QtVariantProperty* prop = _props[kv.first];

            if (kv.second.type() == QVariant::DateTime)
            {
                prop->setValue(kv.second.toDateTime().toString(Qt::ISODate));
            }
            else
            {
                prop->setValue(kv.second);
            }

            _browser->addProperty(prop);
        }
    }
}
