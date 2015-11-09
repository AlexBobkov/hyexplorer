CREATE TABLE public.scenes
(
  ogc_fid serial,
  sensor character varying NOT NULL,
  sceneid character varying NOT NULL,
  orbitpath integer,
  orbitrow integer,
  targetpath integer,
  targetrow integer,
  processinglevel character varying,
  sunazimuth double precision,
  sunelevation double precision,
  satelliteinclination double precision,
  lookangle double precision,
  scenetime timestamp without time zone,
  cloudmin integer,
  cloudmax integer,
  bounds geography(Polygon,4326),
  hasoverview boolean,
  hasscene boolean,
  CONSTRAINT scenes_pk PRIMARY KEY (ogc_fid)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE public.scenes
  OWNER TO "portal";

CREATE INDEX scenes_bounds_index ON scenes USING GIST (bounds);