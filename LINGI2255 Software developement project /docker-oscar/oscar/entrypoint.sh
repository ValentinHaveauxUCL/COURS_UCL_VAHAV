#!/usr/bin/env sh

WORKDIR="/usr/src"
TMPDIR="/tmp"
APP="/oscar"

app_init() {
  cd "${WORKDIR}${APP}" || exit 1

  ./makemigrations.sh
  python manage.py migrate
  python manage.py createsuperuser

  touch ${TMPDIR}/.app_init

  cd - || exit 1
}

# If the docker volume erased the app, copy the temporary app directory to
# the volume directory
if [ ! $(ls "${WORKDIR}${APP}") ]; then
  rsync -r "${TMPDIR}${APP}/*" "${WORKDIR}${APP}/"
fi

# If the POSTGRES_HOST variable is set, replace postgres host within app
# settings file
if [ ! -z ${POSTGRES_HOST+x} ]; then
  sed -ir "s/'HOST': (\b[0-9]{1,3}\.){3}[0-9]{1,3}\b/'HOST': ${POSTGRES_HOST}/" \
    /usr/src/oscar/oscar/settings.py
fi

if [ ! -f ${TMPDIR}/.app_init ]; then
  app_init
fi

chown -R oscar:oscar "${WORKDIR}${APP}"
cd "${WORKDIR}${APP}" || exit 1
python manage.py runserver 0.0.0.0:8000
