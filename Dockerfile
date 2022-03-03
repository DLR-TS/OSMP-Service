FROM ubuntu
MAINTAINER frank.baumgarten@dlr.de

RUN mkdir setlevel
WORKDIR /setlevel
RUN mkdir logs

COPY * /setlevel/

CMD ./CoSimulationManager SetLevelConfig.yml -d
