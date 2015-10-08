CREATE TABLE public.scenes
(
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
  CONSTRAINT scenes_pk PRIMARY KEY (sceneid)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE public.scenes
  OWNER TO "user";
