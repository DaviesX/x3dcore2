TEMPLATE = subdirs
SUBDIRS = corelib/corelib.pro \
          corelib/test/material/material.pro \
          demoplayer/demoplayer.pro \
    corelib/test/pathtracer \
    corelib/test/renderer

CONFIG += ordered
