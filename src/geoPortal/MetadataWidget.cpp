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

    _translations["sceneid"] = tr("Идентификатор");
    _translations["scenetime"] = tr("Дата и время съемки");
    _translations["pixelsize"] = tr("Размер пикселя");
    _translations["sunazimuth"] = tr("Азимут Солнца");
    _translations["sunelevation"] = tr("Высота Солнца");

    _translations["orbitpath"] = tr("Номер витка");
    _translations["orbitrow"] = tr("Номер ряда");
    _translations["targetpath"] = tr("Номер целевого витка");
    _translations["targetrow"] = tr("Номер целевого ряда");
    _translations["processinglevel"] = tr("Уровень обработки");
    _translations["cloudmax"] = tr("Процент облачности");
    _translations["satelliteinclination"] = tr("Наклонение орбиты спутника");
    _translations["lookangle"] = tr("Угол между надиром и центром сцены");

    _translations["sitename"] = tr("Имя объекта");
    _translations["comments"] = tr("Комментарии");
    _translations["investigator"] = tr("Исследователь");
    _translations["scenerotation"] = tr("Вращение сцены");
    _translations["tape"] = tr("Номер ленты");
    _translations["geover"] = tr("Геокоррекция");
    _translations["rdnver"] = tr("RDN");
    _translations["meansceneelev"] = tr("Средняя высота сцены");
    _translations["minsceneelev"] = tr("Минимальная высота сцены");
    _translations["maxsceneelev"] = tr("Максимальная высота сцены");
    _translations["flight"] = tr("Номер вылета");
    _translations["run"] = tr("Номер захода");
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
