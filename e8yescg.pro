TEMPLATE = subdirs
SUBDIRS = demoplayer/demoplayer.pro \
    corelib/corelib.pro \
    corelib/test/tst_material \
    corelib/test/tst_pathtracer \
    corelib/test/tst_renderer

CONFIG += ordered
