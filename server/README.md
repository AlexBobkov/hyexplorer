Инструкция по установке серверной части HyExplorer
==================================================

Протестировано на Федоре 24.

Установить Mercurial:

    sudo dnf install mercurial

Клонировать репозиторий:

    mkdir /opt/hyexplorer
    cd /opt/hyexplorer
    hg clone https://bitbucket.org/AlexBobkov/hyexplorer hyexplorer

Установить дополнительные пакеты:

    sudo dnf install python3-flask
    sudo dnf install python3-psycopg2
    sudo dnf install gdal-python3

Чтобы проверить работу сервера, можно запустить временный веб-сервер:

    ./run.sh
    
Веб-сервер будет поднят на порту 5000, поэтому нужно его открыть в файерволе:

    sudo iptables -A INPUT -p tcp --dport 5000 -j ACCEPT
    
Проверочная ссылка: http://virtualglobe.ru:5000/overview/Hyperion/EO10010012012182110KF_SGS_01

Но для полноценной работы лучше настроить веб-сервер Апач.

Сначала установить mod_wsgi:
    
    sudo dnf install python3-mod_wsgi
    
В конфиге Апача внутри секции VirtualHost дописать:

    WSGIDaemonProcess geoportal user=alex group=alex threads=5
    WSGIScriptAlias /geoportalapi /opt/hyexplorer/hyexplorer/server/geoportal.wsgi
    
    <Directory "/opt/hyexplorer/hyexplorer/server">
        WSGIProcessGroup geoportal
        WSGIApplicationGroup %{GLOBAL}
        WSGIScriptReloading On
        Require all granted
    </Directory>
    
Здесь указываются имя пользователя (alex), путь к файлу .wsgi и папка со скриптами.

Сделать рестарт Апача:

    sudo systemctl restart httpd
    
Изменить права доступа SELinux для файла с логами:

    cd /opt/hyexplorer/hyexplorer/server
    chcon -t httpd_log_t pylog.txt
    
Проверочная ссылка: https://virtualglobe.ru/geoportalapi/overview/Hyperion/EO10010012012182110KF_SGS_01
    
Справка: http://flask-russian-docs.readthedocs.io/ru/latest/deploying/mod_wsgi.html
