-- Копирование части столбцов в отдельную таблицу

INSERT INTO hyperion
SELECT ogc_fid, orbitpath, orbitrow, targetpath, targetrow, processinglevel, satelliteinclination, lookangle, cloudmin, cloudmax FROM scenes;

-- Удаление лишних столбцов из таблицы scenes

ALTER TABLE scenes DROP COLUMN orbitpath;
ALTER TABLE scenes DROP COLUMN orbitrow;
ALTER TABLE scenes DROP COLUMN targetpath;
ALTER TABLE scenes DROP COLUMN targetrow;
ALTER TABLE scenes DROP COLUMN processinglevel;
ALTER TABLE scenes DROP COLUMN satelliteinclination;
ALTER TABLE scenes DROP COLUMN lookangle;
ALTER TABLE scenes DROP COLUMN cloudmin;
ALTER TABLE scenes DROP COLUMN cloudmax;

-- Добавление новых столбцов к таблице scenes

ALTER TABLE scenes ADD COLUMN pixelsize double precision;
UPDATE scenes SET pixelsize=30 WHERE sensor='Hyperion';

ALTER TABLE scenes ADD COLUMN sceneurl character varying;

