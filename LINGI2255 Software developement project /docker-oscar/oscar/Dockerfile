FROM python:2.7-slim
MAINTAINER 3mpr <florian.indot@mail.com>

WORKDIR /tmp

ENV BUILD_DEPS="git wget gcc build-essential" \
    DEPS="python2.7 libpq-dev python2.7-dev rsync" \
    OSCAR_REPO="https://github.com/ioune1993/oscareducation.git" \
    OSCAR_DIR="oscareducation" \
    OSCAR_REQUIREMENTS="requirements-oscar2.txt"

COPY ["oscareducation", "/tmp/oscareducation"]

RUN set -ex \
 && apt-get update && apt-get install -y ${BUILD_DEPS} ${DEPS} \
 && wget https://github.com/Yelp/dumb-init/releases/download/v1.2.0/dumb-init_1.2.0_amd64.deb \
 && dpkg -i dumb-init_*.deb \
 && mkdir -p /usr/src/oscar \
 && pip install --upgrade pip \
 && cd ${OSCAR_DIR} \
 && pip install --upgrade -r ${OSCAR_REQUIREMENTS} \
 && addgroup oscar \
 && adduser --gecos "" --ingroup oscar --home /usr/src/oscar --disabled-login oscar \
 && chown -R oscar:oscar /usr/src/oscar \
 && chown -R oscar:oscar /tmp/oscareducation \
 && apt remove --purge -y ${BUILD_DEPS} \
 && rm -rf /var/cache/apt/*

COPY ["entrypoint.sh", "/"]
VOLUME /usr/src/oscar

ENTRYPOINT ["dumb-init", "--"]
CMD ["/entrypoint.sh"]
