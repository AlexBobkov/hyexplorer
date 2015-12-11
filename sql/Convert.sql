-- Копирование части столбцов в отдельную таблицу

insert into hyperion
select ogc_fid, orbitpath, orbitrow, targetpath, targetrow, processinglevel, satelliteinclination, lookangle, cloudmin, cloudmax from scenes;

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