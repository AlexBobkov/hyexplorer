###############################################################################
# Устаревший скрипт - автоматически добавляет имя обзора в таблицу сцен
# Но оказалось, что имя обзора автоматически не выводится из имени сцены
# Скрипт оставлен в образовательных целях
###############################################################################

import psycopg2

#conn = psycopg2.connect("host=localhost dbname=GeoPortal user=user password=user")
conn = psycopg2.connect("host=virtualglobe.ru dbname=GeoPortal user=portal password=PortalPass")

cur = conn.cursor()
cur.execute("select sceneid from scenes where hasoverview;")

counter = 0
data = cur.fetchone()
while data != None:
    sceneid = data[0]
    filename = sceneid[:3] + sceneid[4:] + '.jpeg'

    updatecur = conn.cursor()
    updatecur.execute("update scenes set overviewname=%s where sceneid=%s;", (filename, sceneid))
    conn.commit()
    updatecur.close()

    counter += 1
    data = cur.fetchone()

print(counter)

cur.close()
conn.close()
