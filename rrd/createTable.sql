create table baatvakta(
	id int not null auto_increment,
	time timestamp,
	tCabin float,
	tEngin float,
	tAft float,
	tOutside float,
	voltage12 float,
	voltage24 float,
	pEngineDuration float,
	pAftDuration float,
	PRIMARY KEY (id)
);
