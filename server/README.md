Инструкция по установке серверной части HyExplorer
==================================================

Протестировано на Ubuntu 20.04

Установить Mercurial:

    sudo apt-get install mercurial

Клонировать репозиторий:

    mkdir /opt/hyexplorer
    cd /opt/hyexplorer
    git clone https://github.com/AlexBobkov/hyexplorer.git hyexplorer

Установить дополнительные пакеты:

    sudo apt-get install build-essential
    sudo apt-get install python3-flask python3-psycopg2 python3-gdal

Чтобы проверить работу сервера, можно запустить временный веб-сервер:

    ./run.sh
    
Веб-сервер будет поднят на порту 5000, поэтому нужно его открыть в файерволе:

    sudo ufw allow 5000/tcp
    
Проверочная ссылка: http://virtualglobe.ru:5000/overview/Hyperion/EO10010012012182110KF_SGS_01

Но для полноценной работы лучше настроить веб-сервер nginx.

Вписать в конфиг nginx секции для статических файлов и API:

    location /geoportal/ {
        root /opt/www;
        try_files $uri $uri/ =404;
    }

    location = /geoportalapi { rewrite ^ /geoportalapi/; }
    location /geoportalapi { try_files $uri @geoportalapi; }
    location @geoportalapi {
        include uwsgi_params;
        uwsgi_pass unix:/tmp/geoportal.sock;
    }

Порестартить nginx:

    sudo systemctl restart nginx

Проверить работоспособность можно, если запустить uwsgi командой:

    sudo -u www-data uwsgi geoportal.ini
    
Далее нужно настроить службу:

    sudo cp geoportal.service /etc/systemd/system/
    sudo systemctl enable geoportal
    sudo systemctl start geoportal

Проверочная ссылка: https://virtualglobe.ru/geoportalapi/overview/Hyperion/EO10010012012182110KF_SGS_01
    
Flask: https://flask.palletsprojects.com/en/1.1.x/deploying/uwsgi/
UWSGI: https://uwsgi-docs.readthedocs.io/en/latest/index.html
