#!/usr/bin/env python3
"""
paquete.py - crea el paquete del Gestor de placas (Arduino IDE 2.x) de stc8duino.

Genera en dist/:
  - stc8duino-<version>.tar.bz2         la plataforma
  - package_stc8duino_index.json        indice (plataforma + SDCC)

Publicacion:
  1. python3 tools/paquete.py
  2. subir el .tar.bz2 a la rama "paquetes" del repositorio
  3. copiar el .json a la raiz de la rama main y hacer push
  4. en el IDE, URL adicional del gestor de placas:
     https://raw.githubusercontent.com/CEPH81/arduino-stc8duino/main/package_stc8duino_index.json

SDCC 4.3 (mcs51) es el que publica ch55xduino (DeqingSun) en sus releases de GitHub.
Parte de stc8duino. LGPL 2.1.
"""
import bz2, hashlib, io, json, os, re, sys, tarfile

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DIST = os.path.join(RAIZ, 'dist')
REPO = 'CEPH81/arduino-stc8duino'
PAQUETE = 'stc8duino'
FECHA = 1767225600  # 2026-01-01
BASE = os.environ.get('STC8DUINO_URL', 'https://raw.githubusercontent.com/%s/paquetes/' % REPO)

INCLUIR = ['boards.txt', 'platform.txt', 'README.md', 'cores', 'variants', 'libraries',
           'tools/wrapper', 'tools/win', 'tools/linux']
EXCLUIR = re.compile(r'(^|/)(platform\.local\.txt|\.git[^/]*|__pycache__|[^/]*\.(o|d|pyc))(/|$)')

SDCC_VER = 'build.14184_2'
SDCC_URL = 'https://github.com/DeqingSun/ch55xduino/releases/download/0.0.18/'
SDCC = {"name": "sdcc", "version": SDCC_VER, "systems": [
    {"host": "x86_64-pc-linux-gnu", "url": SDCC_URL + "sdcc-mcs51-amd64-unknown-linux2.5-20230702-14184_2.tar.bz2",
     "archiveFileName": "sdcc-mcs51-amd64-unknown-linux2.5-20230702-14184_2.tar.bz2",
     "checksum": "SHA-256:0beeccaa9c5c16bf542385d323021c2d5bde890160299871f6a52c89f9cdfaf2", "size": "3306990"},
    {"host": "i686-mingw32", "url": SDCC_URL + "sdcc-mcs51-i586-mingw32msvc-20230702-14184_2.tar.bz2",
     "archiveFileName": "sdcc-mcs51-i586-mingw32msvc-20230702-14184_2.tar.bz2",
     "checksum": "SHA-256:1d4313cad00e11c0b454c7dd24a2cccb27df9c23029701651271cc89924aaffc", "size": "5040777"},
    {"host": "aarch64-linux-gnu", "url": SDCC_URL + "sdcc-mcs51-aarch64-linux-gnu-20230701-14184_2.tar.bz2",
     "archiveFileName": "sdcc-mcs51-aarch64-linux-gnu-20230701-14184_2.tar.bz2",
     "checksum": "SHA-256:4459b7d3e6c690a91a4ea4ac5f8915c390d4b9ada7e9dac4be45e8865b28c569", "size": "3283863"},
]}


def leer(clave):
    for l in open(os.path.join(RAIZ, 'platform.txt'), encoding='utf-8'):
        if l.startswith(clave + '='):
            return l.split('=', 1)[1].strip()
    sys.exit('platform.txt sin ' + clave)


def placas():
    return [{"name": m.group(1)} for m in
            re.finditer(r'^\w+\.name=(.+)$', open(os.path.join(RAIZ, 'boards.txt'), encoding='utf-8').read(), re.M)]


def crear_tar(version):
    nombre = '%s-%s.tar.bz2' % (PAQUETE, version)
    tope = '%s-%s' % (PAQUETE, version)
    buf = io.BytesIO()
    with tarfile.open(fileobj=buf, mode='w', format=tarfile.GNU_FORMAT) as tar:
        archivos = []
        for inc in INCLUIR:
            ruta = os.path.join(RAIZ, inc)
            if os.path.isfile(ruta):
                archivos.append(inc)
            else:
                for r, ds, fs in os.walk(ruta):
                    ds.sort()
                    for f in sorted(fs):
                        archivos.append(os.path.relpath(os.path.join(r, f), RAIZ).replace(os.sep, '/'))
        for rel in sorted(archivos):
            if EXCLUIR.search(rel):
                continue
            ruta = os.path.join(RAIZ, rel)
            info = tar.gettarinfo(ruta, arcname=tope + '/' + rel)
            info.uid = info.gid = 0
            info.uname = info.gname = ''
            info.mtime = FECHA
            ejecutable = rel.endswith('.sh') or rel.startswith('tools/linux/') or rel.endswith('.exe')
            info.mode = 0o755 if ejecutable else 0o644
            with open(ruta, 'rb') as f:
                tar.addfile(info, f)
    datos = bz2.compress(buf.getvalue(), 9)
    os.makedirs(DIST, exist_ok=True)
    open(os.path.join(DIST, nombre), 'wb').write(datos)
    return nombre, len(datos), hashlib.sha256(datos).hexdigest()


def main():
    version = leer('version')
    nombre, tam, sha = crear_tar(version)
    web = "https://github.com/%s" % REPO
    indice = {"packages": [{
        "name": PAQUETE,
        "maintainer": "MikrocontrollerCephs (CEPH81)",
        "websiteURL": web,
        "email": "",
        "help": {"online": web},
        "platforms": [{
            "name": leer('name'),
            "architecture": "stc8",
            "version": version,
            "category": "Contributed",
            "url": BASE + nombre,
            "archiveFileName": nombre,
            "checksum": "SHA-256:" + sha,
            "size": str(tam),
            "help": {"online": web},
            "boards": placas(),
            "toolsDependencies": [{"packager": PAQUETE, "name": "sdcc", "version": SDCC_VER}],
        }],
        "tools": [SDCC],
    }]}
    ruta = os.path.join(DIST, 'package_%s_index.json' % PAQUETE)
    with open(ruta, 'w', encoding='utf-8') as f:
        json.dump(indice, f, indent=2, ensure_ascii=False)
        f.write('\n')
    print('Plataforma %s: %s (%d bytes, SHA-256 %s)' % (version, nombre, tam, sha))
    print('Indice: %s' % ruta)


if __name__ == '__main__':
    main()
