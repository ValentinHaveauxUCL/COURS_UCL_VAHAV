DOCKER = docker build
DCOMPOSE = docker-compose
ATTACH = attach
GIT = git

REPO = "https://github.com/ioune1993/oscareducation" 
DL_VLP = curl -fsSL "https://raw.githubusercontent.com/CWSpear/local-persist/master/scripts/install.sh" | sudo bash

PULL = clone
BUILD = build
PIPE = | bash
SUPIPE = | sudo bash  

RUN = up -d
STOP = stop

CLEAN = cd oscar && rm -rf oscareducation

TEST = echo "Not implemented!"


.SUFFIXES : .nothing

all:
	cd oscar
	# $(GIT) $(PULL) $(REPO)
	cd ..
	$(DL_VLP)
	#$(DCOMPOSE) $(BUILD) oscar

clean:
	rm -rf oscar/oscareducation

run:
	$(DCOMPOSE) $(RUN) postgres
	$(DCOMPOSE) $(RUN) oscar

stop:
	$(DCOMPOSE) $(STOP) oscar
	$(DCOMPOSE) $(STOP) postgres

attach:
	$(DOCKER) $(ATTACH) oscar 

test:
	$(TEST)

trun:
	$(TEST)

.PHONY: all clean run attach test trun
