CREATE TABLE presley.experiments
(
    id text COLLATE pg_catalog."default" NOT NULL,
    starttime timestamp with time zone,
    endtime timestamp with time zone,
    "user" text COLLATE pg_catalog."default",
    description text COLLATE pg_catalog."default",
    CONSTRAINT experiments_pkey PRIMARY KEY (id)
)

TABLESPACE pg_default;

ALTER TABLE presley.experiments
    OWNER to postgres;

CREATE TABLE presley.series
(
    id text COLLATE pg_catalog."default" NOT NULL,
    experiment text COLLATE pg_catalog."default",
    commitid text COLLATE pg_catalog."default",
    index integer,
    starttime time without time zone,
    endtime time without time zone,
    log text COLLATE pg_catalog."default",
    CONSTRAINT series_pkey PRIMARY KEY (id),
    CONSTRAINT fk_experiment FOREIGN KEY (experiment)
        REFERENCES presley.experiments (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
        NOT VALID
)

TABLESPACE pg_default;

ALTER TABLE presley.series
    OWNER to postgres;

CREATE TABLE presley.measurements
(
    series text COLLATE pg_catalog."default",
    round integer,
    node_id integer,
    cpu_id integer,
    thread_id integer,
    timestamp time without time zone,
    value bigint,
    CONSTRAINT fk_series FOREIGN KEY (series)
        REFERENCES presley.series (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
        NOT VALID
)

TABLESPACE pg_default;

ALTER TABLE presley.measurements
    OWNER to postgres;