CREATE TABLE public.scenes
(
  ogc_fid serial,
  sensor character varying NOT NULL,
  sceneid character varying NOT NULL,  
  scenetime timestamp without time zone,
  pixelsize double precision,
  bounds geography(Polygon,4326),
  sunazimuth double precision,
  sunelevation double precision,
  hasoverview boolean,
  hasscene boolean,
  overviewname character varying,
  CONSTRAINT scenes_pk PRIMARY KEY (ogc_fid)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE public.scenes
  OWNER TO "portal";

CREATE INDEX scenes_bounds_index ON scenes USING GIST (bounds);

--

CREATE TABLE public.hyperion
(
  ogc_fid integer NOT NULL,
  orbitpath integer,
  orbitrow integer,
  targetpath integer,
  targetrow integer,
  processinglevel character varying,
  satelliteinclination double precision,
  lookangle double precision,  
  cloudmin integer,
  cloudmax integer,
  CONSTRAINT hyperion_pk PRIMARY KEY (ogc_fid),
  CONSTRAINT hyperion_fk FOREIGN KEY (ogc_fid)
      REFERENCES public.scenes (ogc_fid) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
ALTER TABLE public.hyperion
  OWNER TO "portal";

--

CREATE TABLE public.aviris
(
  ogc_fid integer NOT NULL,
  sitename character varying,
  comments character varying,
  investigator character varying,
  scenerotation double precision,
  tape character varying,
  geover character varying,
  rdnver character varying,
  meansceneelev double precision,
  minsceneelev double precision,
  maxsceneelev double precision,
  flight integer,
  run integer,
  CONSTRAINT aviris_pk PRIMARY KEY (ogc_fid),
  CONSTRAINT aviris_fk FOREIGN KEY (ogc_fid)
      REFERENCES public.scenes (ogc_fid) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
ALTER TABLE public.aviris
  OWNER TO "portal";